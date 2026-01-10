#include "editor.h"
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>

namespace AL
{

editor::editor() : m_dirty(false)
{}

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
    if (!std::filesystem::is_regular_file(status) || ec)
    {
        // device, fifo, socket, symlink, etc.
        std::cerr << "ERROR: Not a valid file path.\n";
        return false;
    }

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        std::cerr << "ERROR: Could not open file " << path << '\n';
        return false;
    }

    if (!quit())
    {
        ifs.close();
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
    auto status = std::filesystem::status(path, ec);
    if ((std::filesystem::exists(path) && !std::filesystem::is_regular_file(status)) || ec)
    {
        // device, fifo, socket, symlink, etc.
        std::cerr << "ERROR: Not a valid file path. Path: " << path << '\n';
        return false;
    }

    std::filesystem::path temp_file_path = path;
    temp_file_path += ".tmp";

    std::ofstream ofs(temp_file_path, std::ios::binary);
    if (!ofs)
    {
        std::cerr << "ERROR: Could not open file to save. Path: " << path << '\n';
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
        std::cerr << "ERROR: Saving to file failed. Path: " << path << '\n';
        return false;
    }

    m_dirty = false;
    return true;
}

bool editor::quit(bool force_quit, bool save_automatically)
{
    if (force_quit)
        goto quit;

    if (save_automatically)
        goto save;

    if (m_dirty)
    {
        std::cerr << "ERROR: Save current file before opening another\n";
        return false;
    }

save:
    save();

quit:
    m_piece_table.clear();
    m_cursor.reset();
    m_current_file_path.clear();

    return true;
}

std::string editor::get_line(size_t line_number) const
{
    if (line_number <= 0 || m_piece_table.length() <= 0)
        return "";

    return m_piece_table.get_line(line_number);
}

void editor::insert_char(char /*c*/)
{
    // needs batched insertion
}

void editor::delete_char()
{}

void editor::move_cursor(direction /*dir*/)
{}

size_t editor::get_total_lines() const
{
    size_t lines = m_piece_table.get_line_count();
    return lines > 0 ? lines + 1 : 0;
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
