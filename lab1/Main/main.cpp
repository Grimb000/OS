#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>
#include <conio.h>
#include <Employee.h>

void print_report(const std::string& report_file_name) {
    std::cout << "Here is your report\n";
    std::fstream rep(report_file_name, std::ios::in);
    if (!rep)
        throw std::exception("Error! Unable to open file and print report");
    std::string str;
    while (std::getline(rep, str)) {
        std::cout << str << std::endl;
    }
    rep.close();
}

void print_bin_file(const std::string& bin_file_name) {
    std::cout << "Here is the content of the binary file\n";
    std::fstream bf(bin_file_name, std::ios::binary | std::ios::in);
    if (!bf)
        throw std::exception("Error! Unable to open and print binary file");

    employee emp;
    while (bf.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        std::cout << "ID: " << emp.num << " Name: " << emp.name << " Working hours: "
            << emp.hours << std::endl;
    }
    bf.close();
}

void start_process(const std::string& command_line) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    if (!CreateProcess(NULL, const_cast<char*>(command_line.c_str()), NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        throw std::exception("Creator process is not created. Check a name of the process.");
    }
    WaitForSingleObject(pi.hThread, INFINITE);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

int main() {
    std::string bin_file_name;
    int num_records;

    std::cout << "Enter binary file name\n";
    std::cin >> bin_file_name;

    std::cout << "Enter number of records\n";
    std::cin >> num_records;

    std::string creator_command_line = "creator.exe " + bin_file_name + " " + std::to_string(num_records);

    try {
        start_process(creator_command_line);
    }
    catch (const std::exception& e) {
        std::cerr << "Creator started with error: " << e.what();
        return 1;
    }

    try {
        print_bin_file(bin_file_name);
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    std::string report_file_name;
    int price_per_hour;

    std::cout << "Enter report file name\n";
    std::cin >> report_file_name;

    std::cout << "Enter price per hour\n";
    std::cin >> price_per_hour;

    std::string reporter_command_line = "reporter.exe " + bin_file_name + " "
        + report_file_name + " " + std::to_string(price_per_hour);

    try {
        start_process(reporter_command_line);
    }
    catch (const std::exception& e) {
        std::cerr << "Reporter started with error: " << e.what();
        return 1;
    }

    try {
        print_report(report_file_name);
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    std::cout << "program finished successfully\n";
    std::cout << "press any key to continue...\n";

    _getch();
    return 0;
}