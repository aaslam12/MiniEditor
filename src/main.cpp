#include "tui.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string file_path;
    
    // Parse command-line arguments
    if (argc > 1)
    {
        file_path = argv[1];
    }
    
    AL::tui ui;
    if (!ui.init(file_path))
    {
        std::cerr << "Could not initialize the TUI!\n";
        return 67;
    }

#if !MINIEDITOR_TESTING
    while (!ui.should_quit())
    {
        ui.tick();
    }
#endif

    return 0;
}
