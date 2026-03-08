#include "piecetable.h"
#include "implicit_treap.h"
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

namespace AL
{
size_t piece_table::normalize(std::string& text)
{
    if (text.find('\n') != std::string::npos)
    {}
    else if (text.find('\r') == std::string::npos)
        return 0;

    size_t newline_count = 0;
    auto it = text.begin();

    for (auto c : text)
    {
        if (c == '\n')
            newline_count++;

        if (c != '\r')
            *it++ = c;
    }

    text.erase(it, text.end());
    return newline_count;
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

piece_table::piece_table() : m_needs_rebuild(true)
{}

piece_table::~piece_table()
{}

piece_table::piece_table(piece_table&& other) noexcept
{
    m_add_buffer = std::move(other.m_add_buffer);
    m_original_buffer = std::move(other.m_original_buffer);
    m_treap = std::move(other.m_treap);
    m_cached_string = std::move(other.m_cached_string);
    m_needs_rebuild = other.m_needs_rebuild;
}

piece_table& piece_table::operator=(piece_table&& other) noexcept
{
    if (&other == this)
        return *this;
    clear();

    m_add_buffer = std::move(other.m_add_buffer);
    m_original_buffer = std::move(other.m_original_buffer);
    m_treap = std::move(other.m_treap);
    m_cached_string = std::move(other.m_cached_string);
    m_needs_rebuild = other.m_needs_rebuild;

    return *this;
}

piece_table::piece_table(std::string initial_content) : m_needs_rebuild(true)
{
    // normalize the content before doing anything
    size_t newline_count = normalize(initial_content);
    m_original_buffer = std::move(initial_content);

    // dont insert a zero length piece  it breaks treap operations
    if (m_original_buffer.empty())
        return;

    AL::piece piece;
    piece.length = m_original_buffer.length();
    piece.buf_type = AL::buffer_type::ORIGINAL;
    piece.start = 0;
    piece.newline_count = newline_count;

    m_treap.insert(0, piece, get_split_strategy());
}

void piece_table::insert(size_t file_insert_position, std::string text)
{
    size_t newline_count = 0;

    // Only do full normalize (strip \r) if text could be pasted content
    if (text.find('\r') != std::string::npos)
        newline_count = normalize(text);
    else
    {
        for (char c : text)
            if (c == '\n')
                newline_count++;
    }

    if (file_insert_position > length())
    {
        // clamp to prevent out of bounds
        file_insert_position = length();
    }

    size_t start_pos = m_add_buffer.length();
    size_t text_length = text.length();

    m_add_buffer.append(text);

    m_treap.insert(file_insert_position,
                   {.buf_type = AL::buffer_type::ADD, .start = start_pos, .length = text_length, .newline_count = newline_count},
                   get_split_strategy());

    m_needs_rebuild = true;
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
    m_needs_rebuild = true;
}

void piece_table::clear()
{
    m_original_buffer.clear();
    m_add_buffer.clear();
    m_treap.clear();
    m_needs_rebuild = true;
}

size_t piece_table::get_index_for_line(size_t target_line) const
{
    if (target_line == 0 || m_treap.empty())
        return 0;

    const size_t total_lines = get_line_count();
    if (target_line > total_lines)
        return length();
    if (target_line == 1)
        return 0;

    node* n = nullptr;
    size_t byte_offset = 0;
    size_t line_in_piece = 0;
    m_treap.find_line_position(target_line, n, byte_offset, line_in_piece);

    if (!n)
        return length();

    // line_in_piece tells us this is the Nth line that starts in this piece
    // Line 1 in piece starts after 1st newline, line 2 after 2nd, etc.
    size_t newlines_to_skip = line_in_piece;

    const std::string& buffer = (n->data.buf_type == buffer_type::ORIGINAL ? m_original_buffer : m_add_buffer);
    std::string_view piece_view(buffer.data() + n->data.start, n->data.length);

    size_t newlines_found = 0;
    for (size_t i = 0; i < piece_view.length(); ++i)
    {
        if (piece_view[i] == '\n')
        {
            newlines_found++;
            if (newlines_found == newlines_to_skip)
            {
                return byte_offset + i + 1;
            }
        }
    }

    // should not reach here if tree is consistent
    return byte_offset + n->data.length;
}

void piece_table::write_to(std::ostream& os) const
{
    m_treap.for_each([this, &os](const AL::piece& piece) {
        if (piece.buf_type == AL::buffer_type::ORIGINAL)
        {
            const std::string& buffer = m_original_buffer;
            os.write(buffer.data() + piece.start, piece.length);
        }
        else
        {
            const std::string& buffer = m_add_buffer;
            os.write(buffer.data() + piece.start, piece.length);
        }

        return false;
    });
}

std::string piece_table::to_string() const
{
    if (!m_needs_rebuild)
        return m_cached_string;

    m_cached_string.clear();
    m_cached_string.reserve(m_treap.size());

    m_treap.for_each([this](const AL::piece& p) {
        if (p.buf_type == AL::buffer_type::ORIGINAL)
        {
            m_cached_string.append(m_original_buffer, p.start, p.length);
        }
        else
        {
            m_cached_string.append(m_add_buffer, p.start, p.length);
        }

        return false;
    });

    m_needs_rebuild = false;
    return m_cached_string;
}

std::string piece_table::get_line(size_t line_number) const
{
    std::string result;
    if (line_number == 0 || line_number > get_line_count())
        return result;

    size_t line_start_index = get_index_for_line(line_number);
    if (line_start_index >= length())
        return result;

    // Find the piece containing line_start_index in O(log n)
    node* start_node = nullptr;
    size_t piece_byte_offset = 0;
    m_treap.find_by_byte(line_start_index, start_node, piece_byte_offset);
    if (!start_node)
        return result;

    const size_t skip_in_first = line_start_index - piece_byte_offset;
    bool is_first = true;

    // Emit from piece_byte_offset onwards, skipping the leading bytes in the first piece
    m_treap.for_each_from_byte(piece_byte_offset, [&](const AL::piece& p) {
        const std::string& buffer = (p.buf_type == buffer_type::ORIGINAL ? m_original_buffer : m_add_buffer);

        size_t offset = 0;
        if (is_first)
        {
            offset = skip_in_first;
            is_first = false;
        }

        std::string_view pv(buffer.data() + p.start + offset, p.length - offset);
        const size_t nl = pv.find('\n');
        if (nl != std::string_view::npos)
        {
            result.append(pv.data(), nl);
            return true;
        }
        result.append(pv);
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

    const size_t newline_count = m_treap.get_newline_count();
    if (length() == 0)
        return 0;

    if (newline_count == 0)
        return 1;

    const bool ends_with_newline = get_char_at(length() - 1) == '\n';
    return ends_with_newline ? newline_count : newline_count + 1;
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

    const size_t line_start_index = get_index_for_line(line_number);
    const size_t next_line_start_index = (line_number < get_line_count()) ? get_index_for_line(line_number + 1) : length();

    size_t end_index = next_line_start_index;
    if (end_index > line_start_index && end_index <= length() && get_char_at(end_index - 1) == '\n')
    {
        end_index -= 1;
    }

    return end_index > line_start_index ? end_index - line_start_index : 0;
}
} // namespace AL
