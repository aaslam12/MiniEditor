#include "piecetable.h"
#include <catch2/catch_test_macros.hpp>
#include <editor.h>
#include <implicit_treap.h>
#include <piecetable.h>

using piece_table = AL::piece_table;
using implicit_treap = AL::implicit_treap;

// Tests the initial state of the piece table.
TEST_CASE("piece_table Initialization", "[piecetable]")
{
    // Test case 1: Default constructor creates an empty document.
    piece_table pt_empty;
    CHECK(pt_empty.length() == 0);
    CHECK(pt_empty.to_string() == "");

    // Test case 2: Constructor with initial content.
    piece_table pt_initial("Hello, world!");
    CHECK(pt_initial.length() == 13);
    CHECK(pt_initial.to_string() == "Hello, world!");
}

// Tests the insert functionality.
TEST_CASE("piece_table Insertion", "[piecetable]")
{
    piece_table pt("Hello world!");

    // Test case 1: Insert at the beginning.
    pt.insert(0, "Awesome ");
    CHECK(pt.length() == 20);
    CHECK(pt.to_string() == "Awesome Hello world!");

    // Test case 2: Insert in the middle.
    pt.insert(8, "new ");
    CHECK(pt.length() == 24);
    CHECK(pt.to_string() == "Awesome new Hello world!");

    // Test case 3: Insert at the end.
    pt.insert(24, " It's a beautiful day.");
    CHECK(pt.length() == 46);
    CHECK(pt.to_string() == "Awesome new Hello world! It's a beautiful day.");

    // Test case 4: Insert empty string.
    pt.insert(8, "");
    CHECK(pt.length() == 46);
    CHECK(pt.to_string() == "Awesome new Hello world! It's a beautiful day.");
}

// Tests the delete functionality.
TEST_CASE("piece_table Deletion", "[piecetable]")
{
    piece_table pt("Hello, beautiful world!");

    // Test case 1: Delete from the beginning.
    pt.remove(0, 7); // "Hello, "
    CHECK(pt.length() == 16);
    CHECK(pt.to_string() == "beautiful world!");

    // Test case 2: Delete from the middle.
    pt.remove(0, 10); // "beautiful "
    CHECK(pt.length() == 6);
    CHECK(pt.to_string() == "world!");

    // Test case 3: Delete from the end.
    pt.remove(5, 1); // "!"
    CHECK(pt.length() == 5);
    CHECK(pt.to_string() == "world");

    // Test case 4: Delete the entire content.
    pt.remove(0, 5);
    CHECK(pt.length() == 0);
    CHECK(pt.to_string() == "");

    // Test case 5: Delete zero characters.
    piece_table pt2("No change");
    pt2.remove(3, 0);
    CHECK(pt2.length() == 9);
    CHECK(pt2.to_string() == "No change");
}

// Tests a mix of insert and delete operations.
TEST_CASE("piece_table Mixed Operations", "[piecetable]")
{
    piece_table pt("This is a test.");

    // Replace "a test" with "an example"
    pt.remove(8, 6); // "a test"
    pt.insert(8, "an example");
    CHECK(pt.to_string() == "This is an example.");
    CHECK(pt.length() == 19);

    // Prepend text
    pt.insert(0, "Note: ");
    CHECK(pt.to_string() == "Note: This is an example.");
    CHECK(pt.length() == 25);

    // Delete from the middle
    pt.remove(4, 6); // ": This"
    CHECK(pt.to_string() == "Note is an example.");
    CHECK(pt.length() == 19);
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
    CHECK(pt.length() == 100);
    CHECK(pt.to_string() == expected);

    // Test case 2: A long series of small deletes from the start.
    for (int i = 0; i < 100; ++i)
    {
        pt.remove(0, 1);
    }
    CHECK(pt.length() == 0);
    CHECK(pt.to_string() == "");

    // Test case 3: Operations on an empty table.
    piece_table pt_empty;
    pt_empty.insert(0, "hello");
    CHECK(pt_empty.to_string() == "hello");
    pt_empty.remove(0, 5);
    CHECK(pt_empty.to_string() == "");

    // Test case 4: Deleting more characters than available.
    // The behavior here might depend on implementation (throw or clamp).
    // We'll assume it clamps for now. A robust implementation should define this.
    piece_table pt_clamp("short");
    pt_clamp.remove(0, 100); // Try to delete more than exists
    CHECK(pt_clamp.length() == 0);
    CHECK(pt_clamp.to_string() == "");
}

