#include "piecetable.h"
#include "alias.h"
#include <iostream>
#include <random>
#include <vector>

int main()
{
    // Sequential inserts (like bench_to_string)
    {
        AL::piece_table pt;
        for (int i = 1; i <= 10000; ++i)
        {
            pt.insert(pt.length(), "Line " + std::to_string(i) + "\n");
        }
        
        std::vector<AL::piece> pieces;
        pt.get_pieces(pieces);
        std::cout << "Sequential 10k inserts: " << pieces.size() << " pieces, " 
                  << pt.length() << " bytes" << std::endl;
    }
    
    // Random edits (like stress_random_edits, smaller scale)
    {
        AL::piece_table pt;
        std::mt19937 rng(42);
        
        for (int i = 0; i < 100000; ++i)
        {
            size_t pos = pt.length() > 0 ? (rng() % pt.length()) : 0;
            pt.insert(pos, "x");
            
            if (pt.length() > 10 && rng() % 2)
            {
                size_t del_pos = rng() % (pt.length() - 1);
                pt.remove(del_pos, 1);
            }
        }
        
        std::vector<AL::piece> pieces;
        pt.get_pieces(pieces);
        std::cout << "Random 100k edits: " << pieces.size() << " pieces, " 
                  << pt.length() << " bytes" << std::endl;
    }
    
    return 0;
}
