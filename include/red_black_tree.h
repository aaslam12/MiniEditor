#pragma once

#include <cstddef> // for size_t
#include <cstdint>
#include <utility>

template<typename T>
class red_black_tree
{
    enum class Color : std::uint8_t
    {
        RED,
        BLACK
    };

    struct Node
    {
        T data;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
    };

public:

    red_black_tree();
    red_black_tree(red_black_tree&&)                 = default;
    red_black_tree(const red_black_tree&)            = default;
    red_black_tree& operator=(red_black_tree&&)      = default;
    red_black_tree& operator=(const red_black_tree&) = default;
    ~red_black_tree();

    std::pair<Node*, size_t> find_by_index(size_t index);
    Node* split_at_index(size_t index);
    // Node* insert_before(Node* pos, const Payload& p);
    void erase_range(size_t start_index, size_t len);
    // void inorder_traverse(Function f);
    size_t total_weight();

private:
};
