#pragma once

#include "implicit_treap.h"
#include <cstddef>
#include <string>

namespace AL
{

/*
 * Piece table created using an Implicit Treap
 */
class piece_table
{
private:

    std::string m_original_buffer;
    std::string m_add_buffer;
    AL::implicit_treap m_treap;

    // normalizes string IN PLACE
    // replaces OS specific line endings with '\n'
    void normalize(std::string& text);

public:

    piece_table();
    ~piece_table();

    piece_table(const std::string initial_content);
    void insert(size_t position, std::string text);
    void remove(size_t position, size_t length);
    void clear();
    size_t get_index_for_line(size_t target_line) const;
    std::string to_string() const;
    size_t length() const;
};
} // namespace AL
