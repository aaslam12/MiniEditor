#include "editor.h"
#include "piecetable.h"
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>

namespace AL
{

constexpr char NEWLINE = '\n';

editor::editor() : m_dirty(false), m_insert_position(0)
{
    m_cursor.reset();
    m_insert_buffer.reserve(m_max_insert_buffer_length);
}

editor::~editor()
{}

bool editor::open(const std::filesystem::path& path)
{
    // TODO: add a feature where it searches if there is a .tmp counterpart to this file we want to open.
    // then we can prompt the user if they want to open this tmp file or to delete the temp file and open the real file
    if (path.empty())
        return false;

    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    const bool size_known = !ec;

    auto status = std::filesystem::status(path, ec);
    if (ec)
    {
        // device, fifo, socket, symlink, etc.
        std::cerr << "ERROR: Not a valid file path: " << path << NEWLINE;
        return false;
    }

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        std::cerr << "ERROR: Could not open file " << path << NEWLINE;
        return false;
    }

    std::string str;
    if (size_known)
    {
        str.resize(static_cast<std::size_t>(size));
        ifs.read(str.data(), str.size());

        const auto bytes = ifs.gcount(); // for race conditions and if a file changed after we read
        if (bytes < static_cast<std::streamsize>(str.size()))
            str.resize(static_cast<std::size_t>(bytes));
    }
    else
    {
        char chunk[64 * 1024];
        while (ifs.read(chunk, sizeof(chunk)))
            str.append(chunk, sizeof(chunk));
        str.append(chunk, ifs.gcount());
    }

    m_current_file_path = path;
    m_dirty = false;

    m_piece_table = piece_table(str);
    m_cursor.col = 0;
    m_cursor.col_internal = 0;
    m_cursor.row = 1;
    m_cursor.global_index = 0;
    return true;
}

bool editor::save()
{
    return save(m_current_file_path);
}

bool editor::save(const std::filesystem::path& path)
{
    if (path.empty())
        return false;

    std::error_code ec;
    if (std::filesystem::exists(path))
    {
        auto status = std::filesystem::status(path, ec);
        if (!std::filesystem::is_regular_file(status) || ec)
        {
            // device, fifo, socket, symlink, etc.
            std::cerr << "ERROR: Not a valid file path. Path: " << path << NEWLINE;
            return false;
        }
    }

    std::filesystem::path temp_file_path = path;
    temp_file_path += ".tmp";

    std::ofstream ofs(temp_file_path, std::ios::binary);
    if (!ofs)
    {
        std::cerr << "ERROR: Could not open file to save. Path: " << path << NEWLINE;
        return false;
    }

    // temp file opened for saving
    std::string str = m_piece_table.to_string();
    ofs.write(str.c_str(), str.length());
    ofs.close();

    // then we try to write to the actual file
    std::filesystem::rename(temp_file_path, path, ec);
    if (ec)
    {
        std::cerr << "ERROR: Saving to file failed. Path: " << path << NEWLINE;
        return false;
    }

    m_current_file_path = path;
    m_dirty = false;
    return true;
}

bool editor::quit(bool force_quit, bool save_automatically)
{
    if (force_quit)
        goto quit;

    if (save_automatically)
        save();

    if (m_dirty)
    {
        std::cerr << "ERROR: Save current file before opening another\n";
        return false;
    }

quit:
    m_piece_table.clear();
    m_cursor.reset();
    m_current_file_path.clear();

    return true;
}

std::string editor::get_line(size_t line_number) const
{
    if (line_number == 0 || line_number > m_piece_table.get_line_count())
        return "";

    return m_piece_table.get_line(line_number);
}

void editor::insert_char(char c)
{
    m_dirty = true;

    if (m_insert_buffer.empty())
    {
        m_insert_position = m_cursor.global_index;
    }

    m_insert_buffer.push_back(c);

    if (c == NEWLINE)
    {
        flush_insert_buffer();
        m_cursor.row++;
        m_cursor.col = 0;
        m_cursor.global_index++;
        return;
    }

    if (m_insert_buffer.length() == m_max_insert_buffer_length)
    {
        flush_insert_buffer();
    }

    m_cursor.col++;
    m_cursor.global_index++;
}

