#include "piecetable.h"
#include "low_overhead_bench.h"
#include <iostream>
#include <random>
#include <string>

// Stress test for handling many newlines and line operations
int main()
{
    AL::piece_table pt;
    const int NUM_LINES = 100000;
    const double cycles_per_sec = bench::estimate_cycles_per_second();

    std::cout << "\n--- Newline Heavy Stress Test ---" << std::endl;
    std::cout << "Building file with " << NUM_LINES << " lines..." << std::endl;

    uint64_t start_build = bench::rdtsc();

    // Create a file with many lines
    for (int i = 0; i < NUM_LINES; ++i)
    {
        std::string line = "This is line number " + std::to_string(i + 1) + "\n";
        pt.insert(pt.length(), line);

        if ((i + 1) % 20000 == 0)
        {
            std::cout << "  Built " << (i + 1) << " lines..." << std::endl;
        }
    }

    uint64_t end_build = bench::rdtsc();
    uint64_t build_cycles = end_build - start_build;

    std::cout << "\n[Build Statistics]" << std::endl;
    std::cout << "Build cycles: " << build_cycles << std::endl;
    std::cout << "Build time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(build_cycles), cycles_per_sec)) << std::endl;
    const double build_avg_cycles = bench::cycles_per_op(build_cycles, NUM_LINES);
    std::cout << "Avg per insert: " << build_avg_cycles << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(build_avg_cycles, cycles_per_sec)) << ")" << std::endl;
    std::cout << "Line count: " << pt.get_line_count() << std::endl;
    std::cout << "Total length: " << pt.length() << " bytes" << std::endl;

    // Test random line access
    std::cout << "\nTesting random line access..." << std::endl;
    std::mt19937 rng(54321);
    std::uniform_int_distribution<size_t> line_dist(1, pt.get_line_count());

    const int NUM_ACCESSES = 10000;
    uint64_t start_access = bench::rdtsc();

    for (int i = 0; i < NUM_ACCESSES; ++i)
    {
        size_t line_num = line_dist(rng);
        std::string line = pt.get_line(line_num);

        // Verify the line content
        std::string expected = "This is line number " + std::to_string(line_num);
        if (line != expected)
        {
            std::cerr << "ERROR: Line " << line_num << " mismatch!" << std::endl;
            std::cerr << "Expected: " << expected << std::endl;
            std::cerr << "Got: " << line << std::endl;
            return 1;
        }
    }

    uint64_t end_access = bench::rdtsc();
    uint64_t access_cycles = end_access - start_access;

    std::cout << "\n[Line Access Statistics]" << std::endl;
    std::cout << "Total accesses: " << NUM_ACCESSES << std::endl;
    std::cout << "Access cycles: " << access_cycles << std::endl;
    std::cout << "Access time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(access_cycles), cycles_per_sec)) << std::endl;
    const double access_avg_cycles = bench::cycles_per_op(access_cycles, NUM_ACCESSES);
    std::cout << "Avg per access: " << access_avg_cycles << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(access_avg_cycles, cycles_per_sec)) << ")" << std::endl;

    // Test get_index_for_line
    std::cout << "\nTesting get_index_for_line..." << std::endl;
    const int NUM_INDEX_TESTS = 1000;
    uint64_t start_index = bench::rdtsc();

    for (int i = 0; i < NUM_INDEX_TESTS; ++i)
    {
        size_t line_num = line_dist(rng);
        size_t index = pt.get_index_for_line(line_num);

        // Verify by checking the character at that position
        if (index < pt.length())
        {
            char c = pt.get_char_at(index);
            if (line_num == 1 && c != 'T')
            {
                std::cerr << "ERROR: Index for line 1 doesn't point to 'T'" << std::endl;
                return 1;
            }
        }
    }

    uint64_t end_index = bench::rdtsc();
    uint64_t index_cycles = end_index - start_index;

    std::cout << "\n[Index Lookup Statistics]" << std::endl;
    std::cout << "Total lookups: " << NUM_INDEX_TESTS << std::endl;
    std::cout << "Lookup cycles: " << index_cycles << std::endl;
    std::cout << "Lookup time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(index_cycles), cycles_per_sec)) << std::endl;
    const double lookup_avg_cycles = bench::cycles_per_op(index_cycles, NUM_INDEX_TESTS);
    std::cout << "Avg per lookup: " << lookup_avg_cycles << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(lookup_avg_cycles, cycles_per_sec)) << ")" << std::endl;

    // Now stress test by inserting newlines in the middle
    std::cout << "\nInserting newlines in the middle..." << std::endl;
    const int NUM_NEWLINE_INSERTS = 1000;
    std::uniform_int_distribution<size_t> pos_dist(0, pt.length());

    uint64_t start_newline = bench::rdtsc();

    for (int i = 0; i < NUM_NEWLINE_INSERTS; ++i)
    {
        size_t pos = pos_dist(rng);
        pt.insert(pos, "\n");
    }

    uint64_t end_newline = bench::rdtsc();
    uint64_t newline_cycles = end_newline - start_newline;

    std::cout << "\n[Newline Insert Statistics]" << std::endl;
    std::cout << "Newlines inserted: " << NUM_NEWLINE_INSERTS << std::endl;
    std::cout << "Insert cycles: " << newline_cycles << std::endl;
    std::cout << "Insert time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(newline_cycles), cycles_per_sec)) << std::endl;
    const double newline_avg_cycles = bench::cycles_per_op(newline_cycles, NUM_NEWLINE_INSERTS);
    std::cout << "Avg per insert: " << newline_avg_cycles << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(newline_avg_cycles, cycles_per_sec)) << ")" << std::endl;
    std::cout << "New line count: " << pt.get_line_count() << std::endl;

    std::cout << "\n[PASSED] Newline heavy stress test" << std::endl;
    return 0;
}
