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

    while (!ui.should_quit())
    {
        ui.tick();
    }

    std::cout << "Hello World!\n";
    std::cout << "Hello World!\n";
    return 0;
}
