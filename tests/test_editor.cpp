#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_version_macros.hpp>
#include <editor.h>
#include <filesystem>
#include <fstream>
#include <string>

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
        CHECK(ed.get_cursor_col() == 0);
        CHECK_NOTHROW(ed.move_cursor(AL::direction::LEFT));
        CHECK(ed.get_cursor_col() == 0);
    }
    std::filesystem::remove(path);
}

TEST_CASE("Editor: State Getters (Stubs)", "[editor]")
{
    AL::editor ed;

    CHECK(ed.get_total_lines() == 0);
    CHECK(ed.get_cursor_row() == 0);
    CHECK(ed.get_cursor_col() == 0);
    CHECK_FALSE(ed.is_dirty());
    CHECK(ed.get_filename().empty());

    auto path = create_temp_file("test_getters.txt", "content");
    ed.open(path);

    CHECK(ed.get_total_lines() == 1);
    CHECK(ed.get_filename() == "test_getters.txt");
    CHECK_FALSE(ed.is_dirty());

    std::filesystem::remove(path);
}
