#pragma once

#include "slab.h"
#include <array>
#include <new>

namespace AL
{

// Global slab config for general fixed-size hot allocations.
// Avoid routing std::string/std::vector through this; use it only for known-size structs.
using global_slab_config = slab_config<>;
using global_slab_t = slab<global_slab_config>;

inline global_slab_t& get_global_slab()
{
    alignas(global_slab_t) static unsigned char storage[sizeof(global_slab_t)];
    static global_slab_t* ptr = ::new (storage) global_slab_t();
    return *ptr;
}

// Slab config for implicit_treap nodes only
inline constexpr std::array<size_class, 1> TREAP_NODE_SIZE_CLASSES = {
    size_class{.byte_size = 128, .num_blocks = 4096, .batch_size = 128}
};

using treap_node_config = slab_config<1, TREAP_NODE_SIZE_CLASSES, 1>;
using treap_node_slab_t = slab<treap_node_config>;

// Global treap node allocator
inline treap_node_slab_t& get_treap_slab()
{
    alignas(treap_node_slab_t) static unsigned char storage[sizeof(treap_node_slab_t)];
    static treap_node_slab_t* ptr = ::new (storage) treap_node_slab_t();
    return *ptr;
}

} // namespace AL
