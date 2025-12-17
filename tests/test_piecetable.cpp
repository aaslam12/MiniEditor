#include "piecetable.h"
#include <catch2/catch_test_macros.hpp>

// Tests the initial state of the piece table.
TEST_CASE("piece_table Initialization", "[piecetable]")
{
    // Test case 1: Default constructor creates an empty document.
    piece_table pt_empty;
    REQUIRE(pt_empty.length() == 0);
    REQUIRE(pt_empty.to_string() == "");

    // Test case 2: Constructor with initial content.
    piece_table pt_initial("Hello, world!");
    REQUIRE(pt_initial.length() == 13);
    REQUIRE(pt_initial.to_string() == "Hello, world!");
}

// Tests the insert functionality.
TEST_CASE("piece_table Insertion", "[piecetable]")
{
    piece_table pt("Hello world!");

    // Test case 1: Insert at the beginning.
    pt.insert(0, "Awesome ");
    REQUIRE(pt.length() == 20);
    REQUIRE(pt.to_string() == "Awesome Hello world!");

    // Test case 2: Insert in the middle.
    pt.insert(8, "new ");
    REQUIRE(pt.length() == 24);
    REQUIRE(pt.to_string() == "Awesome new Hello world!");

    // Test case 3: Insert at the end.
    pt.insert(24, " It's a beautiful day.");
    REQUIRE(pt.length() == 46);
    REQUIRE(pt.to_string() == "Awesome new Hello world! It's a beautiful day.");

    // Test case 4: Insert empty string.
    pt.insert(8, "");
    REQUIRE(pt.length() == 46);
    REQUIRE(pt.to_string() == "Awesome new Hello world! It's a beautiful day.");
}

// Tests the delete functionality.
TEST_CASE("piece_table Deletion", "[piecetable]")
{
    piece_table pt("Hello, beautiful world!");

    // Test case 1: Delete from the beginning.
    pt.remove(0, 7); // "Hello, "
    REQUIRE(pt.length() == 16);
    REQUIRE(pt.to_string() == "beautiful world!");

    // Test case 2: Delete from the middle.
    pt.remove(0, 10); // "beautiful "
    REQUIRE(pt.length() == 6);
    REQUIRE(pt.to_string() == "world!");

    // Test case 3: Delete from the end.
    pt.remove(5, 1); // "!"
    REQUIRE(pt.length() == 5);
    REQUIRE(pt.to_string() == "world");

    // Test case 4: Delete the entire content.
    pt.remove(0, 5);
    REQUIRE(pt.length() == 0);
    REQUIRE(pt.to_string() == "");

    // Test case 5: Delete zero characters.
    piece_table pt2("No change");
    pt2.remove(3, 0);
    REQUIRE(pt2.length() == 9);
    REQUIRE(pt2.to_string() == "No change");
}

// Tests a mix of insert and delete operations.
TEST_CASE("piece_table Mixed Operations", "[piecetable]")
{
    piece_table pt("This is a test.");

    // Replace "a test" with "an example"
    pt.remove(8, 6); // "a test"
    pt.insert(8, "an example");
    REQUIRE(pt.to_string() == "This is an example.");
    REQUIRE(pt.length() == 20);

    // Prepend text
    pt.insert(0, "Note: ");
    REQUIRE(pt.to_string() == "Note: This is an example.");
    REQUIRE(pt.length() == 26);

    // Delete from the middle
    pt.remove(4, 6); // ": This"
    REQUIRE(pt.to_string() == "Note is an example.");
    REQUIRE(pt.length() == 20);
}

// Tests edge cases for the piece table.
TEST_CASE("piece_table Edge Cases", "[piecetable]")
{
    // Test case 1: A long series of small inserts.
    piece_table pt;
    for (int i = 0; i < 100; ++i)
    {
        pt.insert(pt.length(), "a");
    }
    std::string expected(100, 'a');
    REQUIRE(pt.length() == 100);
    REQUIRE(pt.to_string() == expected);

    // Test case 2: A long series of small deletes from the start.
    for (int i = 0; i < 100; ++i)
    {
        pt.remove(0, 1);
    }
    REQUIRE(pt.length() == 0);
    REQUIRE(pt.to_string() == "");

    // Test case 3: Operations on an empty table.
    piece_table pt_empty;
    pt_empty.insert(0, "hello");
    REQUIRE(pt_empty.to_string() == "hello");
    pt_empty.remove(0, 5);
    REQUIRE(pt_empty.to_string() == "");

    // Test case 4: Deleting more characters than available.
    // The behavior here might depend on implementation (throw or clamp).
    // We'll assume it clamps for now. A robust implementation should define this.
    piece_table pt_clamp("short");
    pt_clamp.remove(0, 100); // Try to delete more than exists
    REQUIRE(pt_clamp.length() == 0);
    REQUIRE(pt_clamp.to_string() == "");
}
