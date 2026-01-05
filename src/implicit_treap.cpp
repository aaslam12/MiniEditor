#include "implicit_treap.h"
#include <chrono>

void implicit_treap::delete_nodes(node* n)
{
    if (!n)
        return;
    delete_nodes(n->left);
    delete_nodes(n->right);
    delete n;
}

implicit_treap::implicit_treap() : m_root(nullptr)
{
    rng(std::chrono::steady_clock::now().time_since_epoch().count());
}

implicit_treap::~implicit_treap()
{
    delete_nodes(m_root);
}

implicit_treap::node* implicit_treap::find(size_t index, node* current) const
{
    if (!current)
        return nullptr;

    const size_t left_len = get_subtree_length(current->left);

    if (index < left_len)
    {
        // entirely in left subtree
        return find(index, current->left);
    }
    else if (index < left_len + current->data.length)
    {
        // entirely in current node
        return current;
    }
    else
    {
        // entirely in right node
        return find(index - left_len - current->data.length, current->right);
    }

    return nullptr;
}

implicit_treap::node* implicit_treap::find(size_t index) const
{
    return find(index, m_root);
}

void implicit_treap::insert(size_t index, const piece& value)
{
    if (value.length == 0)
        return;

    node *l = nullptr, *r = nullptr;
    node* new_node = new node(value);

    split(m_root, index, l, r);
    m_root = merge(merge(l, new_node), r);
}

void implicit_treap::erase(size_t index, size_t length)
{
    if (length == 0)
        return;

    node *l, *r, *m;
    split(m_root, index, l, r);
    split(r, length, m, r);
    delete_nodes(m);
    m_root = merge(l, r);
}

size_t implicit_treap::size() const
{
    return get_subtree_length(m_root);
}

bool implicit_treap::empty() const
{
    return !m_root;
}

void implicit_treap::split(implicit_treap::node* current, size_t index, node*& l, node*& r)
{
    if (!current)
    {
        l = r = nullptr;
        return;
    }

    size_t left_len = get_subtree_length(current->left);
    if (index <= left_len)
    {
        split(current->left, index, l, current->left);
        r = current;
    }
    else if (index > left_len && index < left_len + current->data.length)
    {
        // We need to split the current node
        size_t split_offset = index - left_len;

        // The right part of the piece becomes a new node
        piece right_piece = {
            .buf_type = current->data.buf_type, .start = current->data.start + split_offset, .length = current->data.length - split_offset};
        node* new_node = new node(right_piece);

        // The current node is truncated to become the left part
        current->data.length = split_offset;

        // The new_node is inserted to the right of current
        new_node->right = current->right;
        current->right = new_node;

        // Now we can split normally. The split point is right after `current`.
        split(current, index, l, r);
    }
    else
    {
        split(current->right, index - left_len - current->data.length, current->right, r);
        l = current;
    }

    current->update_size();
}

implicit_treap::node* implicit_treap::merge(node* l, node* r)
{
    if (!l || !r)
        return l ? l : r;

    if (l->priority > r->priority)
    {
        l->right = merge(l->right, r);
        l->update_size();
        return l;
    }
    else
    {
        r->left = merge(l, r->left);
        r->update_size();
        return r;
    }
}

implicit_treap::node* implicit_treap::copy_nodes(const node* n)
{
    if (!n)
        return nullptr;

    piece p;
    p.buf_type = n->data.buf_type;
    p.length = n->data.length;
    p.start = n->data.start;

    node* new_node = new node(p);
    new_node->priority = n->priority;
    new_node->left = copy_nodes(n->left);
    new_node->right = copy_nodes(n->right);
    new_node->subtree_length = n->subtree_length;

    return new_node;
}

void implicit_treap::get_pieces(node* n, std::vector<piece>& pieces) const
{
    if (!n)
        return;

    get_pieces(n->left, pieces);
    pieces.push_back(n->data);
    get_pieces(n->right, pieces);
}

implicit_treap::implicit_treap(const implicit_treap& other) : m_root(copy_nodes(other.m_root))
{}

implicit_treap& implicit_treap::operator=(const implicit_treap& other)
{
    if (this != &other)
    {
        clear();
        m_root = copy_nodes(other.m_root);
    }
    return *this;
}

implicit_treap::implicit_treap(implicit_treap&& other) noexcept : m_root(other.m_root)
{
    other.m_root = nullptr;
}

implicit_treap& implicit_treap::operator=(implicit_treap&& other) noexcept
{
    if (this == &other)
        return *this;

    clear();
    m_root = other.m_root;
    other.m_root = nullptr;
    return *this;
}

void implicit_treap::clear()
{
    delete_nodes(m_root);
    m_root = nullptr;
}

void implicit_treap::get_pieces(std::vector<implicit_treap::piece>& pieces) const
{
    return get_pieces(m_root, pieces);
}

void implicit_treap::for_each(node* current, const std::function<void(const piece&)>& callback) const
{
    // helper recursive function
    if (!current)
        return;

    for_each(current->left, callback);
    callback(current->data);
    for_each(current->right, callback);
}

void implicit_treap::for_each(const std::function<void(const piece&)>& callback) const
{
    // public facing function
    if (m_root)
        for_each(m_root, callback);
}
