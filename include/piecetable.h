#pragma once

#include "implicit_treap.h"
#include <string>

/*
 * Piece table created using an Implicit Treap
 */
class piece_table
{
private:

    std::string m_original_buffer;
    std::string m_add_buffer;
    implicit_treap m_treap;

public:

    piece_table();
    ~piece_table();

    piece_table(const std::string initial_content);
    void insert(size_t position, const std::string& text);
    void remove(size_t position, size_t length);
    std::string to_string() const;
    size_t length() const;
};
