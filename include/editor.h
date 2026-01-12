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

    size_t row;          // 1-indexed
    size_t col;          // 1-indexed
    size_t col_internal; // for when you move your cursor up or down and the line doesnt have enough characters so you have to clamp it. we save this
                         // as the old cursor column

    void reset()
    {
        global_index = 0;
        row = 1;
        col = 1;
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
 *
 * Uses batching for insertions but not for deletions.
 * Batched insertions will be 512 bytes max at a time.
 * If the user types more, then the buffer gets flushed.
 *
 * Any paste action (not implemented yet) will not use the
 * insertion buffer but will just be directly inserted.
 *
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
    void delete_char(); // deletes BEFORE the cursor (backspace)
    void move_cursor(direction dir);

    size_t get_total_lines() const;
    size_t get_cursor_row() const; // 1-indexed
    size_t get_cursor_col() const; // 1-indexed
    bool is_dirty() const;
    std::string get_filename() const;
    std::string get_line(size_t line_number) const; // refers to the 1-indexed line number
    const std::string& get_insert_buffer() const;
    size_t get_insert_buffer_start_col() const; // returns the column where insert buffer starts (1-indexed), or 0 if buffer is empty

private:
    piece_table m_piece_table;
    std::filesystem::path m_current_file_path;
    bool m_dirty; // whether file was edited but not saved.
    cursor m_cursor;

    // batching
    // reserve 512 bytes for the insert buffer.
    // if a user types ~512 characters, then it will flush instead of resizing
    // it will not be allocating more memory
    constexpr static size_t m_max_insert_buffer_length = 512;
    std::string m_insert_buffer; // the temporary insert buffer
    size_t m_insert_position;    // the global index where the text in the insert buffer is inserted into the piece table

    // handles closing the file for i/o gracefully
    // return true for quitting successfully
    // force quitting means data loss.
    bool quit(bool force_quit = false, bool save_automatically = false);
#if MINIEDITOR_TESTING
public:
#endif
    void flush_insert_buffer();

private:
    // cursor movement helpers
    void handle_cursor_up();
    void handle_cursor_down();
    void handle_cursor_left();
    void handle_cursor_right();
};
} // namespace AL
