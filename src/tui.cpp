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
    : m_window(nullptr), m_editor(editor()), m_quit(false), m_viewport_top_line(0), m_viewport_height(0), m_viewport_width(0),
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
    clear();

    if (m_viewport_height < 20 || m_viewport_width < 20)
        return; // dont render anything if terminal too small

    if (m_viewport_top_line > m_editor.get_cursor_row())
    {
        // cursor above viewport. have to move up
        m_viewport_top_line = m_editor.get_cursor_row();
    }
    else if ((m_viewport_top_line + m_viewport_height - 2) < m_editor.get_cursor_row())
    {
        // cursor below viewport. have to move down
        m_viewport_top_line = m_editor.get_cursor_row() - (m_viewport_height - 2);
    }

    size_t line_number_width = std::to_string(m_editor.get_total_lines()).length();

    for (size_t screen_row = 0; screen_row < m_viewport_height - 1; screen_row++)
    {
        render_line(screen_row, line_number_width + 1);
    }

    render_status_bar(m_viewport_height - 1);

    if (m_editor.get_cursor_row() >= m_viewport_top_line && m_editor.get_cursor_row() < m_viewport_top_line + m_viewport_height - 1)
    {
        size_t line_num_width = std::to_string(m_editor.get_total_lines()).length();
        int screen_row = m_editor.get_cursor_row() - m_viewport_top_line;
        int screen_col = line_num_width + 3 + m_editor.get_cursor_col();

        // clamp cursor to viewport width
        if (screen_col >= static_cast<int>(m_viewport_width))
            screen_col = static_cast<int>(m_viewport_width) - 1;

        move(screen_row, screen_col);
    }
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

    std::stringstream ss;

    ss << std::setw(col_offset) << std::right << line_num << " | ";

    // if this is the cursor line and there's an insert buffer, insert it at the correct position
    if (line_num == m_editor.get_cursor_row() && !m_editor.get_insert_buffer().empty())
    {
        size_t insert_start_col = m_editor.get_insert_buffer_start_col();
        // insert_start_col is 1-indexed, convert to 0-indexed for string position
        size_t insert_pos = (insert_start_col > 0) ? insert_start_col - 1 : 0;

        // insert buffer at the correct position in the line
        if (insert_pos <= content.length())
        {
            ss << content.substr(0, insert_pos);
            ss << m_editor.get_insert_buffer();
            ss << content.substr(insert_pos);
        }
        else
        {
            ss << content << m_editor.get_insert_buffer();
        }
    }
    else
    {
        ss << content;
    }

    mvaddstr(static_cast<int>(screen_row), 0, ss.str().c_str());
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
