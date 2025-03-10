#include <gtest/gtest.h>
#include <Employee.h>
#include <fstream>
#include <string>

// Пример теста для проверки создания бинарного файла
TEST(CreatorTest, CreateEmployeeRecords) {
    std::string bin_file_name = "test_employees.bin";
    int num_records = 2;

    // Создаем тестовые данные
    employee emp1 = { 1, "John", 40.0 };
    employee emp2 = { 2, "Jane", 35.5 };

    // Записываем данные в файл
    std::ofstream bf(bin_file_name, std::ios::binary | std::ios::out);
    bf.write(reinterpret_cast<char*>(&emp1), sizeof(employee));
    bf.write(reinterpret_cast<char*>(&emp2), sizeof(employee));
    bf.close();

    // Проверяем, что файл создан и содержит правильные данные
    std::ifstream in_file(bin_file_name, std::ios::binary | std::ios::in);
    employee emp;
    in_file.read(reinterpret_cast<char*>(&emp), sizeof(employee));
    EXPECT_EQ(emp.num, 1);
    EXPECT_STREQ(emp.name, "John");
    EXPECT_EQ(emp.hours, 40.0);

    in_file.read(reinterpret_cast<char*>(&emp), sizeof(employee));
    EXPECT_EQ(emp.num, 2);
    EXPECT_STREQ(emp.name, "Jane");
    EXPECT_EQ(emp.hours, 35.5);

    in_file.close();
}

// Пример теста для проверки генерации отчета
TEST(ReporterTest, GenerateReport) {
    std::string bin_file_name = "test_employees.bin";
    std::string report_file_name = "test_report.txt";
    int price_per_hour = 10;

    // Создаем тестовые данные
    employee emp1 = { 1, "John", 40.0 };
    employee emp2 = { 2, "Jane", 35.5 };

    // Записываем данные в файл
    std::ofstream bf(bin_file_name, std::ios::binary | std::ios::out);
    bf.write(reinterpret_cast<char*>(&emp1), sizeof(employee));
    bf.write(reinterpret_cast<char*>(&emp2), sizeof(employee));
    bf.close();

    // Генерируем отчет
    std::ofstream out_file(report_file_name, std::ios::out);
    out_file << "Report on binary file " << bin_file_name << std::endl;
    out_file << "ID: " << emp1.num << " Name: " << emp1.name << " Working hours: "
        << emp1.hours << " Wage: " << emp1.hours * price_per_hour << std::endl;
    out_file << "ID: " << emp2.num << " Name: " << emp2.name << " Working hours: "
        << emp2.hours << " Wage: " << emp2.hours * price_per_hour << std::endl;
    out_file.close();

    // Проверяем, что отчет создан и содержит правильные данные
    std::ifstream in_file(report_file_name, std::ios::in);
    std::string line;
    std::getline(in_file, line);
    EXPECT_EQ(line, "Report on binary file test_employees.bin");

    std::getline(in_file, line);
    EXPECT_EQ(line, "ID: 1 Name: John Working hours: 40 Wage: 400");

    std::getline(in_file, line);
    EXPECT_EQ(line, "ID: 2 Name: Jane Working hours: 35.5 Wage: 355");

    in_file.close();
}

// Удаляем функцию main, так как она предоставляется GTest::gtest_main