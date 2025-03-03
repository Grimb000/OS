#include <iostream>
#include <fstream>
#include <conio.h>
#include <Employee.h>

void generate_report(const std::string& bin_file_name, const std::string& report_file_name, int price_per_hour) {
    std::fstream bf(bin_file_name, std::ios::binary | std::ios::in);
    std::fstream out_file(report_file_name, std::ios::out);

    if (!bf) {
        throw std::runtime_error("Error! Binary file cannot be opened!");
    }

    out_file << "Report on binary file " << bin_file_name << std::endl;

    employee emp;
    while (bf.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        out_file << "ID: " << emp.num << " Name: " << emp.name << " Working hours: "
            << emp.hours << " Wage: " << emp.hours * price_per_hour << std::endl;
    }
    bf.close();
    out_file.close();
}

int main(int argc, char* argv[]) {
    std::cout << "Reporter started\n";

    if (argc != 4) {
        std::cerr << "reporter.cpp received invalid number of arguments";
        return 1;
    }

    std::string bin_file_name = argv[1];
    std::string report_file_name = argv[2];
    int price_per_hour = std::atoi(argv[3]);

    try {
        generate_report(bin_file_name, report_file_name, price_per_hour);
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    std::cout << "reporter finished successfully\n";
    return 0;
}