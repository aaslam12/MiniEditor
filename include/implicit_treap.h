#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>

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
 */
class implicit_treap
{
private:

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

public:

    enum class buffer_type : uint8_t
    {
        ORIGINAL,
        ADD
    };

    struct piece
    {
        buffer_type buf_type;
        size_t start; // offset in the buffer
        size_t length;
    };

private:

    struct node
    {
        piece data;
        uint64_t priority{};
        size_t subtree_length; // size of subtree rooted at this node
        node* left;            // nodes STRICTLY before (not including this node)
        node* right;           // nodes STRICTLY after (not including this node)
        void update_size()
        {
            size_t x = 0;

            if (left)
                x += left->subtree_length;
            if (right)
                x += right->subtree_length;

            subtree_length = x + data.length;
        }

        node(const piece& data) : data(data), priority(rng()), subtree_length(data.length), left(nullptr), right(nullptr)
        {}
    };

    node* m_root;

    inline static size_t get_subtree_length(const node* x)
    {
        return x ? x->subtree_length : 0;
    }

    void delete_nodes(node* n);

public:

    implicit_treap();
    ~implicit_treap();

    void insert(size_t index, const piece& value);
    void erase(size_t index, size_t length);
    void split(node* root, size_t index, node*& left, node*& right);
    node* merge(node* l, node* r);
    size_t size() const;
    bool empty() const;
};
