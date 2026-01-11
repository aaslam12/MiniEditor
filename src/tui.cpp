#include "tui.h"
#include "editor.h"
#include <curses.h>
#include <iostream>

namespace AL
{

tui::tui() : m_window(nullptr), m_editor(editor()), m_quit(false), m_viewport_top_line(0), m_viewport_height(0), m_viewport_width(0)
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

bool tui::init()
{
    m_window = initscr();
    if (!m_window)
        return false;

    cbreak();
    noecho();
    keypad(m_window, TRUE);

    int max_x, max_y;               // uses int internally
    getmaxyx(stdscr, max_y, max_x); // actual terminal size

    m_viewport_height = max_y - 1;
    m_viewport_width = max_x;
    m_viewport_top_line = 0;

    return true;
}

void tui::update_values()
{
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    // Update if changed
    if (static_cast<size_t>(max_y - 1) != m_viewport_height || static_cast<size_t>(max_x) != m_viewport_width)
    {
        m_viewport_height = static_cast<size_t>(max_y - 1);
        m_viewport_width = static_cast<size_t>(max_x);
        // Might need to clamp viewport_top_line if file got shorter visually
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
    mvaddstr(0, 0, "Hello World");
    render_status_bar();
}

void tui::render_status_bar()
{
    // TODO: Implement status bar rendering
}

void tui::render_line(size_t /*line_idx*/)
{
    // TODO: Implement line rendering
}

void tui::handle_input(const int ch)
{
    if (ch == 'q')
    {
        m_quit = true;
        return;
    }

    switch (ch)
    {
        case KEY_UP:
            m_editor.move_cursor(direction::UP);
            break;

        case KEY_DOWN:
            m_editor.move_cursor(direction::DOWN);
            break;

        case KEY_LEFT:
            m_editor.move_cursor(direction::LEFT);
            break;

        case KEY_RIGHT:
            m_editor.move_cursor(direction::RIGHT);
            break;

        default:
#if MINIEDITOR_DEBUG
            if (m_log.is_open())
                m_log << "Input read (but ignored): " << ch << "\n";
#endif
            break;
    }
}

bool tui::should_quit() const
{
    return m_quit;
}

} // namespace AL
