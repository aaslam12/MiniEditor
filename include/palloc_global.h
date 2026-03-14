#pragma once

#include "dynamic_slab.h"
#include <cstddef>
#include <new>

namespace AL
{

// Global slab config for implicit_treap nodes
inline constexpr std::array<size_class, 1> TREAP_NODE_SIZE_CLASSES = {
    size_class{.byte_size = 128, .num_blocks = 4096, .batch_size = 128}
};

using treap_node_config  = slab_config<1, TREAP_NODE_SIZE_CLASSES, 1>;
using treap_node_slab_t  = dynamic_slab<treap_node_config>;

// Global treap node allocator
inline treap_node_slab_t& get_treap_slab()
{
    alignas(treap_node_slab_t) static unsigned char storage[sizeof(treap_node_slab_t)];
    static treap_node_slab_t* ptr = ::new (storage) treap_node_slab_t();
    return *ptr;
}

} // namespace AL
