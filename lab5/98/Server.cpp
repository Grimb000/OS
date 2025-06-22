#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <windows.h>
#include "common.h"; 
std::string g_binary_filename;
std::vector<Employee> g_employees_cache; CRITICAL_SECTION g_data_critical_section; CRITICAL_SECTION g_locks_critical_section;
struct LockInfo {
	enum LockState { UNLOCKED, READ_LOCKED, WRITE_LOCKED } state;
	std::vector<DWORD> reader_pids;     DWORD writer_pid;
	LockInfo() : state(UNLOCKED), writer_pid(0) {}
};
std::map<int, LockInfo> g_record_locks;
std::vector<HANDLE> g_client_process_handles;
int g_initial_client_count = 0;
volatile int g_terminated_client_count = 0;
void print_employee(const Employee& emp) {
	std::cout << "ID: " << emp.num
		<< ", Name: " << emp.name
		<< ", Hours: " << emp.hours << std::endl;
}

bool load_employees_and_init_locks() {
	std::ifstream file(g_binary_filename.c_str(), std::ios::binary | std::ios::in);
	if (!file) {
		std::cerr << "Error: Could not open file for reading: " << g_binary_filename << std::endl;
		return false;
	}

	EnterCriticalSection(&g_data_critical_section);
	EnterCriticalSection(&g_locks_critical_section);

	g_employees_cache.clear();
	g_record_locks.clear();
	Employee emp;
	while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
		g_employees_cache.push_back(emp);
		g_record_locks[emp.num] = LockInfo();
	}

	LeaveCriticalSection(&g_locks_critical_section);
	LeaveCriticalSection(&g_data_critical_section);
	file.close();
	return true;
}

void display_all_employees(const std::string& title) {
	std::cout << "\n--- " << title << " ---" << std::endl;
	EnterCriticalSection(&g_data_critical_section);
	if (g_employees_cache.empty()) {
		std::cout << "No employees in the file." << std::endl;
	}
	else {
		for (size_t i = 0; i < g_employees_cache.size(); ++i) {
			print_employee(g_employees_cache[i]);
		}
	}
	LeaveCriticalSection(&g_data_critical_section);
	std::cout << "------------------------" << std::endl;
}

