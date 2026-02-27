#include "piecetable.h"
#include <chrono>
#include <iostream>
#include <random>
#include <string>

// Stress test for handling many newlines and line operations
int main()
{
    AL::piece_table pt;
    const int NUM_LINES = 100000;

    std::cout << "\n--- Newline Heavy Stress Test ---" << std::endl;
    std::cout << "Building file with " << NUM_LINES << " lines..." << std::endl;

    auto start_build = std::chrono::high_resolution_clock::now();

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

    auto end_build = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> build_time = end_build - start_build;

    std::cout << "\n[Build Statistics]" << std::endl;
    std::cout << "Build time: " << build_time.count() << " s" << std::endl;
    std::cout << "Line count: " << pt.get_line_count() << std::endl;
    std::cout << "Total length: " << pt.length() << " bytes" << std::endl;

    // Test random line access
    std::cout << "\nTesting random line access..." << std::endl;
    std::mt19937 rng(54321);
    std::uniform_int_distribution<size_t> line_dist(1, pt.get_line_count());

    const int NUM_ACCESSES = 10000;
    auto start_access = std::chrono::high_resolution_clock::now();

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

    auto end_access = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> access_time = end_access - start_access;

    std::cout << "\n[Line Access Statistics]" << std::endl;
    std::cout << "Total accesses: " << NUM_ACCESSES << std::endl;
    std::cout << "Access time: " << access_time.count() << " s" << std::endl;
    std::cout << "Avg per access: " << (access_time.count() * 1e6 / NUM_ACCESSES) << " us" << std::endl;

    // Test get_index_for_line
    std::cout << "\nTesting get_index_for_line..." << std::endl;
    const int NUM_INDEX_TESTS = 1000;
    auto start_index = std::chrono::high_resolution_clock::now();

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

    auto end_index = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> index_time = end_index - start_index;

    std::cout << "\n[Index Lookup Statistics]" << std::endl;
    std::cout << "Total lookups: " << NUM_INDEX_TESTS << std::endl;
    std::cout << "Lookup time: " << index_time.count() << " s" << std::endl;
    std::cout << "Avg per lookup: " << (index_time.count() * 1e6 / NUM_INDEX_TESTS) << " us" << std::endl;

    // Now stress test by inserting newlines in the middle
    std::cout << "\nInserting newlines in the middle..." << std::endl;
    const int NUM_NEWLINE_INSERTS = 1000;
    std::uniform_int_distribution<size_t> pos_dist(0, pt.length());

    auto start_newline = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_NEWLINE_INSERTS; ++i)
    {
        size_t pos = pos_dist(rng);
        pt.insert(pos, "\n");
    }

    auto end_newline = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> newline_time = end_newline - start_newline;

    std::cout << "\n[Newline Insert Statistics]" << std::endl;
    std::cout << "Newlines inserted: " << NUM_NEWLINE_INSERTS << std::endl;
    std::cout << "Insert time: " << newline_time.count() << " s" << std::endl;
    std::cout << "New line count: " << pt.get_line_count() << std::endl;

    std::cout << "\n[PASSED] Newline heavy stress test" << std::endl;
    return 0;
}
