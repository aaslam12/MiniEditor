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
    size_t count_newlines(const piece& p) const;
    size_t count_newlines(const std::string& str) const;

#if MINIEDITOR_TESTING
public:
#endif // MINIEDITOR_TESTING
    auto get_split_strategy()
    {
        return [this](piece& left, size_t split_offset) {
            piece right_piece = {.buf_type = left.buf_type,
                                 .start = left.start + split_offset,
                                 .length = left.length - split_offset,
                                 .newline_count = 0 /* will be set later */};

            size_t count;
            auto old_piece_length = left.length;
            left.length = split_offset;

            if (split_offset < (old_piece_length / 2))
            {
                // then we count the left side since it is shorter
                count = count_newlines(left);
                right_piece.newline_count = left.newline_count - count;
            }
            else
            {
                // right side is shorter
                count = count_newlines(right_piece);
                right_piece.newline_count = count;
            }

            left.newline_count -= right_piece.newline_count;
            return right_piece;
        };
    }

public:
    piece_table();
    piece_table(piece_table&& other) noexcept;
    piece_table& operator=(piece_table&& other) noexcept;
    ~piece_table();

    piece_table(const std::string initial_content);
    void insert(size_t position, std::string text);
    void remove(size_t position, size_t length);
    void clear();
    size_t get_index_for_line(size_t target_line) const;
    std::string to_string() const;
    std::string get_line(size_t line_number) const;
    size_t length() const;
    size_t get_line_count() const;
    char get_char_at(size_t byte_index) const;
    size_t get_line_length(size_t line_number) const;
};
} // namespace AL