TEST_CASE("piece_table Rigorous Operations", "[piecetable]")
{
    // 1. Clamping on Insert (inserting way past the end)
    piece_table pt("Hello");
    pt.insert(999, " world");
    CHECK(pt.to_string() == "Hello world");
    CHECK(pt.length() == 11);

    // 2. Splitting an ADD piece
    // Document: "Hello world" (1 piece: ADD)
    pt.insert(6, "beautiful ");
    CHECK(pt.to_string() == "Hello beautiful world");

    // 3. Cross-Piece Deletion
    // Document: "Hello beautiful world"
    // H e l l o   b e a u t i f u l   w o r l d
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
    // Let's delete "beautiful " (indices 6 to 15)
    pt.remove(6, 10);
    CHECK(pt.to_string() == "Hello world");

    // 4. Repeated Middle Operations (Heavy Node Splitting)
    piece_table pt2("ABC");
    pt2.insert(1, "1"); // A1BC
    pt2.insert(2, "2"); // A12BC
    pt2.insert(3, "3"); // A123BC
    pt2.remove(1, 3);   // ABC
    CHECK(pt2.to_string() == "ABC");
    CHECK(pt2.length() == 3);

    // 5. Deleting from a single-character piece
    piece_table pt3("A");
    pt3.insert(1, "B");
    pt3.insert(2, "C");
    pt3.remove(1, 1); // remove 'B'
    CHECK(pt3.to_string() == "AC");
}

TEST_CASE("piece_table get_index_for_line", "[piecetable]")
{
    // Case 1: Empty file
    piece_table pt_empty;
    CHECK(pt_empty.get_index_for_line(0) == 0);
    CHECK(pt_empty.get_index_for_line(1) == 0);
    CHECK(pt_empty.get_index_for_line(5) == 0);

    // Case 2: Single piece, multiple lines
    // "Line 1\nLine 2\nLine 3"
    // Indexes:
    // L i n e   1 \n L i n e   2 \n L i n e   3
    // 0 1 2 3 4 5 6  7 8 9 0 1 2 3  4 5 6 7 8 9
    piece_table pt_single("Line 1\nLine 2\nLine 3");
    CHECK(pt_single.get_index_for_line(0) == 0);
    CHECK(pt_single.get_index_for_line(1) == 0);
    CHECK(pt_single.get_index_for_line(2) == 7);
    CHECK(pt_single.get_index_for_line(3) == 14);
    CHECK(pt_single.get_index_for_line(4) == 20); // Past the end returns length

    // Case 3: Fragmented pieces (multi-piece lines)
    piece_table pt_frag("Part 1");
    pt_frag.insert(pt_frag.length(), "\nPart 2\n");
    pt_frag.insert(pt_frag.length(), "Part 3");
    // String: "Part 1\nPart 2\nPart 3"
    // Length: 6 + 1 + 6 + 1 + 6 = 20
    CHECK(pt_frag.to_string() == "Part 1\nPart 2\nPart 3");
    CHECK(pt_frag.get_index_for_line(1) == 0);
    CHECK(pt_frag.get_index_for_line(2) == 7);
    CHECK(pt_frag.get_index_for_line(3) == 14);

    // Case 4: Line split across pieces
    piece_table pt_split("Hello ");
    pt_split.insert(pt_split.length(), "World\n");
    pt_split.insert(pt_split.length(), "Next");
    // String: "Hello World\nNext"
    // Indexes:
    // H e l l o   W o r l d \n N e x t
    // 0 1 2 3 4 5 6 7 8 9 0  1 2 3 4 5
    CHECK(pt_split.get_index_for_line(1) == 0);
    CHECK(pt_split.get_index_for_line(2) == 12);

    // Case 5: Consecutive newlines
    piece_table pt_newlines("A\n\nB");
    CHECK(pt_newlines.get_index_for_line(1) == 0); // 'A'
    CHECK(pt_newlines.get_index_for_line(2) == 2); // First \n
    CHECK(pt_newlines.get_index_for_line(3) == 3); // 'B'
    CHECK(pt_newlines.get_index_for_line(4) == 4); // End
}

