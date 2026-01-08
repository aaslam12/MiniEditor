#include "editor.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>

namespace AL
{

editor::editor()
{}

editor::~editor()
{}

bool editor::open(const std::filesystem::path& path)
{
    if (path.empty())
        return false;

    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    const bool size_known = !ec;

    auto status = std::filesystem::status(path, ec);
    if (!std::filesystem::is_regular_file(status))
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
        return false;

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

    m_piece_table.insert(0, str);
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
    if (std::filesystem::exists(path) && !std::filesystem::is_regular_file(status))
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

    return true;
}

void editor::insert_char(char c)
{}

void editor::delete_char()
{}

void editor::move_cursor(direction dir)
{}

} // namespace AL
