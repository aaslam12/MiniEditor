#pragma once

#include "piecetable.h"
#include <cstddef>
#include <cstdint>
#include <filesystem>
namespace AL
{

enum class direction : uint8_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

struct cursor
{
    size_t global_index;

    size_t row;
    size_t col;

    void reset()
    {
        global_index = -1;
        row = -1;
        col = -1;
    }

    cursor()
    {
        reset();
    }
};

/*
 * Editor class for the text editor.
 *
 * Responsible for:
 * - File I/O operations (loading and saving files)
 * - File metadata management (name, path, modification state)
 * - Session state tracking (cursor position, view state)
 * - Text manipulation and editing operations
 */
class editor
{
public:
    editor();
    ~editor();

    bool open(const std::filesystem::path& path);
    bool save();
    bool save(const std::filesystem::path& path);

    void insert_char(char c);
    void delete_char();
    void move_cursor(direction dir);

    size_t get_total_lines() const;
    size_t get_cursor_row() const;
    size_t get_cursor_col() const;
    bool is_dirty() const;
    std::string get_filename() const;
    std::string get_line(size_t line_number) const;

private:
    piece_table m_piece_table;
    std::filesystem::path m_current_file_path;
    bool m_dirty; // whether file was edited but not saved.
    cursor m_cursor;

    // handles closing the file for i/o gracefully
    // return true for quitting successfully
    // force quitting means data loss.
    bool quit(bool force_quit = false, bool save_automatically = false);
};
} // namespace AL
