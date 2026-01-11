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

piece_table::piece_table(piece_table&& other) noexcept
{
    m_add_buffer = std::move(other.m_add_buffer);
    m_original_buffer = std::move(other.m_original_buffer);
    m_treap = std::move(other.m_treap);
}

piece_table& piece_table::operator=(piece_table&& other) noexcept
{
    if (&other == this)
        return *this;
    clear();

    m_add_buffer = std::move(other.m_add_buffer);
    m_original_buffer = std::move(other.m_original_buffer);
    m_treap = std::move(other.m_treap);

    return *this;
}

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
    if (m_treap.empty() || target_line == 0)
        return 0;

    node* n = nullptr;
    size_t global_index_of_piece_start = 0;
    
    m_treap.find_by_line(target_line, n, global_index_of_piece_start);

    if (!n)
    {
        return length(); 
    }

    size_t lines_before_current_piece_start = implicit_treap::get_subtree_newlines(n->left);
    size_t relative_target_line_in_piece = target_line - (lines_before_current_piece_start + 1);

    if (relative_target_line_in_piece == 0)
    {
        return global_index_of_piece_start;
    }

    const std::string& buffer = (n->data.buf_type == buffer_type::ORIGINAL ? m_original_buffer : m_add_buffer);
    std::string_view piece_view(buffer.data() + n->data.start, n->data.length);

    size_t current_newline_count_in_piece = 0;
    size_t last_newline_pos_in_piece = 0;

    for (size_t pos = 0; pos < piece_view.length(); ++pos)
    {
        if (piece_view[pos] == '\n')
        {
            current_newline_count_in_piece++;
            last_newline_pos_in_piece = pos;

            if (current_newline_count_in_piece == relative_target_line_in_piece)
            {
                return global_index_of_piece_start + pos + 1;
            }
        }
    }

    if (current_newline_count_in_piece < relative_target_line_in_piece)
    {
        return global_index_of_piece_start + last_newline_pos_in_piece + 1;
    }
    
    return global_index_of_piece_start;
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

std::string piece_table::get_line(size_t line_number) const
{
    std::string result;
    if (line_number == 0 || line_number > get_line_count())
        return result;

    size_t line_start_index = get_index_for_line(line_number);
    if (line_start_index >= length())
        return result;

    size_t current_index = 0;
    bool started = false;

    m_treap.for_each([&](const AL::piece& p) {
        size_t piece_global_start_index = current_index;
        size_t piece_global_end_index = current_index + p.length;

        if (!started && line_start_index >= piece_global_start_index && line_start_index < piece_global_end_index)
        {
            started = true;
            size_t offset_in_piece = line_start_index - piece_global_start_index;
            const std::string& buffer = (p.buf_type == buffer_type::ORIGINAL ? m_original_buffer : m_add_buffer);
            std::string_view piece_view(buffer.data() + p.start + offset_in_piece, p.length - offset_in_piece);

            for (char c : piece_view)
            {
                if (c == '\n')
                {
                    return true; // Stop iteration
                }
                result += c;
            }
        }
        else if (started)
        {
            const std::string& buffer = (p.buf_type == buffer_type::ORIGINAL ? m_original_buffer : m_add_buffer);
            std::string_view piece_view(buffer.data() + p.start, p.length);

            for (char c : piece_view)
            {
                if (c == '\n')
                {
                    return true; // Stop iteration
                }
                result += c;
            }
        }
        current_index += p.length;
        return false;
    });

    return result;
}

size_t piece_table::length() const
{
    return m_treap.size();
}

size_t piece_table::get_line_count() const
{
    if (m_treap.empty())
        return 0;

    size_t newline_count = m_treap.get_newline_count();
    if (newline_count == 0 && length() > 0)
    {
        return 1;
    }
    return newline_count + 1;
}

char piece_table::get_char_at(size_t byte_index) const
{
    if (byte_index >= length())
        return '\0';

    node* n;
    size_t byte_offset;
    m_treap.find_by_byte(byte_index, n, byte_offset);
    if (!n)
        return '\0';

    return (n->data.buf_type == buffer_type::ORIGINAL ? m_original_buffer : m_add_buffer).c_str()[n->data.start + byte_index - byte_offset];
}

size_t piece_table::get_line_length(size_t line_number) const
{
    if (line_number == 0 || line_number > get_line_count())
        return 0;

    size_t line_start_index = get_index_for_line(line_number);
    size_t next_line_start_index = get_index_for_line(line_number + 1);

    if (next_line_start_index == line_start_index)
    {
        return length() - line_start_index;
    }
    return next_line_start_index - line_start_index - 1; // -1 to account for the newline character
}
} // namespace AL
