#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <implicit_treap.h>
#include <iterator>
#include <string>

using implicit_treap = AL::implicit_treap;
using buffer_type = AL::buffer_type;

const auto split_func = [](AL::piece& left, size_t split_offset) {
    AL::piece right_piece = {.buf_type = left.buf_type, .start = left.start + split_offset, .length = left.length - split_offset, .newline_count = 0};
    left.length = split_offset;
    return right_piece;
};

// Tests for implicit_treap Initialization
TEST_CASE("implicit_treap Initialization", "[ImplicitTreap]")
{
    implicit_treap treap;
    CHECK(treap.size() == 0);
    CHECK(treap.empty() == true);
}

// Tests for Insertion
TEST_CASE("implicit_treap Insertion", "[ImplicitTreap]")
{
    implicit_treap treap;

    // Insert at the beginning of an empty treap
    treap.insert(0, {.buf_type = buffer_type::ADD, .start = 0, .length = 1, .newline_count = 0}, split_func);
    CHECK(treap.size() == 1);

    // Insert at the end
    treap.insert(1, {.buf_type = buffer_type::ADD, .start = 1, .length = 1, .newline_count = 0}, split_func);
    CHECK(treap.size() == 2);

    // Insert in the middle
    treap.insert(1, {.buf_type = buffer_type::ADD, .start = 2, .length = 1, .newline_count = 0}, split_func);
    CHECK(treap.size() == 3);

    // Insert at the front
    treap.insert(0, {.buf_type = buffer_type::ADD, .start = 3, .length = 1, .newline_count = 0}, split_func);
    CHECK(treap.size() == 4);
}

// Tests for Deletion
TEST_CASE("implicit_treap Deletion", "[ImplicitTreap]")
{
    implicit_treap treap;
    treap.insert(0, {.buf_type = buffer_type::ADD, .start = 0, .length = 1, .newline_count = 0}, split_func);
    treap.insert(1, {.buf_type = buffer_type::ADD, .start = 1, .length = 1, .newline_count = 0}, split_func);
    treap.insert(2, {.buf_type = buffer_type::ADD, .start = 2, .length = 1, .newline_count = 0}, split_func);
    treap.insert(3, {.buf_type = buffer_type::ADD, .start = 3, .length = 1, .newline_count = 0}, split_func);
    CHECK(treap.size() == 4);

    // Delete from the middle
    treap.erase(1, 1, split_func); // delete 2nd item
    CHECK(treap.size() == 3);

    // Delete from the beginning
    treap.erase(0, 1, split_func); // delete 1st item
    CHECK(treap.size() == 2);

    // Delete from the end
    treap.erase(1, 1, split_func); // delete last item (at index 1 of 2)
    CHECK(treap.size() == 1);

    // Delete the last element
    treap.erase(0, 1, split_func);
    CHECK(treap.size() == 0);
    CHECK(treap.empty() == true);
}

// Tests for Mixed Operations
TEST_CASE("implicit_treap Mixed Operations", "[ImplicitTreap]")
{
    implicit_treap treap;
    std::string text = "This is a test";
    treap.insert(0, {.buf_type = buffer_type::ORIGINAL, .start = 0, .length = text.length(), .newline_count = 0}, split_func);

    CHECK(treap.size() == text.length());

    // Erase " a"
    treap.erase(7, 2, split_func);
    CHECK(treap.size() == text.length() - 2);

    // Insert " an example"
    std::string insert_text = "an example";
    treap.insert(7, {.buf_type = buffer_type::ADD, .start = 0, .length = insert_text.length(), .newline_count = 0}, split_func);

    std::string expected = "This is an exampletest";
    CHECK(treap.size() == expected.length());
}

// Tests for Edge Cases
TEST_CASE("implicit_treap Edge Cases", "[ImplicitTreap]")
{
    implicit_treap treap;

    // Operations on an empty treap
    REQUIRE_NOTHROW(treap.insert(0, {buffer_type::ADD, 0, 1, 0}, split_func));
    CHECK(treap.size() == 1);
    REQUIRE_NOTHROW(treap.erase(0, 1, split_func));
    CHECK(treap.size() == 0);

    // Large number of insertions
    size_t large_number = 1000;
    for (size_t i = 0; i < large_number; ++i)
    {
        treap.insert(i, {.buf_type = buffer_type::ADD, .start = (size_t)i, .length = 1, .newline_count = 0}, split_func);
    }
    CHECK(treap.size() == large_number);

    // Large number of deletions from the front
    for (size_t i = 0; i < large_number; ++i)
    {
        treap.erase(0, 1, split_func);
    }
    CHECK(treap.size() == 0);
    CHECK(treap.empty() == true);
}
