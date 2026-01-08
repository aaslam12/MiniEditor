#include "piecetable.h"
#include "implicit_treap.h"
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <strings.h>

namespace AL
{
void piece_table::normalize(std::string& text)
{
    text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char c) { return c == '\r'; }), text.end());
}

size_t piece_table::count_newlines(const piece& p) const
{
    size_t count = 0;
    if (p.buf_type == buffer_type::ORIGINAL)
    {
        std::string_view str(m_original_buffer.data() + p.start, p.length);
        count = std::count(str.begin(), str.end(), '\n');
    }
    else if (p.buf_type == buffer_type::ADD)
    {
        std::string_view str(m_add_buffer.data() + p.start, p.length);
        count = std::count(str.begin(), str.end(), '\n');
    }

    return count;
}

size_t piece_table::count_newlines(const std::string& str) const
{
    return std::count(str.begin(), str.end(), '\n');
}

piece_table::piece_table()
{}

piece_table::~piece_table()
{}

piece_table::piece_table(std::string initial_content)
{
    // normalize the content before doing anything
    normalize(initial_content);
    m_original_buffer = std::move(initial_content);

    AL::piece piece;
    piece.length = m_original_buffer.length();
    piece.buf_type = AL::buffer_type::ORIGINAL;
    piece.start = 0;
    piece.newline_count = count_newlines(initial_content);

    m_treap.insert(0, piece, get_split_strategy());
}

void piece_table::insert(size_t file_insert_position, std::string text)
{
    // normalize the content before doing anything
    normalize(text);

    if (file_insert_position > length())
    {
        // clamp to prevent out of bounds
        file_insert_position = length();
    }

    size_t start_pos = m_add_buffer.length();
    size_t text_length = text.length();

    m_add_buffer.append(text);

    m_treap.insert(file_insert_position,
                   {.buf_type = AL::buffer_type::ADD, .start = start_pos, .length = text_length, .newline_count = count_newlines(text)},
                   get_split_strategy());
}

void piece_table::remove(size_t position, size_t length)
{
    if (position > this->length())
        return;

    if (position + length > this->length())
    {
        length = this->length() - position;
    }

    m_treap.erase(position, length, get_split_strategy());
}

void piece_table::clear()
{
    m_original_buffer.clear();
    m_add_buffer.clear();
    m_treap.clear();
}

size_t piece_table::get_index_for_line(size_t target_line) const
{
    if (target_line <= 1)
        return 0;

    size_t found_index = (size_t)-1; // final result
    size_t current_line = 1;         // what LINE am i on?
    size_t global_index = 0;         // how many characters have i passed?

    m_treap.for_each([&current_line, &global_index, &found_index, &target_line, this](const AL::piece& p) {
        if (found_index != (size_t)-1)
            return true;

        // std::string_view::find can use AVX instructions that can significantly increase performance
        std::string_view search_window((p.buf_type == AL::buffer_type::ORIGINAL ? m_original_buffer : m_add_buffer).data() + p.start, p.length);

        size_t last_found = 0;
        for (size_t i = search_window.find('\n'); i != std::string::npos; i = search_window.find('\n', last_found))
        {
            last_found = i + 1;
            current_line++;

            if (current_line == target_line)
            {
                found_index = global_index + i + 1;
                return true;
            }
        }

        // wasnt in this piece. go to the next piece.
        global_index += p.length;
        return false;
    });

    return (found_index == (size_t)-1) ? length() : found_index;
}

std::string piece_table::to_string() const
{
    std::string result;
    result.reserve(m_treap.size());

    m_treap.for_each([&result, this](const AL::piece& p) {
        if (p.buf_type == AL::buffer_type::ORIGINAL)
        {
            result.append(m_original_buffer, p.start, p.length);
        }
        else
        {
            result.append(m_add_buffer, p.start, p.length);
        }

        return false;
    });

    return result;
}

size_t piece_table::length() const
{
    return m_treap.size();
}
} // namespace AL
