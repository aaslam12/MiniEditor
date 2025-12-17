#pragma once

#include "red_black_tree.h"
#include <string>

/*
 * Piece table created using Red Black trees.
 */
class piece_table
{
public:

    piece_table();
    piece_table(piece_table&&) = default;
    piece_table(const piece_table&) = default;
    piece_table& operator=(piece_table&&) = default;
    piece_table& operator=(const piece_table&) = default;
    ~piece_table();

    piece_table(const std::string& initial_content);
    void insert(size_t position, const std::string& text);
    void remove(size_t position, size_t length);
    std::string to_string() const;
    size_t length() const;

private:

    std::string original_buffer;
    std::string add_buffer;
    RBT<std::string> rbtree;
};
