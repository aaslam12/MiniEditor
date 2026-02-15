#include "tui.h"
#include "editor.h"
#include <cstddef>
#include <curses.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace AL
{

tui::tui()
    : m_window(nullptr), m_editor(editor()), m_quit(false), m_viewport_top_line(0), m_viewport_height(0), m_viewport_width(0), m_viewport_left_col(0),
      m_show_status_message(false)
{
#if MINIEDITOR_DEBUG
    m_log.open("/tmp/minieditor.log", std::ios::app);
#endif
}

tui::~tui()
{
    if (m_window)
        endwin();
}

bool tui::init(const std::string& file_path)
{
    m_window = initscr();
    if (!m_window)
        return false;

    raw(); // disable Ctrl+S/Ctrl+Q flow control
    noecho();
    keypad(m_window, TRUE);

    int max_x, max_y;               // uses int internally
    getmaxyx(stdscr, max_y, max_x); // actual terminal size

    m_viewport_height = max_y - 1;
    m_viewport_width = max_x;
    m_viewport_top_line = 1;

    if (!file_path.empty())
    {
        m_editor.open(file_path);
    }

    return true;
}

void tui::update_values()
{
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    if (static_cast<size_t>(max_y - 1) != m_viewport_height || static_cast<size_t>(max_x) != m_viewport_width)
    {
        m_viewport_height = static_cast<size_t>(max_y - 1);
        m_viewport_width = static_cast<size_t>(max_x);
        // might need to clamp viewport_top_line if file got shorter visually
    }
}

void tui::tick()
{
    update_values();
    render();
    refresh();

    int ch = getch();
    handle_input(ch);
}

void tui::render()
{
    curs_set(0);

    erase();

    if (m_viewport_height < 20 || m_viewport_width < 20)
    {
        curs_set(2);
        return; // dont render anything if terminal too small
    }

    // vertical scrolling
    if (m_viewport_top_line > m_editor.get_cursor_row())
    {
        m_viewport_top_line = m_editor.get_cursor_row();
    }
    else if ((m_viewport_top_line + m_viewport_height - 2) < m_editor.get_cursor_row())
    {
        m_viewport_top_line = m_editor.get_cursor_row() - (m_viewport_height - 2);
    }

    size_t line_number_width = std::to_string(m_editor.get_total_lines()).length();
    size_t gutter_width = line_number_width + 4; // "NN | "

    // horizontal scrolling: ensure cursor column is visible
    size_t content_area_width = m_viewport_width > gutter_width ? m_viewport_width - gutter_width : 1;
    size_t cursor_col_0 = m_editor.get_cursor_col() - 1; // convert to 0-indexed

    if (cursor_col_0 < m_viewport_left_col)
    {
        m_viewport_left_col = cursor_col_0;
    }
    else if (cursor_col_0 >= m_viewport_left_col + content_area_width)
    {
        m_viewport_left_col = cursor_col_0 - content_area_width + 1;
    }

    for (size_t screen_row = 0; screen_row < m_viewport_height - 1; screen_row++)
    {
        render_line(screen_row, line_number_width + 1);
    }

    render_status_bar(m_viewport_height - 1);

    // position the cursor
    if (m_editor.get_cursor_row() >= m_viewport_top_line && m_editor.get_cursor_row() < m_viewport_top_line + m_viewport_height - 1)
    {
        int screen_row = m_editor.get_cursor_row() - m_viewport_top_line;
        int screen_col = static_cast<int>(gutter_width + cursor_col_0 - m_viewport_left_col);

        if (screen_col >= static_cast<int>(m_viewport_width))
            screen_col = static_cast<int>(m_viewport_width) - 1;

        move(screen_row, screen_col);
    }

    curs_set(2);
}

void tui::render_status_bar(size_t status_bar_row)
{
    std::ostringstream oss;
    oss << m_editor.get_filename() << " [" << m_editor.get_cursor_row() << ":" << m_editor.get_cursor_col() << "]";
    if (m_editor.is_dirty())
        oss << " [modified]";

    if (m_show_status_message)
        oss << " " << m_status_message;

    mvaddstr(static_cast<int>(status_bar_row), 0, oss.str().c_str());
}

void tui::clear_status_message()
{
    m_show_status_message = false;
    m_status_message.clear();
}

void tui::set_status_message(const std::string& msg)
{
    m_status_message = msg;
    m_show_status_message = true;
}

void tui::render_line(size_t screen_row, size_t col_offset)
{
    auto line_num = screen_row + m_viewport_top_line;

    if (line_num > m_editor.get_total_lines())
    {
        mvaddstr(static_cast<int>(screen_row), 0, "~");
        return;
    }

    auto content = m_editor.get_line(line_num);

    // if this is the cursor line and there's an insert buffer, splice it in
    if (line_num == m_editor.get_cursor_row() && !m_editor.get_insert_buffer().empty())
    {
        size_t insert_start_col = m_editor.get_insert_buffer_start_col();
        size_t insert_pos = (insert_start_col > 0) ? insert_start_col - 1 : 0;

        if (insert_pos <= content.length())
        {
            content = content.substr(0, insert_pos) + m_editor.get_insert_buffer() + content.substr(insert_pos);
        }
        else
        {
            content += m_editor.get_insert_buffer();
        }
    }

    // build the gutter (line number + separator)
    std::stringstream ss;
    ss << std::setw(col_offset) << std::right << line_num << " | ";
    std::string gutter = ss.str();
    size_t gutter_width = gutter.length();

    // apply horizontal scroll and truncate content to fit viewport
    size_t content_area_width = m_viewport_width > gutter_width ? m_viewport_width - gutter_width : 0;

    std::string visible_content;
    if (m_viewport_left_col < content.length())
    {
        visible_content = content.substr(m_viewport_left_col, content_area_width);
    }

    // move to the screen row and write gutter + visible content
    mvaddstr(static_cast<int>(screen_row), 0, gutter.c_str());
    // write content separately so we can control exactly what appears
    mvaddnstr(static_cast<int>(screen_row), static_cast<int>(gutter_width), visible_content.c_str(), static_cast<int>(content_area_width));
}

void tui::handle_input(const int ch)
{
#if MINIEDITOR_DEBUG
    if (m_log.is_open())
    {
        m_log << "Key pressed: " << ch << " (0x" << std::hex << ch << std::dec << ")\n";
        m_log.flush();
    }
#endif

    switch (ch)
    {
        case '[':
            m_quit = true; // saves and exits

        case ']': // save file with ]
#if MINIEDITOR_DEBUG
            if (m_log.is_open())
                m_log << "] key detected, attempting save\n";
#endif
            if (m_editor.get_filename().empty())
            {
                set_status_message("No file open!");
            }
            else if (m_editor.save())
            {
                set_status_message("File saved!");
#if MINIEDITOR_DEBUG
                if (m_log.is_open())
                    m_log << "Save successful\n";
#endif
            }
            else
            {
                set_status_message("Save failed!");
#if MINIEDITOR_DEBUG
                if (m_log.is_open())
                    m_log << "Save failed\n";
#endif
            }
            break;

        case KEY_UP:
            clear_status_message();
            m_editor.move_cursor(direction::UP);
            break;

        case KEY_DOWN:
            clear_status_message();
            m_editor.move_cursor(direction::DOWN);
            break;

        case KEY_LEFT:
            clear_status_message();
            m_editor.move_cursor(direction::LEFT);
            break;

        case KEY_RIGHT:
            clear_status_message();
            m_editor.move_cursor(direction::RIGHT);
            break;

        case KEY_BACKSPACE:
        case 127:
        case 8:
            clear_status_message();
            m_editor.delete_char();
#if MINIEDITOR_DEBUG
            if (m_log.is_open())
                m_log << "Delete char called\n";
#endif
            break;

        case '\n':
        case '\r':
            clear_status_message();
            m_editor.insert_char('\n');
            break;

        default:
            // only accept printable ASCII characters (32-126)
            if (ch >= 32 && ch <= 126)
            {
                clear_status_message();
                m_editor.insert_char(static_cast<char>(ch));
            }
#if MINIEDITOR_DEBUG
            else if (m_log.is_open())
            {
                m_log << "Input read (but ignored): " << ch << "\n";
            }
#endif
            break;
    }
}

bool tui::should_quit() const
{
    return m_quit;
}

} // namespace AL
