#pragma once

#include "editor.h"
#include <curses.h>
#include <fstream>

namespace AL
{

/*
 * Terminal User Interface layer.
 *
 * Manages viewport state, input handling, and rendering for the text editor.
 * Translates user input into editor commands and displays the result to the terminal.
 */
class tui
{
public:
    tui();
    ~tui();

    // returns true if the tui initialized properly
    bool init();
    void tick();
    bool should_quit() const;

private:
#if MINIEDITOR_DEBUG
    std::ofstream m_log;
#endif
    // window pointer received from curses library
    WINDOW* m_window;
    editor m_editor;
    bool m_quit;

    size_t m_viewport_top_line;
    size_t m_viewport_height;
    size_t m_viewport_width;

    void update_values();
    void render();
    void render_status_bar();
    void render_line(size_t line_idx);
    void handle_input(const int ch);
};

} // namespace AL
