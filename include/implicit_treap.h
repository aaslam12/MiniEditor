#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace AL
{
enum class buffer_type : uint8_t
{
    ORIGINAL,
    ADD
};

struct piece
{
    buffer_type buf_type;
    size_t start;         // offset in the buffer
    size_t length;        // refers to bytes
    size_t newline_count; // how many newlines in THIS piece
};

struct node
{
    // Using SplitMix64 to randomly generate priority
    // This is good enough and faster than using RNG from std
    static inline uint64_t rng(uint64_t seed = 0x9e3779b97f4a7c15ULL)
    {
        static uint64_t x = seed;
        x += 0x9e3779b97f4a7c15ULL;
        uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    piece data;
    uint64_t priority{};
    size_t subtree_length;        // size of subtree rooted at this node
    size_t subtree_newline_count; // how many newlines in entire subtree
    node* left;                   // nodes STRICTLY before (not including this node)
    node* right;                  // nodes STRICTLY after (not including this node)
    void update_size()
    {
        size_t x = 0;
        size_t y = 0;

        if (left)
        {
            x += left->subtree_length;
            y += left->subtree_newline_count;
        }
        if (right)
        {
            x += right->subtree_length;
            y += right->subtree_newline_count;
        }

        subtree_newline_count = y + data.newline_count;
        subtree_length = x + data.length;
    }

    node(const piece& data)
        : data(data), priority(rng()), subtree_length(data.length), subtree_newline_count(data.newline_count), left(nullptr), right(nullptr)
    {}
};

// concepts to restrict the callback to correct signature for the for_each function
template<typename T>
concept piece_callback = requires(T func, const piece& p) {
    { func(p) } -> std::convertible_to<bool>;
};

// concepts to restrict the callback to correct signature for the split comparator function
// template<typename T>
// concept split_strategy = requires(T func, piece& p, const size_t split_offset) {
//     { func(p, split_offset) } -> std::convertible_to<piece>;
// };

/*
 * This is an implicit treap.
 *
 *          D
 *         / \
 *        B   F
 *       / \ / \
 *      A  C E  G
 *
 *  An in-order traversal gives:
 *  A B C D E F G
 *
 *  Length refers to bytes.
 */
class implicit_treap
{
private:
    node* m_root;

public:
    inline static size_t get_subtree_length(const node* x)
    {
        return x ? x->subtree_length : 0;
    }

    inline static size_t get_subtree_newlines(const node* x)
    {
        return x ? x->subtree_newline_count : 0;
    }

private:
    node* merge(node* l, node* r);

    // private helper recursive functions
    node* find(size_t index, node* current) const;
    void find_by_line(size_t line_number, node* current, node*& n, size_t& byte_offset) const;
    void find_by_byte(size_t index, node* current, node*& n, size_t& byte_offset) const;
    void find_line_position(size_t target_line, node* current, size_t lines_before, node*& n, size_t& byte_offset, size_t& line_in_piece) const;
    void delete_nodes(node* n);
    node* copy_nodes(const node* n); // performs deep copy
    void get_pieces(node* n, std::vector<piece>& pieces) const;

    // helper function allows you traverse through all nodes in the subtree of the specified node in in-order
    // and run a callback function on each of them
    template<piece_callback func_callback>
    bool for_each_internal(node* current, func_callback&& callback) const
    {
        // helper recursive function
        if (!current)
            return false;

        if (for_each_internal(current->left, std::forward<func_callback>(callback)))
            return true;

        if (callback(current->data))
            return true;

        if (for_each_internal(current->right, std::forward<func_callback>(callback)))
            return true;

        return false;
    }

public:
    implicit_treap();
    ~implicit_treap();

    implicit_treap(const implicit_treap& other);
    implicit_treap& operator=(const implicit_treap& other);
    implicit_treap(implicit_treap&& other) noexcept;
    implicit_treap& operator=(implicit_treap&& other) noexcept;

    // finds and returns the node with the containing index
    node* find(size_t index) const;
    void find_by_line(size_t line_number, node*& n, size_t& byte_offset) const;
    void find_by_byte(size_t index, node*& n, size_t& byte_offset) const;

    // find which node contains the start of target_line and return the line number relative to that piece
    // returns byte_offset to start of piece, and line_in_piece (1-indexed within piece)
    void find_line_position(size_t target_line, node*& n, size_t& byte_offset, size_t& line_in_piece) const;

    size_t size() const;
    size_t get_newline_count() const;
    bool empty() const;
    void clear();
    void get_pieces(std::vector<AL::piece>& pieces) const;

    // allows you traverse through all nodes in in-order
    // and run a callback function on each of them
    // allows short-circuit with the return value
    // return false to keep running. true to stop.
    template<piece_callback func_callback>
    void for_each(func_callback&& callback) const
    {
        for_each_internal(m_root, std::forward<func_callback>(callback));
    }

    // helper function allows you traverse through all nodes in the subtree of the specified node in in-order
    // and run a callback function on each of them
    // allows short-circuit with the return value
    // return false to keep running. true to stop.
    template<piece_callback func_callback>
    void for_each(node* n, func_callback&& callback) const
    {
        for_each_internal(n, std::forward<func_callback>(callback));
    }

    template<typename split_strategy>
    void insert(size_t index, const piece& value, split_strategy&& callback)
    {
        if (value.length == 0)
            return;

        node *l = nullptr, *r = nullptr;
        node* new_node = new node(value);

        split(m_root, index, l, r, std::forward<split_strategy>(callback));
        m_root = merge(merge(l, new_node), r);
    }

    template<typename split_strategy>
    void erase(size_t index, size_t length, split_strategy&& callback)
    {
        if (length == 0)
            return;

        node *l, *r, *m;
        split(m_root, index, l, r, callback);
        split(r, length, m, r, callback);
        delete_nodes(m);
        m_root = merge(l, r);
    }

    // callback should handle how the right node should be split
    // 1. Modify the original piece to become the "Left Half".
    // 2. Create and return a new piece that represents the "Right Half".
    template<typename split_strategy>
    void split(node* current, size_t index, node*& l, node*& r, split_strategy&& callback)
    {
        if (!current)
        {
            l = r = nullptr;
            return;
        }

        size_t left_len = get_subtree_length(current->left);
        if (index <= left_len)
        {
            split(current->left, index, l, current->left, callback);
            r = current;
        }
        else if (index > left_len && index < left_len + current->data.length)
        {
            // We need to split the current node
            size_t split_offset = index - left_len;

            // The right part of the piece becomes a new node
            piece right_piece = callback(current->data, split_offset);
            node* new_node = new node(right_piece);

            // The current node is truncated to become the left part
            // Note: callback already updated current->data.newline_count and right_piece.newline_count

            // The new_node is inserted to the right of current
            new_node->right = current->right;
            current->right = new_node;

            // Now we can split normally. The split point is right after `current`.
            split(current, index, l, r, callback);
        }
        else
        {
            split(current->right, index - left_len - current->data.length, current->right, r, callback);
            l = current;
        }

        current->update_size();
    }
};
} // namespace AL
