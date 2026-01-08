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
};

/*
 * Editor class for the text editor.
 *
 * Responsible for:
 * - File I/O operations (loading and saving files)
 * - File metadata management (name, path, modification state)
 * - Session state tracking (cursor position, undo/redo history, view state)
 * - Text manipulation and editing operations
 *
 * Uses batching to efficiently batch pieces in the implicit treap data structure
 * for optimized text storage and retrieval.
 */
class editor
{
public:

    editor();
    ~editor();

    bool open(const std::filesystem::path& path);
    bool save();
    bool save(const std::filesystem::path& path);

private:

    piece_table m_piece_table;
    std::filesystem::path m_current_file_path;
    bool m_dirty; // whether file was edited but not saved.
    cursor m_cursor;

    // handles closing the file for i/o gracefully
    // return true for quitting successfully
    // force quitting means data loss.
    bool quit(bool force_quit = false, bool save_automatically = false);
    void insert_char(char c);
    void delete_char();
    void move_cursor(direction dir);
};
} // namespace AL
