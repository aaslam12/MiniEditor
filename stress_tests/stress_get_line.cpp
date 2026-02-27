#include "piecetable.h"
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

int main()
{
    AL::piece_table pt("Initial line\n");
    std::vector<std::string> expected_lines;
    expected_lines.push_back("Initial line");

    const int num_iterations = 10000;
    std::cout << "Building piece table with " << num_iterations << " pieces..." << std::endl;

    for (int i = 0; i < num_iterations; ++i)
    {
        std::string new_line = "Line " + std::to_string(i + 1);
        pt.insert(pt.length(), new_line + "\n");
        expected_lines.push_back(new_line);
    }

    std::cout << "Piece table length: " << pt.length() << " bytes." << std::endl;
    std::cout << "Line count: " << pt.get_line_count() << std::endl;

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> dist(1, pt.get_line_count());

    const int num_reads = 50000;
    std::cout << "Performing " << num_reads << " random get_line reads..." << std::endl;

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < num_reads; ++i)
    {
        size_t line_num = dist(rng);
        std::string line = pt.get_line(line_num);
        if (line != expected_lines[line_num - 1])
        {
            std::cerr << "Mismatch at line " << line_num << "!" << std::endl;
            std::cerr << "Expected: " << expected_lines[line_num - 1] << std::endl;
            std::cerr << "Got: " << line << std::endl;
            return 1;
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Stress test passed!" << std::endl;
    std::cout << "Total time for " << num_reads << " reads: " << diff.count() << "s" << std::endl;
    std::cout << "Average time per read: " << (diff.count() / num_reads) * 1e6 << "us" << std::endl;

    return 0;
}
