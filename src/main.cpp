#include "tui.h"
#include <iostream>

int main()
{
    AL::tui ui;
    if (!ui.init())
    {
        std::cerr << "Could not initialize the TUI!\n";
        return 67;
    }

#if !MINIEDITOR_TESTING
    // while (!ui.should_quit())
    // {
    //     ui.tick();
    // }
#endif

    std::cout << "Hello World!\n";
    std::cout << "Hello World!\n";
    return 0;
}
