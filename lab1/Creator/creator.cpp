#include <iostream>
#include <fstream>
#include <conio.h>
#include <Employee.h>

void create_employee_records(const std::string& bin_file_name, int num_records) {
    std::fstream bf(bin_file_name, std::ios::binary | std::ios::out);

    if (!bf) {
        throw std::runtime_error("Unable to create file");
    }

    for (int i = 0; i < num_records; i++) {
        employee emp;
        std::string name;
        std::cout << "Enter ID\n";
        std::cin >> emp.num;
        std::cout << "Enter name\n";
        std::cin >> name;
        std::cout << "Enter number of hours\n";
        std::cin >> emp.hours;
        strncpy_s(emp.name, name.c_str(), sizeof(emp.name) - 1);
        bf.write(reinterpret_cast<char*>(&emp), sizeof(emp));
    }
    bf.close();
}

int main(int argc, char* argv[]) {
    std::cout << "Creator started\n";

    if (argc != 3) {
        std::cerr << "creator.cpp received invalid number of arguments";
        return 1;
    }

    std::string bin_file_name = argv[1];
    int num_records = std::atoi(argv[2]);

    try {
        create_employee_records(bin_file_name, num_records);
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    std::cout << "creator finished successfully\n";
    return 0;
}