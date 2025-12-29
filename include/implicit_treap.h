#pragma once

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
template<typename T>
class ImpTreap
{
private:

    // Using SplitMix64 instead of std random number generators since they will be overkill for a treap
    static inline uint64_t rng()
    {
        static uint64_t x = 0x9e3779b97f4a7c15ULL;
        x += 0x9e3779b97f4a7c15ULL;
        uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    struct Node
    {
        T data;
        uint64_t priority{};
        size_t size{1}; // size of subtree rooted at this node
        Node* left;     // nodes STRICTLY before (not including this node)
        Node* right;    // nodes STRICTLY after (not including this node)

        Node(const T& data) : data(data), priority(rng()), left(nullptr), right(nullptr)
        {}
    };

    std::unique_ptr<Node> m_root;

    size_t get_size(Node* node) const;
    void update_size(Node* node);
    void delete_nodes(Node* node);

    /*
     * should never use root again after this function call. must use merge if you want root to work again
     * index means how many elements go to the left subree (not inclusive)
     * left and right are outputs.
     * */
    void split(Node* root, size_t index, Node*& left, Node*& right);
    Node* merge(Node* l, Node* r);
    Node* find_by_index(size_t index) const;

public:

    ImpTreap();
    ~ImpTreap();

    void insert(size_t index, const T& value);
    void erase(size_t index);
    T& at(size_t index);
    size_t size() const;
    bool empty() const;
};

template<typename T>
ImpTreap<T>::ImpTreap() : m_root(nullptr)
{}

template<typename T>
void ImpTreap<T>::delete_nodes(Node* node)
{
    if (!node)
        return;
    delete_nodes(node->left);
    delete_nodes(node->right);
    delete node;
}

template<typename T>
ImpTreap<T>::~ImpTreap()
{
    delete_nodes(m_root.release());
}

template<typename T>
void ImpTreap<T>::insert(size_t index, const T& value)
{
    Node* l = nullptr;
    Node* r = nullptr;
    Node* new_node = new Node(value);

    Node* current_root = m_root.release();
    split(current_root, index, l, r);
    m_root.reset(merge(merge(l, new_node), r));
}

template<typename T>
void ImpTreap<T>::erase(size_t index)
{
    Node *l, *r, *m;
    Node* current_root = m_root.release();
    split(current_root, index, l, r);
    split(r, 1, m, r);
    delete m;
    m_root.reset(merge(l, r));
}

template<typename T>
T& ImpTreap<T>::at(size_t index)
{
    return find_by_index(index)->data;
}

template<typename T>
size_t ImpTreap<T>::size() const
{
    return get_size(m_root.get());
}

template<typename T>
bool ImpTreap<T>::empty() const
{
    return !m_root;
}

template<typename T>
size_t ImpTreap<T>::get_size(Node* node) const
{
    return node ? node->size : 0;
}

template<typename T>
void ImpTreap<T>::update_size(Node* node)
{
    if (!node)
        return;

    node->size = get_size(node->left) + get_size(node->right) + 1;
}

template<typename T>
void ImpTreap<T>::split(Node* t, size_t index, Node*& l, Node*& r)
{
    if (!t)
    {
        l = nullptr;
        r = nullptr;
        return;
    }

    size_t left_size = get_size(t->left);
    if (index <= left_size)
    {
        split(t->left, index, l, t->left);
        r = t;
    }
    else
    {
        split(t->right, index - left_size - 1, t->right, r);
        l = t;
    }

    update_size(t);
}

template<typename T>
typename ImpTreap<T>::Node* ImpTreap<T>::merge(Node* l, Node* r)
{
    if (!l || !r)
        return l ? l : r;

    if (l->priority > r->priority)
    {
        l->right = merge(l->right, r);
        update_size(l);
        return l;
    }
    else
    {
        r->left = merge(l, r->left);
        update_size(r);
        return r;
    }
}

template<typename T>
typename ImpTreap<T>::Node* ImpTreap<T>::find_by_index(size_t index) const
{
    Node* current = m_root.get();
    while (current)
    {
        size_t left_size = get_size(current->left);
        if (index < left_size)
        {
            current = current->left;
        }
        else if (index > left_size)
        {
            index -= left_size + 1;
            current = current->right;
        }
        else
        {
            return current;
        }
    }
    return nullptr;
}
