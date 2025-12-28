#pragma once

#include <cstdint>
#include <memory>

/*
 * This is an implicit treap.
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
        size_t size{};
        Node* left;
        Node* right;

        Node(const T& data) : data(data), priority(rng()), left(nullptr), right(nullptr)
        {}
    };

    std::unique_ptr<Node> m_root;

    void update_size(Node* node);
    void split(Node* t, int index, Node*& l, Node*& r);
    Node* merge(Node* l, Node* r);
    Node* find_by_index(int index);

public:

    ImpTreap();
    ~ImpTreap();

    void insert(int index, const T& value);
    void erase(int index);
    T& at(int index);
    int size() const;
    bool empty() const;
};

template<typename T>
ImpTreap<T>::ImpTreap()
{}

template<typename T>
ImpTreap<T>::~ImpTreap()
{}

template<typename T>
void ImpTreap<T>::insert(int index, const T& value)
{}

template<typename T>
void ImpTreap<T>::erase(int index)
{}

template<typename T>
T& ImpTreap<T>::at(int index)
{}

template<typename T>
int ImpTreap<T>::size() const
{}

template<typename T>
bool ImpTreap<T>::empty() const
{
    return m_root ? false : true;
}

template<typename T>
void ImpTreap<T>::update_size(Node* node)
{
    if (!node)
        return;

    node->size = node->left->size + node->right->size + 1;
}

template<typename T>
void ImpTreap<T>::split(Node* t, int index, Node*& l, Node*& r)
{}

template<typename T>
ImpTreap<T>::Node* ImpTreap<T>::merge(Node* l, Node* r)
{}
