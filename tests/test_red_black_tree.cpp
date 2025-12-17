#include "red_black_tree.h"
#include <catch2/catch_test_macros.hpp>

// NOTE: The current implementation of RBT::insert is incomplete and
// contains bugs. These tests reflect the current state and are expected to fail
// or crash. A key issue is that the 'key' member of the Node struct is not
// being set, and the function creates a pointer to a temporary stack object.

TEST_CASE("RBT Insertion", "[RBT]")
{
    RBT<int> rbt;

    // Test case 1: Insert into an empty tree.
    int data1 = 10;
    rbt.insert(data1);

    // Test case 2: Insert a second element.
    int data2 = 5;
    rbt.insert(data2);
}

TEST_CASE("RBT Left Rotation on root", "[RBT]")
{
    RBT<int> rbt;
    // Manually construct a tree for testing left rotation
    //      2 (x)
    //       \
    //        4 (y)
    //       /
    //      3 (T2l)
    rbt.m_root = std::make_unique<RBT<int>::Node>(2);
    auto* x = rbt.m_root.get();
    x->right = std::make_unique<RBT<int>::Node>(4);
    x->right->parent = x;
    auto* y = x->right.get();
    y->left = std::make_unique<RBT<int>::Node>(3);
    y->left->parent = y;
    auto* t2l = y->left.get();

    rbt.left_rotate(rbt.m_root);

    // Check the new structure
    //        4 (y)
    //       /
    //      2 (x)
    //       \
    //        3 (T2l)
    REQUIRE(rbt.m_root.get() == y);
    REQUIRE(y->parent == nullptr);
    REQUIRE(y->left.get() == x);
    REQUIRE(x->parent == y);
    REQUIRE(x->right.get() == t2l);
    REQUIRE(t2l->parent == x);
    REQUIRE(x->left == nullptr);
}

TEST_CASE("RBT Left Rotation with parent", "[RBT]")
{
    RBT<int> rbt;
    //      5 (p)
    //     /
    //    2 (x)
    //     \
    //      4 (y)
    rbt.m_root = std::make_unique<RBT<int>::Node>(5);
    auto* p = rbt.m_root.get();
    p->left = std::make_unique<RBT<int>::Node>(2);
    p->left->parent = p;
    auto* x = p->left.get();
    x->right = std::make_unique<RBT<int>::Node>(4);
    x->right->parent = x;
    auto* y = x->right.get();

    rbt.left_rotate(p->left);

    //      5 (p)
    //     /
    //    4 (y)
    //   /
    //  2 (x)
    REQUIRE(rbt.m_root.get() == p);
    REQUIRE(p->left.get() == y);
    REQUIRE(y->parent == p);
    REQUIRE(y->left.get() == x);
    REQUIRE(x->parent == y);
    REQUIRE(x->right == nullptr);
    REQUIRE(y->right == nullptr);
}

TEST_CASE("RBT Right Rotation on root", "[RBT]")
{
    RBT<int> rbt;
    //        4 (y)
    //       /
    //      2 (x)
    //       \
    //        3 (T2)
    rbt.m_root = std::make_unique<RBT<int>::Node>(4);
    auto* y = rbt.m_root.get();
    y->left = std::make_unique<RBT<int>::Node>(2);
    y->left->parent = y;
    auto* x = y->left.get();
    x->right = std::make_unique<RBT<int>::Node>(3);
    x->right->parent = x;
    auto* t2 = x->right.get();

    rbt.right_rotate(rbt.m_root);

    //      2 (x)
    //       \
    //        4 (y)
    //       /
    //      3 (T2)
    REQUIRE(rbt.m_root.get() == x);
    REQUIRE(x->parent == nullptr);
    REQUIRE(x->right.get() == y);
    REQUIRE(y->parent == x);
    REQUIRE(y->left.get() == t2);
    REQUIRE(t2->parent == y);
    REQUIRE(y->right == nullptr);
    REQUIRE(x->left == nullptr);
}

TEST_CASE("RBT Right Rotation with parent", "[RBT]")
{
    RBT<int> rbt;
    //      5 (p)
    //     /
    //    4 (y)
    //   /
    //  2 (x)
    rbt.m_root = std::make_unique<RBT<int>::Node>(5);
    auto* p = rbt.m_root.get();
    p->left = std::make_unique<RBT<int>::Node>(4);
    p->left->parent = p;
    auto* y = p->left.get();
    y->left = std::make_unique<RBT<int>::Node>(2);
    y->left->parent = y;
    auto* x = y->left.get();

    rbt.right_rotate(p->left);

    //      5 (p)
    //     /
    //    2 (x)
    //     \
    //      4 (y)
    REQUIRE(rbt.m_root.get() == p);
    REQUIRE(p->left.get() == x);
    REQUIRE(x->parent == p);
    REQUIRE(x->right.get() == y);
    REQUIRE(y->parent == x);
    REQUIRE(y->left == nullptr);
    REQUIRE(x->left == nullptr);
}
