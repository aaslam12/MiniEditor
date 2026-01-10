#include "implicit_treap.h"

namespace AL
{
void implicit_treap::delete_nodes(node* n)
{
    if (!n)
        return;
    delete_nodes(n->left);
    delete_nodes(n->right);
    delete n;
}

implicit_treap::implicit_treap() : m_root(nullptr)
{}

implicit_treap::~implicit_treap()
{
    delete_nodes(m_root);
}

node* implicit_treap::find(size_t index, node* current) const
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

node* implicit_treap::find(size_t index) const
{
    return find(index, m_root);
}

void implicit_treap::find_by_line(size_t line_number, node* current, node*& n, size_t& byte_offset) const
{
    if (!current)
        return;

    const size_t left_newlines = get_subtree_newlines(current->left);

    if (line_number < left_newlines)
    {
        // entirely in left subtree
        find_by_line(line_number, current->left, n, byte_offset);
    }
    else if (line_number <= left_newlines + 1 + current->data.newline_count)
    {
        // entirely in current node
        byte_offset += get_subtree_length(current->left);
        n = current;
    }
    else
    {
        // entirely in right node
        byte_offset += get_subtree_length(current->left) + current->data.length;
        find_by_line(line_number - 1 - left_newlines - current->data.newline_count, current->right, n, byte_offset);
    }
}

void implicit_treap::find_by_line(size_t line_number, node*& n, size_t& byte_offset) const
{
    byte_offset = 0;
    n = nullptr;
    find_by_line(line_number, m_root, n, byte_offset);
}

void implicit_treap::find_by_byte(size_t index, node* current, node*& n, size_t& byte_offset) const
{
    if (!current)
        return;

    const size_t left_len = get_subtree_length(current->left);

    if (index < left_len)
    {
        // entirely in left subtree
        find_by_byte(index, current->left, n, byte_offset);
    }
    else if (index < left_len + current->data.length)
    {
        // entirely in current node
        byte_offset += left_len;
        n = current;
    }
    else
    {
        // entirely in right subtree
        byte_offset += left_len + current->data.length;
        find_by_byte(index - left_len - current->data.length, current->right, n, byte_offset);
    }
}

void implicit_treap::find_by_byte(size_t index, node*& n, size_t& byte_offset) const
{
    byte_offset = 0;
    n = nullptr;
    find_by_byte(index, m_root, n, byte_offset);
}

size_t implicit_treap::size() const
{
    return get_subtree_length(m_root);
}

size_t implicit_treap::get_newline_count() const
{
    return m_root ? m_root->subtree_newline_count : 0;
}

bool implicit_treap::empty() const
{
    return !m_root;
}

node* implicit_treap::merge(node* l, node* r)
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

node* implicit_treap::copy_nodes(const node* n)
{
    if (!n)
        return nullptr;

    piece p;
    p.buf_type = n->data.buf_type;
    p.length = n->data.length;
    p.start = n->data.start;
    p.newline_count = n->data.newline_count;

    node* new_node = new node(p);
    new_node->priority = n->priority;
    new_node->left = copy_nodes(n->left);
    new_node->right = copy_nodes(n->right);
    new_node->subtree_length = n->subtree_length;
    new_node->subtree_newline_count = n->subtree_newline_count;

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

void implicit_treap::get_pieces(std::vector<piece>& pieces) const
{
    return get_pieces(m_root, pieces);
}
} // namespace AL
