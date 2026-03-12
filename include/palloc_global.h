#pragma once

#include "dynamic_slab.h"
#include "platform.h"
#include <cstddef>
#include <new>
#include <type_traits>
#include <vector>

namespace AL
{

inline constexpr std::array<size_class, 10> EDITOR_SIZE_CLASSES = {
    {
     {8, 512, 64},
     {16, 512, 64},
     {32, 4096, 128}, // piece and small strings
        {64, 1024, 64},
     {128, 4096, 128}, // treap nodes → primary hot path
        {256, 512, 32},
     {512, 256, 16},
     {1024, 128, 8},
     {2048, 64, 8},
     {4096, 32, 4},
     }
};

// MiniEditor does not yet need locks since it is single threaded
struct null_mutex
{
    void lock() noexcept
    {}
    void unlock() noexcept
    {}
    bool try_lock() noexcept
    {
        return true;
    }
};

using editor_slab_config = slab_config<10, EDITOR_SIZE_CLASSES, 10>;
using editor_dynamic_slab = dynamic_slab<editor_slab_config>;

// Maximum size the slab handles (largest size class in EDITOR_SIZE_CLASSES).
inline constexpr size_t SLAB_MAX_SIZE = 4096;

// Global dynamic_slab singleton. Uses the never-destroy pattern to avoid
// shutdown ordering issues with static destructors that deallocate.
inline editor_dynamic_slab& get_global_slab()
{
    alignas(editor_dynamic_slab) static unsigned char storage[sizeof(editor_dynamic_slab)];
    static editor_dynamic_slab* ptr = ::new (storage) editor_dynamic_slab();
    return *ptr;
}

// STL-compatible allocator backed by the global dynamic_slab.
// falls back to platform_mem (mmap) for allocations exceeding slab capacity.
template<typename T>
struct palloc_allocator
{
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::true_type;

    constexpr palloc_allocator() noexcept = default;

    template<typename U>
    constexpr palloc_allocator(const palloc_allocator<U>&) noexcept
    {}

    [[nodiscard]] T* allocate(size_type n)
    {
        if (n == 0)
            return nullptr;

        const size_t bytes = n * sizeof(T);
        void* ptr;

        if (bytes <= SLAB_MAX_SIZE)
        {
            ptr = AL::get_global_slab().palloc(bytes);
        }
        else
        {
            ptr = AL::platform_mem::alloc(bytes);
        }

        if (!ptr)
            throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, size_type n) noexcept
    {
        if (!p || n == 0)
            return;

        const size_t bytes = n * sizeof(T);

        if (bytes <= SLAB_MAX_SIZE)
        {
            AL::get_global_slab().free(p, bytes);
        }
        else
            AL::platform_mem::free(p, bytes);
    }

    template<typename U>
    bool operator==(const palloc_allocator<U>&) const noexcept
    {
        return true;
    }
};

template<typename T>
using palloc_vector = std::vector<T, palloc_allocator<T>>;

} // namespace AL
