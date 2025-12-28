#include <catch2/catch_test_macros.hpp>

// Tests for ImplicitTreap Initialization
TEST_CASE("ImplicitTreap Initialization", "[ImplicitTreap]")
{
    ImplicitTreap<int> treap;
    REQUIRE(treap.size() == 0);
    REQUIRE(treap.empty() == true);
}

// Tests for Insertion
TEST_CASE("ImplicitTreap Insertion", "[ImplicitTreap]")
{
    ImplicitTreap<char> treap;

    // Insert at the beginning of an empty treap
    treap.insert(0, 'a');
    REQUIRE(treap.size() == 1);
    REQUIRE(treap.at(0) == 'a');

    // Insert at the end
    treap.insert(1, 'c');
    REQUIRE(treap.size() == 2);
    REQUIRE(treap.at(0) == 'a');
    REQUIRE(treap.at(1) == 'c');

    // Insert in the middle
    treap.insert(1, 'b');
    REQUIRE(treap.size() == 3);
    REQUIRE(treap.at(0) == 'a');
    REQUIRE(treap.at(1) == 'b');
    REQUIRE(treap.at(2) == 'c');

    // Insert at the front
    treap.insert(0, 'x');
    REQUIRE(treap.size() == 4);
    REQUIRE(treap.at(0) == 'x');
    REQUIRE(treap.at(1) == 'a');
    REQUIRE(treap.at(2) == 'b');
    REQUIRE(treap.at(3) == 'c');
}

// Tests for Deletion
TEST_CASE("ImplicitTreap Deletion", "[ImplicitTreap]")
{
    ImplicitTreap<int> treap;
    treap.insert(0, 10);
    treap.insert(1, 20);
    treap.insert(2, 30);
    treap.insert(3, 40);

    // Delete from the middle
    treap.erase(1); // delete 20
    REQUIRE(treap.size() == 3);
    REQUIRE(treap.at(0) == 10);
    REQUIRE(treap.at(1) == 30);
    REQUIRE(treap.at(2) == 40);

    // Delete from the beginning
    treap.erase(0); // delete 10
    REQUIRE(treap.size() == 2);
    REQUIRE(treap.at(0) == 30);
    REQUIRE(treap.at(1) == 40);

    // Delete from the end
    treap.erase(1); // delete 40
    REQUIRE(treap.size() == 1);
    REQUIRE(treap.at(0) == 30);

    // Delete the last element
    treap.erase(0);
    REQUIRE(treap.size() == 0);
    REQUIRE(treap.empty() == true);
}

// Tests for Access (at method)
TEST_CASE("ImplicitTreap Access", "[ImplicitTreap]")
{
    ImplicitTreap<std::string> treap;
    treap.insert(0, "hello");
    treap.insert(1, "world");
    treap.insert(2, "!");

    REQUIRE(treap.at(0) == "hello");
    REQUIRE(treap.at(1) == "world");
    REQUIRE(treap.at(2) == "!");

    // Test modification through reference
    treap.at(1) = "beautiful";
    REQUIRE(treap.at(1) == "beautiful");
}

// Tests for Mixed Operations
TEST_CASE("ImplicitTreap Mixed Operations", "[ImplicitTreap]")
{
    ImplicitTreap<char> treap;
    std::string text = "This is a test";
    for (size_t i = 0; i < text.length(); ++i)
    {
        treap.insert(i, text[i]);
    }

    REQUIRE(treap.size() == text.length());

    // Erase " a"
    treap.erase(7); // ' '
    treap.erase(7); // 'a'
    REQUIRE(treap.size() == text.length() - 2);
    REQUIRE(treap.at(7) == ' ');

    // Insert " an example"
    std::string insert_text = "an example";
    for (size_t i = 0; i < insert_text.length(); ++i)
    {
        treap.insert(8 + i, insert_text[i]);
    }

    std::string expected = "This is an example test";
    REQUIRE(treap.size() == expected.length());
    for (size_t i = 0; i < expected.length(); ++i)
    {
        REQUIRE(treap.at(i) == expected[i]);
    }
}

// Tests for Edge Cases
TEST_CASE("ImplicitTreap Edge Cases", "[ImplicitTreap]")
{
    ImplicitTreap<int> treap;

    // Operations on an empty treap
    REQUIRE_NOTHROW(treap.insert(0, 1));
    REQUIRE(treap.size() == 1);
    REQUIRE_NOTHROW(treap.erase(0));
    REQUIRE(treap.size() == 0);

    // Large number of insertions
    int large_number = 1000;
    for (int i = 0; i < large_number; ++i)
    {
        treap.insert(i, i);
    }
    REQUIRE(treap.size() == large_number);
    for (int i = 0; i < large_number; ++i)
    {
        REQUIRE(treap.at(i) == i);
    }

    // Large number of deletions from the front
    for (int i = 0; i < large_number; ++i)
    {
        treap.erase(0);
    }
    REQUIRE(treap.size() == 0);
    REQUIRE(treap.empty() == true);
}
