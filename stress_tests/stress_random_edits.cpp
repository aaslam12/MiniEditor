#include "piecetable.h"
#include "low_overhead_bench.h"
#include <iostream>
#include <random>
#include <string>

int main()
{
    AL::piece_table pt;
    const int NUM_OPERATIONS = 500'000;
    const int INITIAL_SIZE = 1'000'000;
    const double cycles_per_sec = bench::estimate_cycles_per_second();

    std::cout << "\n--- Random Edits Stress Test ---" << std::endl;
    std::cout << "Initializing with " << INITIAL_SIZE << " characters..." << std::endl;

    // Fill with some initial data
    std::string initial_data(INITIAL_SIZE, 'A');
    pt.insert(0, initial_data);

    std::mt19937 rng(42);                             // Fixed seed for reproducibility
    std::uniform_int_distribution<int> op_dist(0, 1); // 0 = Insert, 1 = Delete
    std::uniform_int_distribution<int> char_dist('a', 'z');
    std::uniform_int_distribution<int> len_dist(1, 100);

    std::cout << "Performing " << NUM_OPERATIONS << " random insertions/deletions..." << std::endl;

    uint64_t start = bench::rdtsc();

    for (int i = 0; i < NUM_OPERATIONS; ++i)
    {
        size_t current_len = pt.length();
        int op = op_dist(rng);

        if (op == 0)
        { // INSERT
            // Create random string
            int str_len = len_dist(rng);
            std::string s;
            s.reserve(str_len);
            for (int k = 0; k < str_len; ++k)
                s += (char)char_dist(rng);

            // Random position
            std::uniform_int_distribution<size_t> pos_dist(0, current_len);
            size_t pos = pos_dist(rng);

            pt.insert(pos, s);
        }
        else
        { // DELETE
            if (current_len == 0)
                continue;

            // Random position and length
            std::uniform_int_distribution<size_t> pos_dist(0, current_len - 1);
            size_t pos = pos_dist(rng);

            // Don't delete past end
            size_t max_del = current_len - pos;
            std::uniform_int_distribution<size_t> del_len_dist(1, std::min((size_t)100, max_del));
            size_t len = del_len_dist(rng);

            pt.remove(pos, len);
        }
    }

    uint64_t end = bench::rdtsc();
    uint64_t cycles = end - start;

    std::cout << "\n[Random Edit Statistics]" << std::endl;
    std::cout << "Total cycles:     " << cycles << std::endl;
    std::cout << "Total time:       " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(cycles), cycles_per_sec)) << std::endl;
    const double avg_edit_cycles = bench::cycles_per_op(cycles, NUM_OPERATIONS);
    std::cout << "Avg per Edit:     " << avg_edit_cycles << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(avg_edit_cycles, cycles_per_sec)) << ")" << std::endl;
    std::cout << "Final Size:       " << pt.length() << " characters" << std::endl;

    // Sanity check: Retrieve the string to ensure the tree isn't broken
    // (This traverses the whole tree)
    uint64_t string_start = bench::rdtsc();
    std::string final_str = pt.to_string();
    uint64_t string_end = bench::rdtsc();
    uint64_t string_cycles = string_end - string_start;

    std::cout << "Reconstruction:   " << string_cycles << " cycles" << std::endl;
    std::cout << "Reconstruction time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(string_cycles), cycles_per_sec)) << std::endl;
    const double cycles_per_char = bench::cycles_per_op(string_cycles, final_str.length());
    std::cout << "Cycles per char:  " << cycles_per_char
              << " (" << bench::format_seconds(bench::cycles_to_seconds(cycles_per_char, cycles_per_sec)) << ")" << std::endl;
    std::cout << "Check: String length matches tree size? " << (final_str.length() == pt.length() ? "YES" : "NO") << std::endl;

    return 0;
}
