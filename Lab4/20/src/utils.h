#ifndef UTILS_MODERN_H
#define UTILS_MODERN_H

#include <windows.h>
#include <memory>      
#include <stdexcept>  
#include <string>     
#include <system_error>
#include <iostream>    
struct HandleDeleter {
	using pointer = HANDLE;
	void operator()(HANDLE handle) const {
		if (handle != NULL && handle != INVALID_HANDLE_VALUE) {
			if (!CloseHandle(handle)) {
			}
		}
	}
};

using unique_handle = std::unique_ptr<void, HandleDeleter>;
class WinApiException : public std::runtime_error {
public:
	WinApiException(const std::string& message, DWORD errorCode = GetLastError())
		: std::runtime_error(build_full_message(message, errorCode)),
		errorCode_(errorCode) {
	}

	[[nodiscard]] DWORD get_error_code() const noexcept {
		return errorCode_;
	}

private:
	DWORD errorCode_;

	static std::string build_full_message(const std::string& baseMessage, DWORD errorCode) {
		return baseMessage + " (Windows Error Code: " + std::to_string(errorCode) + ")";
	}
};

[[nodiscard]] inline std::pair<unique_handle, unique_handle> create_process_cpp(
	const std::string& applicationName,
	const std::string& commandLine) {

	STARTUPINFOA si = {};     si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {};

	std::vector<char> cmdLineVec(commandLine.begin(), commandLine.end());
	cmdLineVec.push_back('\0');
	LPCSTR appNameLPCSTR = applicationName.empty() ? NULL : applicationName.c_str();

	if (!CreateProcessA(
		appNameLPCSTR, cmdLineVec.data(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		throw WinApiException("Failed to create process for command: " + commandLine);
	}
	return { unique_handle(pi.hProcess), unique_handle(pi.hThread) };
}


#endif 