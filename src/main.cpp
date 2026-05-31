#include "tui.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string file_path;
    if (argc > 1)
        file_path = argv[1];

    AL::tui ui;
    if (!ui.init(file_path))
    {
        std::cerr << "Could not initialize the TUI!\n";
        return 67;
    }

    while (!ui.should_quit())
    {
        ui.tick();
    }

    return 0;
}
