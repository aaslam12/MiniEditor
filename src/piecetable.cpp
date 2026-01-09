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
    piece.newline_count = count_newlines(m_original_buffer);

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
    if (target_line <= 1 || m_treap.empty())
        return 0;

    node* n;
    size_t byte_offset;
    m_treap.find_by_line(target_line, n, byte_offset);

    if (!n)
        return length();
    piece p = n->data;

    if (p.newline_count == 0)
    {
        return byte_offset;
    }

    size_t lines_so_far = implicit_treap::get_subtree_newlines(n->left) + 1;
    const std::string& buffer = p.buf_type == buffer_type::ORIGINAL ? m_original_buffer : m_add_buffer;

    // string_view::find for AVX instructions speed up
    std::string_view search_window(buffer.data() + p.start, p.length);
    size_t last_found = 0;

    // pos is the index of the new line found
    for (size_t pos = search_window.find('\n', 0); pos != std::string::npos; pos = search_window.find('\n', pos))
    {
        // found a new line
        lines_so_far++;
        last_found = pos;

        if (target_line == lines_so_far)
            return byte_offset + last_found + 1;

        pos++;
    }

    return length();
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
