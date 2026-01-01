#include "implicit_treap.h"

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
