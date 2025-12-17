#include "piecetable.h"
#include "red_black_tree.h"
#include <iostream>

int main(int argc, char* argv[])
{
    RBT<int> rbt;
    rbt.insert(5);
    // rbt.left_rotate(rbt.root.get());

    piece_table pt;
    pt.length();

    std::cout << "Hello World!\n";
    std::cout << "Hello World!\n";
    return 0;
}
