#include "piecetable.h"
#include "low_overhead_bench.h"
#include <cstddef>
#include <iostream>
#include <random>
#include <string>

// Stress test for alternating insert/delete pattern
int main()
{
    AL::piece_table pt;
    const int NUM_CYCLES = 50000;
    const double cycles_per_sec = bench::estimate_cycles_per_second();

    std::cout << "\n--- Alternating Insert/Delete Stress Test ---" << std::endl;
    std::cout << "Testing rapid alternation between inserts and deletes" << std::endl;
    std::cout << "Cycles: " << NUM_CYCLES << std::endl;

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> char_dist('a', 'z');
    std::uniform_int_distribution<int> len_dist(5, 50);

    uint64_t start = bench::rdtsc();

    for (int cycle = 0; cycle < NUM_CYCLES; ++cycle)
    {
        // Insert phase: add random text
        int insert_len = len_dist(rng);
        std::string text;
        text.reserve(insert_len);
        for (int i = 0; i < insert_len; ++i)
        {
            text += static_cast<char>(char_dist(rng));
        }

        size_t pos = pt.length() > 0 ? (rng() % pt.length()) : 0;
        pt.insert(pos, text);

        // Delete phase: remove some text
        if (pt.length() > 0)
        {
            size_t delete_len = std::min(len_dist(rng), static_cast<int>(pt.length()));
            size_t del_pos = pt.length() > delete_len ? (rng() % (pt.length() - delete_len)) : 0;
            pt.remove(del_pos, delete_len);
        }

        if ((cycle + 1) % 10000 == 0)
        {
            std::cout << "  Progress: " << (cycle + 1) << " cycles, length=" << pt.length() << std::endl;
        }
    }

    uint64_t end = bench::rdtsc();
    uint64_t cycles = end - start;

    std::cout << "\n[Results]" << std::endl;
    std::cout << "Total cycles: " << cycles << std::endl;
    std::cout << "Total time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(cycles), cycles_per_sec)) << std::endl;
    const double cycles_per_cycle = bench::cycles_per_op(cycles, NUM_CYCLES);
    std::cout << "Avg per cycle: " << cycles_per_cycle << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(cycles_per_cycle, cycles_per_sec)) << ")" << std::endl;
    std::cout << "Final length: " << pt.length() << std::endl;

    // Sanity check: ensure we can still retrieve the string
    uint64_t retrieve_start = bench::rdtsc();
    std::string result = pt.to_string();
    uint64_t retrieve_end = bench::rdtsc();
    uint64_t retrieve_cycles = retrieve_end - retrieve_start;

    std::cout << "String retrieval cycles: " << retrieve_cycles << std::endl;
    std::cout << "String retrieval time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(retrieve_cycles), cycles_per_sec)) << std::endl;
    const double cycles_per_char = bench::cycles_per_op(retrieve_cycles, result.length());
    std::cout << "Cycles per char: " << cycles_per_char
              << " (" << bench::format_seconds(bench::cycles_to_seconds(cycles_per_char, cycles_per_sec)) << ")" << std::endl;

    if (result.length() != pt.length())
    {
        std::cerr << "ERROR: Retrieved string length doesn't match!" << std::endl;
        return 1;
    }

    std::cout << "\n[PASSED] Alternating insert/delete stress test" << std::endl;
    return 0;
}