void editor::delete_char()
{
    m_dirty = true;
    if (m_cursor.global_index == 0)
        return;

    if (char deleted_char = m_piece_table.get_char_at(m_cursor.global_index - 1); deleted_char == NEWLINE)
    {
        m_piece_table.remove(m_cursor.global_index - 1, 1);
        m_cursor.global_index--;
        m_cursor.row--;
        m_cursor.col = m_piece_table.get_line_length(m_cursor.row);
    }
    else
    {
        m_piece_table.remove(m_cursor.global_index - 1, 1);
        m_cursor.global_index--;
        m_cursor.col--;
    }
}

void editor::move_cursor(direction dir)
{
    flush_insert_buffer();
    switch (dir)
    {
        case direction::UP:
            handle_cursor_up();
            break;

        case direction::DOWN:
            handle_cursor_down();
            break;

        case direction::LEFT:
            handle_cursor_left();
            break;

        case direction::RIGHT:
            handle_cursor_right();
            break;
    }
}

void editor::handle_cursor_up()
{
    if (m_cursor.row == 1)
    {
        // the start of the file
        m_cursor.col = 0;
        m_cursor.col_internal = 0;
        m_cursor.global_index = 0;
        return;
    }

    m_cursor.row--;

    if (size_t line_len = m_piece_table.get_line_length(m_cursor.row); line_len <= m_cursor.col)
    {
        m_cursor.col = line_len;
    }
    else
    {
        m_cursor.col = m_cursor.col_internal;
    }

    m_cursor.global_index = m_piece_table.get_index_for_line(m_cursor.row) + m_cursor.col;
}

void editor::handle_cursor_down()
{
    size_t line_cnt = m_piece_table.get_line_count();
    if (line_cnt == 0)
    {
        m_cursor.row = 1;
        m_cursor.col = 0;
        m_cursor.global_index = 0;
        return;
    }

    if (line_cnt == m_cursor.row)
    {
        m_cursor.col = m_piece_table.get_line_length(m_cursor.row);
        m_cursor.global_index = m_piece_table.length();
        return;
    }

    m_cursor.row++;

    if (size_t line_len = m_piece_table.get_line_length(m_cursor.row); line_len <= m_cursor.col)
    {
        m_cursor.col = line_len;
    }
    else
    {
        m_cursor.col = m_cursor.col_internal;
    }

    m_cursor.global_index = m_piece_table.get_index_for_line(m_cursor.row) + m_cursor.col;
}

void editor::handle_cursor_left()
{
    if (m_cursor.global_index == 0)
        return;

    if (m_cursor.col == 0)
    {
        // move to last line
        m_cursor.row--;
        m_cursor.col = m_piece_table.get_line_length(m_cursor.row);
        m_cursor.col_internal = m_cursor.col;
    }
    else
    {
        m_cursor.col--;
        m_cursor.col_internal = m_cursor.col;
    }
    m_cursor.global_index--;
}

void editor::handle_cursor_right()
{
    if (m_cursor.global_index == m_piece_table.length())
        return;

    if (m_cursor.col == m_piece_table.get_line_length(m_cursor.row))
    {
        // move to new line
        m_cursor.row++;
        m_cursor.col = 0;
        m_cursor.col_internal = 0;
        m_cursor.global_index++;
    }
    else
    {
        m_cursor.col++;
        m_cursor.col_internal = m_cursor.col;
        m_cursor.global_index++;
    }
}

void editor::flush_insert_buffer()
{
    if (m_insert_buffer.empty())
        return;

    m_piece_table.insert(m_insert_position, m_insert_buffer);

    m_insert_position += m_insert_buffer.length();
    m_insert_buffer.clear();
}

size_t editor::get_total_lines() const
{
    size_t lines = m_piece_table.get_line_count();
    return lines > 0 ? lines : 1;
}

size_t editor::get_cursor_row() const
{
    return m_cursor.row;
}

size_t editor::get_cursor_col() const
{
    return m_cursor.col;
}

bool editor::is_dirty() const
{
    return m_dirty;
}

std::string editor::get_filename() const
{
    return m_current_file_path.filename().string();
}

} // namespace AL
