#include "piecetable.h"
#include "implicit_treap.h"
#include <cstddef>

piece_table::piece_table()
{}

piece_table::~piece_table()
{}

piece_table::piece_table(std::string initial_content) : m_original_buffer(std::move(initial_content))
{
    implicit_treap::piece piece;
    piece.length = m_original_buffer.length();
    piece.buf_type = implicit_treap::buffer_type::ORIGINAL;
    piece.start = 0;

    m_treap.insert(0, piece);
}

void piece_table::insert(size_t file_insert_position, const std::string& text)
{
    if (file_insert_position > length())
    {
        // clamp to prevent out of bounds
        file_insert_position = length();
    }

    size_t start_pos = m_add_buffer.length();
    size_t text_length = text.length();

    m_add_buffer.append(text);

    m_treap.insert(file_insert_position, {.buf_type = implicit_treap::buffer_type::ADD, .start = start_pos, .length = text_length});
}

void piece_table::remove(size_t position, size_t length)
{
    if (position > this->length())
        return;

    if (position + length > this->length())
    {
        length = this->length() - position;
    }

    m_treap.erase(position, length);
}

std::string piece_table::to_string() const
{
    std::string result;
    result.reserve(m_treap.size());

    m_treap.for_each([&result, this](const implicit_treap::piece& p) {
        if (p.buf_type == implicit_treap::buffer_type::ORIGINAL)
        {
            result.append(m_original_buffer, p.start, p.length);
        }
        else
        {
            result.append(m_add_buffer, p.start, p.length);
        }
    });

    return result;
}

size_t piece_table::length() const
{
    return m_treap.size();
}
