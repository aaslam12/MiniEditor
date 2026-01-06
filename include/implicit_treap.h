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
        size_t start;  // offset in the buffer
        size_t length; // refers to bytes
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

    // concepts to restrict the callback to correct signature for the for_each function in implicit_treap
    template<typename T>
    concept piece_callback = requires(T func, const piece& p) {
        { func(p) } -> std::convertible_to<bool>;
    };

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

        inline static size_t get_subtree_length(const node* x)
        {
            return x ? x->subtree_length : 0;
        }

        node* find(size_t index, node* current) const;
        void delete_nodes(node* n);
        node* copy_nodes(const node* n); // performs deep copy
        void get_pieces(node* n, std::vector<piece>& pieces) const;

        // helper function that allows you traverse through all nodes in in-order
        // and run a callback function on each of them
        template<piece_callback func_callback>
        bool for_each(node* current, func_callback&& callback) const
        {
            // helper recursive function
            if (!current)
                return false;

            if (for_each(current->left, std::forward<func_callback>(callback)))
                return true;

            if (callback(current->data))
                return true;

            if (for_each(current->right, std::forward<func_callback>(callback)))
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
        void insert(size_t index, const piece& value);
        void erase(size_t index, size_t length);
        void split(node* root, size_t index, node*& left, node*& right);
        node* merge(node* l, node* r);
        size_t size() const;
        bool empty() const;
        void clear();
        void get_pieces(std::vector<AL::piece>& pieces) const;

        // public facing function that allows you traverse through all nodes in in-order
        // and run a callback function on each of them
        // allows short-circuit with the return value
        // return false to keep running. true to stop.
        template<piece_callback func_callback>
        void for_each(func_callback&& callback) const
        {
            for_each(m_root, std::forward<func_callback>(callback));
        }
    };
} // namespace AL