int find_employee_index(int id) {
	for (size_t i = 0; i < g_employees_cache.size(); ++i) {
		if (g_employees_cache[i].num == id) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

bool update_employee_in_file(const Employee& emp_to_update) {
	EnterCriticalSection(&g_data_critical_section);
	int index = find_employee_index(emp_to_update.num);
	if (index == -1) {
		LeaveCriticalSection(&g_data_critical_section);
		std::cerr << "Error: Employee with ID " << emp_to_update.num << " not found for update." << std::endl;
		return false;
	}

	g_employees_cache[index] = emp_to_update;

	std::fstream file(g_binary_filename.c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
	if (!file) {
		LeaveCriticalSection(&g_data_critical_section);
		std::cerr << "Error: Could not open file for updating: " << g_binary_filename << std::endl;
		return false;
	}

	file.seekp(static_cast<long>(index) * sizeof(Employee), std::ios::beg);
	if (!file) {
		LeaveCriticalSection(&g_data_critical_section);
		std::cerr << "Error: Seekg failed in file: " << g_binary_filename << std::endl;
		file.close();
		return false;
	}
	file.write(reinterpret_cast<const char*>(&emp_to_update), sizeof(Employee));
	if (!file) {
		LeaveCriticalSection(&g_data_critical_section);
		std::cerr << "Error: Write failed in file: " << g_binary_filename << std::endl;
		file.close();
		return false;
	}
	file.close();
	LeaveCriticalSection(&g_data_critical_section);
	return true;
}


void handle_client_request(HANDLE hPipe, const RequestMessage& request) {
	ResponseMessage response;
	response.type = OPERATION_FAIL_OTHER;     memset(response.error_message, 0, sizeof(response.error_message));

	HANDLE hClientResponsePipe = INVALID_HANDLE_VALUE;

	hClientResponsePipe = CreateFile(
		request.client_response_pipe_name,
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (hClientResponsePipe == INVALID_HANDLE_VALUE) {
		std::cerr << "Server Error: Could not open client response pipe: " << request.client_response_pipe_name
			<< ". GLE: " << GetLastError() << std::endl;
		return;
	}

	EnterCriticalSection(&g_locks_critical_section);
	int record_id = request.record_id;
	LockInfo* lock = NULL;
	std::map<int, LockInfo>::iterator it = g_record_locks.find(record_id);
	if (it != g_record_locks.end()) {
		lock = &it->second;
	}
	else if (request.type != REQ_READ && request.type != REQ_MODIFY) {
	}


	switch (request.type) {
	case REQ_READ: {
		std::cout << "Server: Client " << request.client_pid << " requests READ for record " << record_id << std::endl;
		if (!lock) {
			response.type = OPERATION_FAIL_NOT_FOUND;
			strncpy(response.error_message, "Record not found.", sizeof(response.error_message) - 1);
		}
		else if (lock->state == LockInfo::WRITE_LOCKED && lock->writer_pid != request.client_pid) {
			response.type = OPERATION_FAIL_LOCKED;
			strncpy(response.error_message, "Record is write-locked by another client.", sizeof(response.error_message) - 1);
		}
		else {
			EnterCriticalSection(&g_data_critical_section);
			int index = find_employee_index(record_id);
			if (index != -1) {
				response.emp_data = g_employees_cache[index];
				response.type = DATA_RECORD;
				if (lock->state == LockInfo::UNLOCKED) {
					lock->state = LockInfo::READ_LOCKED;
				}
				bool found_reader = false;
				for (size_t i = 0; i < lock->reader_pids.size(); ++i) {
					if (lock->reader_pids[i] == request.client_pid) {
						found_reader = true;
						break;
					}
				}
				if (!found_reader) lock->reader_pids.push_back(request.client_pid);
			}
			else {
				response.type = OPERATION_FAIL_NOT_FOUND;
				strncpy(response.error_message, "Record not found in cache.", sizeof(response.error_message) - 1);
			}
			LeaveCriticalSection(&g_data_critical_section);
		}
		break;
	}
	case REQ_MODIFY: {
		std::cout << "Server: Client " << request.client_pid << " requests MODIFY for record " << record_id << std::endl;
		if (!lock) {
			response.type = OPERATION_FAIL_NOT_FOUND;
			strncpy(response.error_message, "Record not found.", sizeof(response.error_message) - 1);
		}
		else if (lock->state == LockInfo::UNLOCKED) {
			EnterCriticalSection(&g_data_critical_section);
			int index = find_employee_index(record_id);
			if (index != -1) {
				response.emp_data = g_employees_cache[index];
				response.type = DATA_RECORD;                 lock->state = LockInfo::WRITE_LOCKED;
				lock->writer_pid = request.client_pid;
				lock->reader_pids.clear();
			}
			else {
				response.type = OPERATION_FAIL_NOT_FOUND;
				strncpy(response.error_message, "Record not found in cache for modify.", sizeof(response.error_message) - 1);
			}
			LeaveCriticalSection(&g_data_critical_section);
		}
		else {
			response.type = OPERATION_FAIL_LOCKED;
			if (lock->writer_pid == request.client_pid) {
				strncpy(response.error_message, "Record already write-locked by you.", sizeof(response.error_message) - 1);
			}
			else {
				strncpy(response.error_message, "Record is locked by another client.", sizeof(response.error_message) - 1);
			}
		}
		break;
	}
	case MODIFIED_RECORD: {
		std::cout << "Server: Client " << request.client_pid << " sends MODIFIED_RECORD for " << record_id << std::endl;
		if (!lock) {
			response.type = OPERATION_FAIL_NOT_FOUND;
			strncpy(response.error_message, "Record not found for update.", sizeof(response.error_message) - 1);
		}
		else if (lock->state == LockInfo::WRITE_LOCKED && lock->writer_pid == request.client_pid) {
			if (update_employee_in_file(request.emp_data)) {
				response.type = OPERATION_SUCCESS;
			}
			else {
				response.type = OPERATION_FAIL_OTHER;
				strncpy(response.error_message, "Failed to write update to file.", sizeof(response.error_message) - 1);
			}
		}
		else {
			response.type = OPERATION_FAIL_LOCKED;
			strncpy(response.error_message, "Record not write-locked by you or lock lost.", sizeof(response.error_message) - 1);
		}
		break;
	}
	case RELEASE_LOCK_MODIFY: {
		std::cout << "Server: Client " << request.client_pid << " RELEASE_LOCK_MODIFY for " << record_id << std::endl;
		if (!lock) {
			response.type = OPERATION_FAIL_NOT_FOUND;
			strncpy(response.error_message, "Record not found for release.", sizeof(response.error_message) - 1);
		}
		else if (lock->state == LockInfo::WRITE_LOCKED && lock->writer_pid == request.client_pid) {
			lock->state = LockInfo::UNLOCKED;
			lock->writer_pid = 0;
			response.type = OPERATION_SUCCESS;
		}
		else {
			response.type = OPERATION_FAIL_LOCKED;             strncpy(response.error_message, "Record not write-locked by you for release.", sizeof(response.error_message) - 1);
		}
		break;
	}
	case FINISH_READ: {
		std::cout << "Server: Client " << request.client_pid << " FINISH_READ for " << record_id << std::endl;
		if (!lock) {
			response.type = OPERATION_FAIL_NOT_FOUND;
			strncpy(response.error_message, "Record not found for finish read.", sizeof(response.error_message) - 1);
		}
		else if (lock->state == LockInfo::READ_LOCKED) {
			bool found = false;
			for (size_t i = 0; i < lock->reader_pids.size(); ++i) {
				if (lock->reader_pids[i] == request.client_pid) {
					lock->reader_pids.erase(lock->reader_pids.begin() + i);
					found = true;
					break;
				}
			}
			if (lock->reader_pids.empty()) {
				lock->state = LockInfo::UNLOCKED;
			}
			response.type = OPERATION_SUCCESS;
		}
		else {
			response.type = OPERATION_FAIL_LOCKED;             strncpy(response.error_message, "Record was not read-locked by you or state changed.", sizeof(response.error_message) - 1);
		}
		break;
	}
	case CLIENT_EXITING: {
		std::cout << "Server: Client " << request.client_pid << " is EXITING." << std::endl;
		for (std::map<int, LockInfo>::iterator map_it = g_record_locks.begin(); map_it != g_record_locks.end(); ++map_it) {
			LockInfo& current_lock = map_it->second;
			if (current_lock.state == LockInfo::WRITE_LOCKED && current_lock.writer_pid == request.client_pid) {
				current_lock.state = LockInfo::UNLOCKED;
				current_lock.writer_pid = 0;
				std::cout << "Server: Released write lock for record " << map_it->first << " held by exiting client " << request.client_pid << std::endl;
			}
			else if (current_lock.state == LockInfo::READ_LOCKED) {
				for (size_t i = 0; i < current_lock.reader_pids.size(); ) {
					if (current_lock.reader_pids[i] == request.client_pid) {
						current_lock.reader_pids.erase(current_lock.reader_pids.begin() + i);
						std::cout << "Server: Removed read interest for record " << map_it->first << " from exiting client " << request.client_pid << std::endl;
					}
					else {
						++i;
					}
				}
				if (current_lock.reader_pids.empty()) {
					current_lock.state = LockInfo::UNLOCKED;
				}
			}
		}
		g_terminated_client_count++;
		response.type = OPERATION_SUCCESS;         break;
	}
	default:
		std::cerr << "Server: Unknown message type: " << request.type << std::endl;
		strncpy(response.error_message, "Unknown request type.", sizeof(response.error_message) - 1);
		response.type = OPERATION_FAIL_OTHER;
	}

	LeaveCriticalSection(&g_locks_critical_section);
	DWORD bytesWritten;
	if (!WriteFile(hClientResponsePipe, &response, sizeof(ResponseMessage), &bytesWritten, NULL)) {
		std::cerr << "Server Error: Failed to write response to client " << request.client_pid
			<< ". GLE: " << GetLastError() << std::endl;
	}
	else {
	}

	CloseHandle(hClientResponsePipe);
}


int main() {
	InitializeCriticalSection(&g_data_critical_section);
	InitializeCriticalSection(&g_locks_critical_section);

	std::cout << "Enter binary filename: ";
	std::cin >> g_binary_filename;
	std::cin.ignore();
	int num_employees_initial;
	std::cout << "Enter number of employees to add initially: ";
	std::cin >> num_employees_initial;
	std::cin.ignore();

	std::ofstream outfile(g_binary_filename.c_str(), std::ios::binary | std::ios::trunc);
	if (!outfile) {
		std::cerr << "Error: Cannot create/open file for writing: " << g_binary_filename << std::endl;
		DeleteCriticalSection(&g_data_critical_section);
		DeleteCriticalSection(&g_locks_critical_section);
		return 1;
	}

	for (int i = 0; i < num_employees_initial; ++i) {
		Employee emp;
		std::cout << "\nEmployee " << i + 1 << ":" << std::endl;
		std::cout << "  ID (integer): ";
		std::cin >> emp.num;
		std::cin.ignore();
		std::cout << "  Name (max " << NAME_MAX_LENGTH - 1 << " chars): ";
		std::cin.getline(emp.name, NAME_MAX_LENGTH);
		if (std::cin.fail() && !std::cin.eof()) {
			std::cin.clear();
			std::cin.ignore(10000, '\n');             emp.name[NAME_MAX_LENGTH - 1] = '\0';
		}

		std::cout << "  Hours worked: ";
		std::cin >> emp.hours;
		std::cin.ignore();
		outfile.write(reinterpret_cast<char*>(&emp), sizeof(Employee));
	}
	outfile.close();
	std::cout << "\nInitial binary file '" << g_binary_filename << "' created." << std::endl;

	if (!load_employees_and_init_locks()) {
		DeleteCriticalSection(&g_data_critical_section);
		DeleteCriticalSection(&g_locks_critical_section);
		return 1;
	}
	display_all_employees("Initial File Contents");

	std::cout << "\nEnter number of client processes to start: ";
	std::cin >> g_initial_client_count;
	std::cin.ignore();

	if (g_initial_client_count < 0) g_initial_client_count = 0;
	if (g_initial_client_count > 0) {
		for (int i = 0; i < g_initial_client_count; ++i) {
			STARTUPINFOA si;             PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));

			char client_cmd[] = "Client.exe";

			if (!CreateProcessA(NULL, client_cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
				std::cerr << "CreateProcess failed for client " << i + 1 << " (GLE: " << GetLastError() << ")." << std::endl;
			}
			else {
				std::cout << "Client " << i + 1 << " (PID: " << pi.dwProcessId << ") started." << std::endl;
				g_client_process_handles.push_back(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}
	}
	else {
		std::cout << "No clients to start." << std::endl;
	}


	if (g_initial_client_count > 0) {
		HANDLE hRequestPipe = CreateNamedPipe(
			SERVER_REQUEST_PIPE_NAME,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			1,
			sizeof(ResponseMessage),
			sizeof(RequestMessage),
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL
		);

		if (hRequestPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Fatal Error: CreateNamedPipe failed for server request pipe. GLE: " << GetLastError() << std::endl;
		}
		else {
			std::cout << "\nServer is listening for client connections on " << SERVER_REQUEST_PIPE_NAME << std::endl;

			while (g_terminated_client_count < g_initial_client_count) {
				std::cout << "Server: Waiting for a client to connect... ("
					<< g_terminated_client_count << "/" << g_initial_client_count << " clients terminated)" << std::endl;

				BOOL connected = ConnectNamedPipe(hRequestPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

				if (connected) {
					std::cout << "Server: Client connected to main request pipe." << std::endl;
					RequestMessage client_request;
					DWORD bytesRead;

					BOOL success = ReadFile(
						hRequestPipe,
						&client_request,
						sizeof(RequestMessage),
						&bytesRead,
						NULL
					);

					if (success && bytesRead == sizeof(RequestMessage)) {
						handle_client_request(hRequestPipe, client_request);
					}
					else {
						DWORD readError = GetLastError();
						if (bytesRead == 0 && success) {
							std::cerr << "Server: Client disconnected before sending full request (0 bytes read)." << std::endl;
						}
						else if (!success && readError == ERROR_BROKEN_PIPE) {
							std::cerr << "Server: Client connection broken before request read. GLE: " << readError << std::endl;
						}
						else {
							std::cerr << "Server: ReadFile from main request pipe failed or incomplete. GLE: " << readError << std::endl;
						}
					}
					DisconnectNamedPipe(hRequestPipe);
					std::cout << "Server: Disconnected client from main request pipe instance. Ready for new connection." << std::endl;
				}
				else {
					std::cerr << "Server: ConnectNamedPipe failed for main pipe. GLE: " << GetLastError() << std::endl;
					Sleep(100);
				}
			}
			CloseHandle(hRequestPipe);
			std::cout << "Server: All initially launched clients have sent EXITING. Main listen loop finished." << std::endl;
		}
	}


	if (!g_client_process_handles.empty()) {
		std::cout << "Server: Waiting for all client processes to fully exit..." << std::endl;
		WaitForMultipleObjects(static_cast<DWORD>(g_client_process_handles.size()), &g_client_process_handles[0], TRUE, INFINITE);
		for (size_t i = 0; i < g_client_process_handles.size(); ++i) {
			CloseHandle(g_client_process_handles[i]);
		}
		g_client_process_handles.clear();         std::cout << "Server: All client processes have exited." << std::endl;
	}

	if (!load_employees_and_init_locks()) {
		std::cerr << "Server: Failed to reload file before final display." << std::endl;
	}
	display_all_employees("Modified File Contents (After All Clients)");

	std::cout << "\nType 'exit' to terminate the server: ";
	std::string command;
	while (std::cin >> command && command != "exit") {
		std::cout << "Unknown command. Type 'exit' to terminate: ";
	}

	DeleteCriticalSection(&g_data_critical_section);
	DeleteCriticalSection(&g_locks_critical_section);
	std::cout << "Server shutting down." << std::endl;
	return 0;
}