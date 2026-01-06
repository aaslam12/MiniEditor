#include "piecetable.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/resource.h>
#include <vector>

double get_memory_usage()
{
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        return static_cast<double>(usage.ru_maxrss) / 1024.0;
    }
    return 0.0;
}

int main()
{
    AL::piece_table pt;
    const int num_lines = 10000000; // 10 Million

    std::cout << "Target: " << num_lines << " pieces (10 million)" << std::endl;

    double mem_before = get_memory_usage();

    auto start_build = std::chrono::high_resolution_clock::now();
    for (int i = 1; i <= num_lines; ++i)
    {
        // This measures the time to append to buffer AND insert into the Treap
        pt.insert(pt.length(), "Line " + std::to_string(i) + "\n");

        if (i % 2000000 == 0)
        {
            std::cout << "  Progress: " << i << " pieces inserted..." << std::endl;
        }
    }
    auto end_build = std::chrono::high_resolution_clock::now();

    double mem_after = get_memory_usage();
    std::chrono::duration<double> build_diff = end_build - start_build;

    size_t total_chars = pt.length();
    double total_mb = static_cast<double>(total_chars) / (1024.0 * 1024.0);
    double throughput = total_mb / build_diff.count();

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n[Tree Insertion Statistics]" << std::endl;
    std::cout << "Total Insertion Time: " << build_diff.count() << " s" << std::endl;
    std::cout << "Avg per Insertion:    " << (build_diff.count() * 1e6 / num_lines) << " microseconds" << std::endl;
    std::cout << "Insertion Throughput: " << throughput << " MB/s" << std::endl;

    std::cout << "\n[Memory Statistics]" << std::endl;
    std::cout << "Total Characters:     " << total_chars << " (" << total_mb << " MB)" << std::endl;
    std::cout << "Total RAM Used:       " << (mem_after - mem_before) << " MB" << std::endl;

    const int target = num_lines / 2;
    std::cout << "\n[Search Statistics]" << std::endl;
    std::cout << "Searching for index of line " << target << "..." << std::endl;

    auto start_search = std::chrono::high_resolution_clock::now();
    size_t index = pt.get_index_for_line(target);
    auto end_search = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> search_diff = end_search - start_search;

    std::cout << "Found index:          " << index << std::endl;
    std::cout << "Search time:          " << search_diff.count() * 1000.0 << " ms" << std::endl;

    return 0;
}
