#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_version_macros.hpp>
#include <cstddef>
#include <editor.h>
#include <filesystem>
#include <fstream>
#include <string>

using piece_table = AL::piece_table;
using implicit_treap = AL::implicit_treap;

// Helper to create a temp file for testing
static std::filesystem::path create_temp_file(const std::string& name, const std::string& content)
{
    auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream ofs(path);
    if (ofs)
    {
        ofs << content;
    }
    return path;
}

// Helper to read a file's content
static std::string read_file_content(const std::filesystem::path& path)
{
    std::ifstream ifs(path);
    if (!ifs)
    {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

TEST_CASE("Editor: File Operations", "[editor]")
{
    AL::editor ed;

    SECTION("Open a valid file")
    {
        auto path = create_temp_file("test_open.txt", "line 1\nline 2");
        REQUIRE(ed.open(path));
        CHECK(ed.get_total_lines() == 2);
        CHECK(ed.get_line(1) == "line 1");
        CHECK(ed.get_filename() == "test_open.txt");
        std::filesystem::remove(path);
    }

    SECTION("Open a non-existent file")
    {
        CHECK_FALSE(ed.open("non/existent/file.txt"));
    }

    SECTION("Open an empty file")
    {
        auto path = create_temp_file("test_empty.txt", "");
        REQUIRE(ed.open(path));
        CHECK(ed.get_total_lines() == 1);
        CHECK(ed.get_line(1) == "");
        std::filesystem::remove(path);
    }

    SECTION("Save to current file")
    {
        auto path = create_temp_file("test_save.txt", "some content");
        ed.open(path);
        // Modify content (stubs will be called)
        ed.insert_char('A');
        ed.flush_insert_buffer();
        CHECK(ed.is_dirty());
        CHECK(ed.save());
        CHECK_FALSE(ed.is_dirty());
        CHECK(read_file_content(path) == "Asome content");
        std::filesystem::remove(path);
    }

    SECTION("Save to a new file (Save As)")
    {
        auto path = create_temp_file("test_save_as_orig.txt", "original");
        auto new_path = std::filesystem::temp_directory_path() / "test_save_as_new.txt";
        ed.open(path);
        CHECK(ed.save(new_path));
        REQUIRE(std::filesystem::exists(new_path));
        CHECK(ed.get_filename() == "test_save_as_new.txt");
        CHECK(read_file_content(new_path) == "original");
        std::filesystem::remove(path);

        std::filesystem::remove(new_path);
    }
}

TEST_CASE("Editor: Editing Operations (Stubs)", "[editor]")
{
    AL::editor ed;
    auto path = create_temp_file("test_editing.txt", "abc");
    ed.open(path);

    SECTION("Insert character")
    {
        CHECK_NOTHROW(ed.insert_char('x'));
        ed.flush_insert_buffer();
        CHECK(ed.is_dirty());
        CHECK(ed.get_line(1) == "xabc");
    }

    SECTION("Delete character")
    {
        CHECK_NOTHROW(ed.delete_char());
        CHECK(ed.is_dirty());
        // Assuming cursor starts at 0, this might delete nothing or next char
    }
    std::filesystem::remove(path);
}

TEST_CASE("Editor: Cursor Movement (Stubs)", "[editor]")
{
    AL::editor ed;
    auto path = create_temp_file("test_cursor.txt", "line1\nline2");
    ed.open(path);

    size_t initial_row = ed.get_cursor_row();
    size_t initial_col = ed.get_cursor_col();

    SECTION("Move Right")
    {
        CHECK_NOTHROW(ed.move_cursor(AL::direction::RIGHT));
        CHECK(ed.get_cursor_col() > initial_col);
    }
    SECTION("Move Left")
    {
        ed.move_cursor(AL::direction::RIGHT);
        CHECK_NOTHROW(ed.move_cursor(AL::direction::LEFT));
        CHECK(ed.get_cursor_col() == initial_col);
    }
    SECTION("Move Down")
    {
        CHECK_NOTHROW(ed.move_cursor(AL::direction::DOWN));
        CHECK(ed.get_cursor_row() > initial_row);
    }
    SECTION("Move Up")
    {
        ed.move_cursor(AL::direction::DOWN);
        CHECK_NOTHROW(ed.move_cursor(AL::direction::UP));
        CHECK(ed.get_cursor_row() == initial_row);
    }

    SECTION("Move cursor at boundaries")
    {
        // Move to start
        for (int i = 0; i < 10; ++i)
            ed.move_cursor(AL::direction::LEFT);
        CHECK(ed.get_cursor_col() == 1);
        CHECK_NOTHROW(ed.move_cursor(AL::direction::LEFT));
        CHECK(ed.get_cursor_col() == 1);
    }
    std::filesystem::remove(path);
}

TEST_CASE("Editor: State Getters (Stubs)", "[editor]")
{
    AL::editor ed;

    CHECK(ed.get_total_lines() == 1);
    CHECK(ed.get_cursor_row() == 1);
    CHECK(ed.get_cursor_col() == 1);
    CHECK_FALSE(ed.is_dirty());
    CHECK(ed.get_filename().empty());

    auto path = create_temp_file("test_getters.txt", "content");
    ed.open(path);

    CHECK(ed.get_total_lines() == 1);
    CHECK(ed.get_filename() == "test_getters.txt");
    CHECK_FALSE(ed.is_dirty());

    std::filesystem::remove(path);
}

TEST_CASE("Editor: Cursor movement at file boundaries", "[editor][edge]")
{
    AL::editor ed;
    auto path = create_temp_file("cursor_edge.txt", "line1\nline2\nline3");
    ed.open(path);

    SECTION("Up at first line moves to start")
    {
        ed.move_cursor(AL::direction::RIGHT);
        ed.move_cursor(AL::direction::RIGHT);
        ed.move_cursor(AL::direction::UP);
        CHECK(ed.get_cursor_row() == 1);
        CHECK(ed.get_cursor_col() == 1);
    }

    SECTION("Down at last line stays at end")
    {
        // Move to last line
        ed.move_cursor(AL::direction::DOWN);
        ed.move_cursor(AL::direction::DOWN);
        size_t last_row = ed.get_cursor_row();

        // Try to go down again
        ed.move_cursor(AL::direction::DOWN);
        CHECK(ed.get_cursor_row() == last_row);
    }

    SECTION("Left at start of file does nothing")
    {
        ed.move_cursor(AL::direction::LEFT);
        CHECK(ed.get_cursor_row() == 1);
        CHECK(ed.get_cursor_col() == 1);
    }

    SECTION("Right at end of file does nothing")
    {
        // Move to end
        for (int i = 0; i < 100; i++)
        {
            ed.move_cursor(AL::direction::RIGHT);
        }
        size_t row = ed.get_cursor_row();
        size_t col = ed.get_cursor_col();

        ed.move_cursor(AL::direction::RIGHT);
        CHECK(ed.get_cursor_row() == row);
        CHECK(ed.get_cursor_col() == col);
    }

    std::filesystem::remove(path);
}

TEST_CASE("Editor: Delete operations at boundaries", "[editor][edge]")
{
    AL::editor ed;

    SECTION("Delete at start of file does nothing")
    {
        auto path = create_temp_file("delete_start.txt", "test");
        ed.open(path);
        ed.delete_char();
        ed.flush_insert_buffer();
        CHECK(ed.get_line(1) == "test");
        std::filesystem::remove(path);
    }

    SECTION("Delete newline joins lines")
    {
        auto path = create_temp_file("delete_newline.txt", "line1\nline2");
        ed.open(path);

        // Move to start of line 2
        ed.move_cursor(AL::direction::DOWN);
        CHECK(ed.get_cursor_row() == 2);

        // Delete the newline
        ed.delete_char();
        ed.flush_insert_buffer();
        CHECK(ed.get_line(1) == "line1line2");
        CHECK(ed.get_total_lines() == 1);

        std::filesystem::remove(path);
    }
}

TEST_CASE("Editor: Insert operations with newlines", "[editor][edge]")
{
    AL::editor ed;
    auto path = create_temp_file("insert_newline.txt", "test");
    ed.open(path);

    SECTION("Insert newline in middle")
    {
        ed.move_cursor(AL::direction::RIGHT);
        ed.move_cursor(AL::direction::RIGHT);
        ed.insert_char('\n');
        ed.flush_insert_buffer();

        CHECK(ed.get_total_lines() == 2);
        CHECK(ed.get_cursor_row() == 2);
        CHECK(ed.get_cursor_col() == 1);
    }

    SECTION("Insert newline at start")
    {
        ed.insert_char('\n');
        ed.flush_insert_buffer();

        CHECK(ed.get_total_lines() == 2);
        CHECK(ed.get_cursor_row() == 2);
    }

    std::filesystem::remove(path);
}

TEST_CASE("Editor: Column memory during vertical movement", "[editor][edge]")
{
    AL::editor ed;
    auto path = create_temp_file("col_memory.txt", "long line here\nshort\nlong line again");
    ed.open(path);

    // Move to column 10 on first line
    for (int i = 0; i < 10; i++)
    {
        ed.move_cursor(AL::direction::RIGHT);
    }
    CHECK(ed.get_cursor_col() == 11); // Started at 1, moved right 10 times

    // Move down to shorter line
    ed.move_cursor(AL::direction::DOWN);
    CHECK(ed.get_cursor_col() == 6); // Clamped to "short" length (5) + 1

    // Move down again to long line - should restore column 11
    ed.move_cursor(AL::direction::DOWN);
    CHECK(ed.get_cursor_col() == 11);

    std::filesystem::remove(path);
}

TEST_CASE("implicit_treap: Split at exact node boundaries", "[treap][edge]")
{
    const auto split_func = [](AL::piece& left, size_t split_offset) {
        AL::piece right_piece = {
            .buf_type = left.buf_type, .start = left.start + split_offset, .length = left.length - split_offset, .newline_count = 0};
        left.length = split_offset;
        return right_piece;
    };

    implicit_treap treap;

    // Insert pieces
    treap.insert(0, {AL::buffer_type::ADD, 0, 5, 0}, split_func);
    treap.insert(5, {AL::buffer_type::ADD, 5, 5, 0}, split_func);
    treap.insert(10, {AL::buffer_type::ADD, 10, 5, 0}, split_func);

    CHECK(treap.size() == 15);

    SECTION("Erase at exact piece boundary")
    {
        treap.erase(5, 5, split_func);
        CHECK(treap.size() == 10);
    }

    SECTION("Erase spanning multiple pieces")
    {
        treap.erase(3, 7, split_func);
        CHECK(treap.size() == 8); // 3 bytes from first piece + 5 bytes from third piece
    }
}

TEST_CASE("implicit_treap: Zero-length operations", "[treap][edge]")
{
    const auto split_func = [](AL::piece& left, size_t split_offset) {
        AL::piece right_piece = {
            .buf_type = left.buf_type, .start = left.start + split_offset, .length = left.length - split_offset, .newline_count = 0};
        left.length = split_offset;
        return right_piece;
    };

    implicit_treap treap;
    treap.insert(0, {AL::buffer_type::ADD, 0, 10, 0}, split_func);

    SECTION("Insert zero-length piece does nothing")
    {
        size_t before = treap.size();
        treap.insert(5, {AL::buffer_type::ADD, 0, 0, 0}, split_func);
        CHECK(treap.size() == before);
    }

    SECTION("Erase zero length does nothing")
    {
        size_t before = treap.size();
        treap.erase(5, 0, split_func);
        CHECK(treap.size() == before);
    }
}

TEST_CASE("Editor: Empty file operations", "[editor][edge]")
{
    AL::editor ed;
    auto path = create_temp_file("empty_file.txt", "");
    ed.open(path);

    SECTION("Empty file has one line")
    {
        CHECK(ed.get_total_lines() == 1);
        CHECK(ed.get_line(1) == "");
    }

    SECTION("Cursor operations on empty file")
    {
        CHECK(ed.get_cursor_row() == 1);
        CHECK(ed.get_cursor_col() == 1);

        ed.move_cursor(AL::direction::UP);
        CHECK(ed.get_cursor_row() == 1);

        ed.move_cursor(AL::direction::DOWN);
        CHECK(ed.get_cursor_row() == 1);

        ed.move_cursor(AL::direction::LEFT);
        CHECK(ed.get_cursor_col() == 1);

        ed.move_cursor(AL::direction::RIGHT);
        CHECK(ed.get_cursor_col() == 1);
    }

    SECTION("Insert into empty file")
    {
        ed.insert_char('a');
        ed.flush_insert_buffer();
        CHECK(ed.get_line(1) == "a");
    }

    std::filesystem::remove(path);
}
