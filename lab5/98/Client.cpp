#include <iostream>
#include <string>
#include <vector>
#include <sstream> 
#include <windows.h>
#include "common.h"

HANDLE hMyResponsePipe = INVALID_HANDLE_VALUE;
char g_my_response_pipe_name[PIPE_NAME_MAX_LENGTH];
DWORD g_client_pid;

void print_employee_client(const Employee& emp) {
	std::cout << "  ID: " << emp.num
		<< ", Name: " << emp.name
		<< ", Hours: " << emp.hours << std::endl;
}

bool send_request_and_get_response(RequestMessage& request, ResponseMessage& response) {
	request.client_pid = g_client_pid;
	strncpy(request.client_response_pipe_name, g_my_response_pipe_name, PIPE_NAME_MAX_LENGTH - 1);
	request.client_response_pipe_name[PIPE_NAME_MAX_LENGTH - 1] = '\0';

	HANDLE hCurrentServerRequestPipe = CreateFile(
		SERVER_REQUEST_PIPE_NAME,
		GENERIC_WRITE, 0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (hCurrentServerRequestPipe == INVALID_HANDLE_VALUE) {
		std::cerr << "Client Error: Could not connect to server request pipe '" << SERVER_REQUEST_PIPE_NAME
			<< "' for this request. GLE: " << GetLastError() << std::endl;
		return false;
	}

	DWORD bytesWritten;
	if (!WriteFile(hCurrentServerRequestPipe, &request, sizeof(RequestMessage), &bytesWritten, NULL) || bytesWritten != sizeof(RequestMessage)) {
		std::cerr << "Client Error: WriteFile to server failed. GLE: " << GetLastError() << std::endl;
		CloseHandle(hCurrentServerRequestPipe);         return false;
	}


	BOOL bClientPipeConnected = ConnectNamedPipe(hMyResponsePipe, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

	if (!bClientPipeConnected) {
		std::cerr << "Client Error: Server did not connect to my response pipe (" << g_my_response_pipe_name
			<< "). ConnectNamedPipe GLE: " << GetLastError() << std::endl;
		CloseHandle(hCurrentServerRequestPipe);         return false;
	}


	DWORD bytesRead;
	if (!ReadFile(hMyResponsePipe, &response, sizeof(ResponseMessage), &bytesRead, NULL) || bytesRead != sizeof(ResponseMessage)) {
		std::cerr << "Client Error: ReadFile from own response pipe failed. GLE: " << GetLastError() << std::endl;
		DisconnectNamedPipe(hMyResponsePipe);
		CloseHandle(hCurrentServerRequestPipe);
		return false;
	}


	DisconnectNamedPipe(hMyResponsePipe);

	CloseHandle(hCurrentServerRequestPipe);

	return true;
}


int main() {
	g_client_pid = GetCurrentProcessId();

	std::ostringstream oss;
	oss << "\\\\.\\pipe\\ClientResponsePipe_" << g_client_pid;
	strncpy(g_my_response_pipe_name, oss.str().c_str(), PIPE_NAME_MAX_LENGTH - 1);
	g_my_response_pipe_name[PIPE_NAME_MAX_LENGTH - 1] = '\0';

	std::cout << "Client (PID: " << g_client_pid << ") started. Response pipe: " << g_my_response_pipe_name << std::endl;

	hMyResponsePipe = CreateNamedPipe(
		g_my_response_pipe_name,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1,
		sizeof(RequestMessage),
		sizeof(ResponseMessage),
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL
	);

	if (hMyResponsePipe == INVALID_HANDLE_VALUE) {
		std::cerr << "Client Error: CreateNamedPipe for own response pipe failed. GLE: " << GetLastError() << std::endl;
		return 1;
	}

	std::cout << "Client attempting initial contact with server..." << std::endl;
	HANDLE hTestPipe = CreateFile(SERVER_REQUEST_PIPE_NAME, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hTestPipe == INVALID_HANDLE_VALUE) {
		std::cerr << "Client Error: Server's main request pipe '" << SERVER_REQUEST_PIPE_NAME << "' not accessible at startup. GLE: " << GetLastError() << std::endl;
		std::cerr << "Is the server running?" << std::endl;
		CloseHandle(hMyResponsePipe);
		return 1;
	}
	CloseHandle(hTestPipe);     std::cout << "Client: Server pipe seems available. Proceeding to menu." << std::endl;


	RequestMessage request;
	ResponseMessage response;
	bool running = true;

	while (running) {
		std::cout << "\nClient Menu (PID " << g_client_pid << "):" << std::endl;
		std::cout << "1. Read Record" << std::endl;
		std::cout << "2. Modify Record" << std::endl;
		std::cout << "3. Exit" << std::endl;
		std::cout << "Enter choice: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore();

		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(10000, '\n');             std::cout << "Invalid input. Please enter a number." << std::endl;
			continue;
		}

		ZeroMemory(&request, sizeof(RequestMessage));
		ZeroMemory(&response, sizeof(ResponseMessage));


		switch (choice) {
		case 1: {
			std::cout << "Enter Employee ID to read: ";
			if (!(std::cin >> request.record_id)) {
				std::cout << "Invalid ID input." << std::endl;
				std::cin.clear(); std::cin.ignore(10000, '\n'); continue;
			}
			std::cin.ignore();
			request.type = REQ_READ;

			if (send_request_and_get_response(request, response)) {
				if (response.type == DATA_RECORD) {
					std::cout << "Record received:" << std::endl;
					print_employee_client(response.emp_data);

					RequestMessage finishReadRequest;
					ZeroMemory(&finishReadRequest, sizeof(RequestMessage));
					finishReadRequest.type = FINISH_READ;
					finishReadRequest.record_id = request.record_id;
					ResponseMessage finishReadResponse;
					ZeroMemory(&finishReadResponse, sizeof(ResponseMessage));

					std::cout << "Press Enter to finish viewing and release read access...";
					std::string dummy;
					std::getline(std::cin, dummy);


					if (send_request_and_get_response(finishReadRequest, finishReadResponse)) {
						if (finishReadResponse.type == OPERATION_SUCCESS) {
							std::cout << "Finished viewing record. Read access released by server." << std::endl;
						}
						else {
							std::cout << "Server reported issue finishing read: " << finishReadResponse.error_message << " (Type: " << finishReadResponse.type << ")" << std::endl;
						}
					}
					else {
						std::cout << "Communication error during finish read confirmation." << std::endl;
					}

				}
				else {
					std::cout << "Server Error: " << response.error_message << " (Type: " << response.type << ")" << std::endl;
				}
			}
			else {
				std::cout << "Communication error during read operation." << std::endl;
			}
			break;
		}
		case 2: {
			std::cout << "Enter Employee ID to modify: ";
			if (!(std::cin >> request.record_id)) {
				std::cout << "Invalid ID input." << std::endl;
				std::cin.clear(); std::cin.ignore(10000, '\n'); continue;
			}
			std::cin.ignore();
			request.type = REQ_MODIFY;

			if (send_request_and_get_response(request, response)) {
				if (response.type == DATA_RECORD) {
					std::cout << "Current record data:" << std::endl;
					print_employee_client(response.emp_data);

					Employee modified_emp = response.emp_data;
					std::cout << "Enter new Name (max " << NAME_MAX_LENGTH - 1 << " chars, press Enter to keep current '" << modified_emp.name << "'): ";
					std::string new_name_str;
					std::getline(std::cin, new_name_str);
					if (!new_name_str.empty()) {
						strncpy(modified_emp.name, new_name_str.c_str(), NAME_MAX_LENGTH - 1);
						modified_emp.name[NAME_MAX_LENGTH - 1] = '\0';
					}


					std::cout << "Enter new Hours (press Enter to keep current '" << modified_emp.hours << "'): ";
					std::string hours_str;
					std::getline(std::cin, hours_str);
					if (!hours_str.empty()) {
						char* endptr;
						double new_hours = strtod(hours_str.c_str(), &endptr);
						if (endptr != hours_str.c_str() && *endptr == '\0') {
							modified_emp.hours = new_hours;
						}
						else {
							std::cout << "Invalid hours input, keeping current." << std::endl;
						}
					}


					std::cout << "Send modified record to server? (y/n): ";
					char confirm_send;
					std::cin >> confirm_send;
					std::cin.ignore();

					if (confirm_send == 'y' || confirm_send == 'Y') {
						RequestMessage modRequest;
						ZeroMemory(&modRequest, sizeof(RequestMessage));
						modRequest.type = MODIFIED_RECORD;
						modRequest.record_id = modified_emp.num;
						modRequest.emp_data = modified_emp;
						ResponseMessage modResponse;
						ZeroMemory(&modResponse, sizeof(ResponseMessage));

						if (send_request_and_get_response(modRequest, modResponse)) {
							if (modResponse.type == OPERATION_SUCCESS) {
								std::cout << "Record updated successfully on server." << std::endl;
							}
							else {
								std::cout << "Server Error updating record: " << modResponse.error_message << " (Type: " << modResponse.type << ")" << std::endl;
							}
						}
						else {
							std::cout << "Communication error sending modified record." << std::endl;
						}
					}

					std::cout << "Release lock for record " << request.record_id << "? (y/n): ";
					char confirm_release;
					std::cin >> confirm_release;
					std::cin.ignore();
					if (confirm_release == 'y' || confirm_release == 'Y') {
						RequestMessage releaseRequest;
						ZeroMemory(&releaseRequest, sizeof(RequestMessage));
						releaseRequest.type = RELEASE_LOCK_MODIFY;
						releaseRequest.record_id = request.record_id;
						ResponseMessage releaseResponse;
						ZeroMemory(&releaseResponse, sizeof(ResponseMessage));
						if (send_request_and_get_response(releaseRequest, releaseResponse)) {
							if (releaseResponse.type == OPERATION_SUCCESS) {
								std::cout << "Lock released." << std::endl;
							}
							else {
								std::cout << "Server Error releasing lock: " << releaseResponse.error_message << " (Type: " << releaseResponse.type << ")" << std::endl;
							}
						}
						else {
							std::cout << "Communication error releasing lock." << std::endl;
						}
					}
				}
				else {
					std::cout << "Server Error: " << response.error_message << " (Type: " << response.type << ")" << std::endl;
				}
			}
			else {
				std::cout << "Communication error during modify request." << std::endl;
			}
			break;
		}
		case 3: {
			request.type = CLIENT_EXITING;
			request.record_id = -1;

			ResponseMessage exitResponse;
			ZeroMemory(&exitResponse, sizeof(ResponseMessage));
			std::cout << "Notifying server of exit..." << std::endl;
			if (send_request_and_get_response(request, exitResponse)) {
				if (exitResponse.type == OPERATION_SUCCESS) {
					std::cout << "Server acknowledged exit." << std::endl;
				}
				else {
					std::cout << "Server responded to exit notice with: " << exitResponse.error_message << " (Type: " << exitResponse.type << ")" << std::endl;
				}
			}
			else {
				std::cout << "Failed to send/receive exit confirmation to server." << std::endl;
			}
			running = false;
			break;
		}
		default:
			std::cout << "Invalid choice. Try again." << std::endl;
		}
	}

	CloseHandle(hMyResponsePipe);
	std::cout << "Client (PID: " << g_client_pid << ") shutting down." << std::endl;
	return 0;
}