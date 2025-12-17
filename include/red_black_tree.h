#pragma once

#include <cstddef> // for size_t
#include <cstdint>
#include <memory>
#include <utility>

template<typename T>
class RBT
{
public:

    enum class Color : std::uint8_t
    {
        RED,
        BLACK
    };

    struct Node
    {
        const T data;
        size_t key{};
        Color color;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        Node* parent;

        Node(const T _data) : data(_data), color(Color::RED), parent(nullptr)
        {}
    };

    RBT();
    RBT(RBT&&) = default;
    RBT(const RBT&) = default;
    RBT& operator=(RBT&&) = default;
    RBT& operator=(const RBT&) = default;
    ~RBT();

    void insert(const T data);
    std::pair<Node*, size_t> find_by_index(size_t index);
    Node* split_at_index(size_t index);
    // Node* insert_before(Node* pos, const Payload& p);
    void erase_range(size_t start_index, size_t len);
    // void inorder_traverse(Function f);
    size_t total_weight();

    std::unique_ptr<Node> m_root;

    enum class subtree_side : uint8_t
    {
        LEFT,
        RIGHT,
        INVALID
    };

    // goes to parent and checks if the current node is in its right or left subtree
    subtree_side is_in_right_subtree(Node* current);

    void insert_fixup(Node* current);

    // where current is the "higher" node in the tree before any rotations
    void left_rotate(std::unique_ptr<Node>& A);
    void right_rotate(std::unique_ptr<Node>& A);

    Node* get_uncle(Node* current);
    Node* get_sibling(Node* current);
    Node* get_grandparent(Node* current);
    Node* get_near_node(Node* current);
    Node* get_far_node(Node* current);
    bool is_near_node(Node* current, Node* other);
    bool is_far_node(Node* current, Node* other);
};

template<typename T>
RBT<T>::RBT()
{}

template<typename T>
RBT<T>::~RBT()
{}

template<typename T>
void RBT<T>::insert(const T data)
{
    auto new_node = std::make_unique<Node>(data);

    if (!m_root)
    {
        m_root = std::move(new_node);
        m_root->color = Color::BLACK; // Root must be black
        return;
    }

    Node* current = m_root.get();
    while (true)
    {
        if (new_node->key < current->key)
        {
            if (current->left)
            {
                current = current->left.get();
            }
            else
            {
                new_node->parent = current;
                current->left = std::move(new_node);
                insert_fixup(current->left.get());
                return;
            }
        }
        else
        {
            if (current->right)
            {
                current = current->right.get();
            }
            else
            {
                new_node->parent = current;
                current->right = std::move(new_node);
                insert_fixup(current->right.get());
                return;
            }
        }
    }
}

template<typename T>
void RBT<T>::insert_fixup(Node* z)
{}

template<typename T>
RBT<T>::subtree_side RBT<T>::is_in_right_subtree(Node* current)
{
    if (!current || !current->parent)
        return RBT<T>::subtree_side::INVALID;

    return (current->parent->right && current->parent->right.get() == current) ? RBT<T>::subtree_side::RIGHT : RBT<T>::subtree_side::LEFT;
}

template<typename T>
void RBT<T>::left_rotate(std::unique_ptr<Node>& current)
{
    if (!current)
        return;
    if (!current->right)
        return;

    RBT<T>::subtree_side side = is_in_right_subtree(current.get());
    Node* old_parent = current->parent;

    std::unique_ptr<Node> old_right = std::move(current->right);
    current->right = std::move(old_right->left);
    if (current->right)
    {
        current->right->parent = current.get();
    }

    old_right->left = std::move(current);
    old_right->left->parent = old_right.get();
    old_right->parent = old_parent;

    if (old_parent)
    {
        if (side == RBT<T>::subtree_side::RIGHT)
        {
            old_parent->right = std::move(old_right);
        }
        else if (side == RBT<T>::subtree_side::LEFT)
        {
            old_parent->left = std::move(old_right);
        }
    }
    else
    {
        m_root = std::move(old_right);
        m_root->color = Color::BLACK;
    }
}

template<typename T>
void RBT<T>::right_rotate(std::unique_ptr<Node>& current)
{
    if (!current)
        return;
    if (!current->left)
        return; // can't right rotate without left child

    RBT<T>::subtree_side side = is_in_right_subtree(current.get());
    Node* old_parent = current->parent;

    std::unique_ptr<Node> old_left = std::move(current->left);
    current->left = std::move(old_left->right);

    if (current->left)
    {
        current->left->parent = current.get();
    }

    old_left->right = std::move(current);
    old_left->right->parent = old_left.get();
    old_left->parent = old_parent;

    if (old_parent)
    {
        if (side == RBT<T>::subtree_side::RIGHT)
        {
            old_parent->right = std::move(old_left);
        }
        else if (side == RBT<T>::subtree_side::LEFT)
        {
            old_parent->left = std::move(old_left);
        }
    }
    else
    {
        m_root = std::move(old_left);
        m_root->color = Color::BLACK;
    }
}

template<typename T>
RBT<T>::Node* RBT<T>::get_uncle(Node* current)
{
    auto gp = get_grandparent(current);
    if (!gp || !current)
        return nullptr;

    // means the current node was on the right subtree
    if (gp->right.get() == current->parent.get())
    {
        return gp->left.get();
    }
    else if (gp->left.get() == current->parent.get()) // means the current node was on the left subtree
    {
        return gp->right.get();
    }
}

template<typename T>
RBT<T>::Node* RBT<T>::get_sibling(Node* current)
{
    if (!current || !current->parent) // means it is either an invalid node or a root node
        return nullptr;
    constexpr auto p = current->parent;

    if (is_in_right_subtree(current))
    {
        return p->left;
    }
    else
    {
        return p->right;
    }
}

template<typename T>
RBT<T>::Node* RBT<T>::get_grandparent(Node* current)
{
    if (!current || !current->parent)
        return nullptr;
    return current->parent->parent;
}

template<typename T>
RBT<T>::Node* RBT<T>::get_near_node(Node* current)
{
    if (!current || !current->parent)
        return nullptr;

    if (is_in_right_subtree(current))
    {
        if (current->parent->left)
            return current->parent->left->right;
    }
    else
    {
        if (current->parent->right)
            return current->parent->right->left;
    }

    return nullptr;
}

template<typename T>
RBT<T>::Node* RBT<T>::get_far_node(Node* current)
{
    if (!current || !current->parent)
        return nullptr;

    if (is_in_right_subtree(current))
    {
        if (current->parent->left)
            return current->parent->left->left;
    }
    else
    {
        if (current->parent->right)
            return current->parent->right->right;
    }

    return nullptr;
}

template<typename T>
bool RBT<T>::is_near_node(Node* current, Node* other)
{
    if (!current || !current->parent)
        return false;

    if (is_in_right_subtree(current))
    {
        if (current->parent->left)
            return current->parent->left->right == other;
    }
    else
    {
        if (current->parent->right)
            return current->parent->right->left == other;
    }

    return false;
}

template<typename T>
bool RBT<T>::is_far_node(Node* current, Node* other)
{
    if (!current || !current->parent)
        return false;

    if (is_in_right_subtree(current))
    {
        if (current->parent->left)
            return current->parent->left->left == other;
    }
    else
    {
        if (current->parent->right)
            return current->parent->right->right == other;
    }

    return false;
}
