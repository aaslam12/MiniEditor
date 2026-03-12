#include "piecetable.h"
#include <chrono>
#include <iostream>

int main()
{
    // Test 1: stress_newlines scenario (100k lines, ~2.5 MB)
    {
        AL::piece_table pt;
        for (int i = 0; i < 100000; ++i)
        {
            std::string line = "This is line number " + std::to_string(i + 1) + "\n";
            pt.insert(pt.length(), line);
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        std::string result = pt.to_string();
        auto end = std::chrono::high_resolution_clock::now();
        
        double ms = std::chrono::duration<double>(end - start).count() * 1000.0;
        double mb = result.length() / 1024.0 / 1024.0;
        double throughput = mb / (ms / 1000.0);
        
        std::cout << "100k lines (~" << mb << " MB):" << std::endl;
        std::cout << "  to_string() time: " << ms << " ms" << std::endl;
        std::cout << "  Throughput: " << throughput << " MB/s" << std::endl;
    }
    
    // Test 2: smaller scale of stress_get_index (1M pieces, ~12 MB)
    {
        AL::piece_table pt;
        for (int i = 1; i <= 1000000; ++i)
        {
            pt.insert(pt.length(), "Line " + std::to_string(i) + "\n");
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        std::string result = pt.to_string();
        auto end = std::chrono::high_resolution_clock::now();
        
        double ms = std::chrono::duration<double>(end - start).count() * 1000.0;
        double mb = result.length() / 1024.0 / 1024.0;
        double throughput = mb / (ms / 1000.0);
        
        std::cout << "\n1M pieces (~" << mb << " MB):" << std::endl;
        std::cout << "  to_string() time: " << ms << " ms" << std::endl;
        std::cout << "  Throughput: " << throughput << " MB/s" << std::endl;
    }
    
    return 0;
}