TEST_CASE("piece_table: Line operations with edge cases", "[piecetable][edge]")
{
    SECTION("Empty file line operations")
    {
        piece_table pt;
        CHECK(pt.get_line_count() == 0);
        CHECK(pt.get_line(0) == "");
        CHECK(pt.get_line(1) == "");
        CHECK(pt.get_line_length(0) == 0);
        CHECK(pt.get_line_length(1) == 0);
    }

    SECTION("Single character file")
    {
        piece_table pt("a");
        CHECK(pt.get_line_count() == 1);
        CHECK(pt.get_line(1) == "a");
        CHECK(pt.get_line_length(1) == 1);
    }

    SECTION("File with only newlines")
    {
        piece_table pt("\n\n\n");
        CHECK(pt.get_line_count() == 3);
        CHECK(pt.get_line(1) == "");
        CHECK(pt.get_line(2) == "");
        CHECK(pt.get_line(3) == "");
    }

    SECTION("File ending with newline")
    {
        piece_table pt("line1\nline2\n");
        CHECK(pt.get_line_count() == 2);
        CHECK(pt.get_line(1) == "line1");
        CHECK(pt.get_line(2) == "line2");
    }

    SECTION("File not ending with newline")
    {
        piece_table pt("line1\nline2");
        CHECK(pt.get_line_count() == 2);
        CHECK(pt.get_line(1) == "line1");
        CHECK(pt.get_line(2) == "line2");
    }

    SECTION("Very long line")
    {
        std::string long_line(10000, 'x');
        piece_table pt(long_line);
        CHECK(pt.get_line_count() == 1);
        CHECK(pt.get_line(1) == long_line);
        CHECK(pt.get_line_length(1) == 10000);
    }

    SECTION("Multiple insertions on same line")
    {
        piece_table pt("start");
        pt.insert(5, " middle");
        pt.insert(12, " end");
        CHECK(pt.to_string() == "start middle end");
        CHECK(pt.get_line_count() == 1);
    }
}

TEST_CASE("piece_table: Boundary delete operations", "[piecetable][edge]")
{
    SECTION("Delete at exact boundaries")
    {
        piece_table pt("abc\ndef\nghi");
        pt.remove(3, 1); // Delete first newline
        CHECK(pt.to_string() == "abcdef\nghi");
        CHECK(pt.get_line_count() == 2);
    }

    SECTION("Delete spanning multiple lines")
    {
        piece_table pt("line1\nline2\nline3\nline4");
        pt.remove(6, 12); // Delete "line2\nline3\n"
        CHECK(pt.to_string() == "line1\nline4");
        CHECK(pt.get_line_count() == 2);
    }

    SECTION("Delete everything except one character")
    {
        piece_table pt("abcdefgh");
        pt.remove(0, 7);
        CHECK(pt.to_string() == "h");
        CHECK(pt.length() == 1);
    }

    SECTION("Repeated deletion from end")
    {
        piece_table pt("12345");
        for (int i = 0; i < 5; i++)
        {
            pt.remove(pt.length() - 1, 1);
        }
        CHECK(pt.to_string() == "");
        CHECK(pt.length() == 0);
    }
}

TEST_CASE("piece_table: Windows line ending normalization", "[piecetable][edge]")
{
    SECTION("CRLF normalization")
    {
        piece_table pt("line1\r\nline2\r\nline3");
        CHECK(pt.to_string() == "line1\nline2\nline3");
        CHECK(pt.get_line_count() == 3);
    }

    SECTION("Mixed line endings")
    {
        piece_table pt("line1\r\nline2\nline3\r\n");
        CHECK(pt.to_string() == "line1\nline2\nline3\n");
        CHECK(pt.get_line_count() == 3);
    }
}

TEST_CASE("piece_table: get_char_at edge cases", "[piecetable][edge]")
{
    piece_table pt("hello");

    SECTION("Valid indices")
    {
        CHECK(pt.get_char_at(0) == 'h');
        CHECK(pt.get_char_at(4) == 'o');
    }

    SECTION("Out of bounds indices")
    {
        CHECK(pt.get_char_at(5) == '\0');
        CHECK(pt.get_char_at(100) == '\0');
    }

    SECTION("Empty piece table")
    {
        piece_table pt_empty;
        CHECK(pt_empty.get_char_at(0) == '\0');
    }
}
