#pragma once

#include "dynamic_slab.h"
#include "platform.h"
#include <cstddef>
#include <new>
#include <type_traits>
#include <vector>

namespace AL
{

// Global dynamic_slab singleton. Uses the never-destroy pattern to avoid
// shutdown ordering issues with static destructors that deallocate.
inline default_dynamic_slab& get_global_slab()
{
    alignas(default_dynamic_slab) static unsigned char storage[sizeof(default_dynamic_slab)];
    static default_dynamic_slab* ptr = ::new (storage) default_dynamic_slab();
    return *ptr;
}

// Maximum allocation size that the slab can handle (largest size class).
inline constexpr size_t SLAB_MAX_SIZE = 4096;

// STL-compatible allocator backed by the global dynamic_slab.
// Default-constructible — no need to pass allocator instances to containers.
// Falls back to platform_mem (mmap) for allocations exceeding slab capacity.
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
            ptr = get_global_slab().palloc(bytes);
        else
            ptr = platform_mem::alloc(bytes);
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
            get_global_slab().free(p, bytes);
        else
            platform_mem::free(p, bytes);
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
