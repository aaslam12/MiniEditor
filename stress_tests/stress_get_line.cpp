#include "piecetable.h"
#include "alias.h"
#include "low_overhead_bench.h"
#include <iostream>
#include <random>
#include <string>

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
    const double cycles_per_sec = bench::estimate_cycles_per_second();
    std::cout << "Performing " << num_reads << " random get_line reads..." << std::endl;

    uint64_t start = bench::rdtsc();
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
    uint64_t end = bench::rdtsc();
    uint64_t cycles = end - start;

    std::cout << "Stress test passed!" << std::endl;
    std::cout << "Total cycles for " << num_reads << " reads: " << cycles << std::endl;
    std::cout << "Total time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(cycles), cycles_per_sec)) << std::endl;
    const double avg_read_cycles = bench::cycles_per_op(cycles, num_reads);
    std::cout << "Average cycles per read: " << avg_read_cycles
              << " (" << bench::format_seconds(bench::cycles_to_seconds(avg_read_cycles, cycles_per_sec)) << ")" << std::endl;

    return 0;
}
