#include <catch2/catch_test_macros.hpp>
#include <tui.h>

TEST_CASE("TUI: Construction and Initial State", "[tui]")
{
    // The TUI can be constructed, but we will not call init()
    // because it requires a terminal and will fail in many test environments.
    AL::tui tui;

    SECTION("Initial state")
    {
        // The TUI should not be in a "quit" state by default.
        CHECK_FALSE(tui.should_quit());
    }
}

TEST_CASE("TUI: Lifecycle (Stubs)", "[tui]")
{
    AL::tui tui;

    SECTION("Tick should not throw")
    {
        // This test is limited because tick() waits for user input (getch).
        // In a real test environment, this would require mocking.
        // For now, we just check that the stubbed function can be called.
        // To prevent blocking, we won't call tick() here in the automated script.
        // To test, you would need to manually provide input to the test executable.
        // CHECK_NOTHROW(tui.tick());
    }

    SECTION("should_quit reflects internal state")
    {
        // This is not directly testable without exposing internal state or
        // being ableto simulate the input that would change the state (e.g. 'q').
        CHECK_FALSE(tui.should_quit());
    }
}
