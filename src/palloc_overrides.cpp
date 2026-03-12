#include "palloc_global.h"
#include "platform.h"

#include <cstddef>
#include <cstdint>
#include <new>

// Global operator new / operator delete overrides backed by Palloc.
//
// Allocates directly from the global dynamic_slab with no header
// Falls back to syscalls for large allocations with a 16-byte header

namespace AL
{

struct alloc_meta
{
    void* raw_ptr;     // original mmap pointer
    size_t total_size; // total mmap bytes plus any padding
};

static_assert(sizeof(alloc_meta) == 16, "alloc_meta must be 16 bytes");

constexpr size_t META_SIZE = sizeof(alloc_meta);

inline bool use_slab(size_t size, size_t align) noexcept
{
    return size <= AL::SLAB_MAX_SIZE && align <= alignof(std::max_align_t);
}

void* do_alloc(size_t size, size_t align = alignof(std::max_align_t))
{
    if (size == 0)
        size = 1;

    if (use_slab(size, align))
        return AL::get_global_slab().palloc(size); // no header

    // mmap path
    size_t extra = META_SIZE;
    if (align > META_SIZE)
        extra += align;

    size_t total = size + extra;
    void* raw = AL::platform_mem::alloc(total);
    if (!raw)
        return nullptr;

    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t user_min = raw_addr + META_SIZE;
    uintptr_t user_aligned = (user_min + align - 1) & ~(align - 1);

    alloc_meta* meta = reinterpret_cast<alloc_meta*>(user_aligned - META_SIZE);
    meta->raw_ptr = raw;
    meta->total_size = total;

    return reinterpret_cast<void*>(user_aligned);
}

void do_free_mmap(void* ptr) noexcept
{
    alloc_meta* meta = reinterpret_cast<alloc_meta*>(static_cast<char*>(ptr) - META_SIZE);
    AL::platform_mem::free(meta->raw_ptr, meta->total_size);
}

void do_free_unsized(void* ptr) noexcept
{
    if (!ptr)
        return;
    if (AL::get_global_slab().free_unsized(ptr))
        return;
    do_free_mmap(ptr);
}

void do_free_sized(void* ptr, size_t size, size_t align = alignof(std::max_align_t)) noexcept
{
    if (!ptr)
        return;
    if (use_slab(size, align))
        AL::get_global_slab().free(ptr, size);
    else
        do_free_mmap(ptr);
}

} // namespace AL

void* operator new(std::size_t size)
{
    void* p = AL::do_alloc(size);
    if (!p)
        throw std::bad_alloc();
    return p;
}

void* operator new[](std::size_t size)
{
    void* p = AL::do_alloc(size);
    if (!p)
        throw std::bad_alloc();
    return p;
}

void* operator new(std::size_t size, std::align_val_t align)
{
    void* p = AL::do_alloc(size, static_cast<size_t>(align));
    if (!p)
        throw std::bad_alloc();
    return p;
}

void* operator new[](std::size_t size, std::align_val_t align)
{
    void* p = AL::do_alloc(size, static_cast<size_t>(align));
    if (!p)
        throw std::bad_alloc();
    return p;
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    return AL::do_alloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    return AL::do_alloc(size);
}

void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept
{
    return AL::do_alloc(size, static_cast<size_t>(align));
}

void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept
{
    return AL::do_alloc(size, static_cast<size_t>(align));
}

void operator delete(void* ptr) noexcept
{
    AL::do_free_unsized(ptr);
}

void operator delete[](void* ptr) noexcept
{
    AL::do_free_unsized(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    AL::do_free_unsized(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    AL::do_free_unsized(ptr);
}

void operator delete(void* ptr, std::align_val_t) noexcept
{
    AL::do_free_unsized(ptr);
}

void operator delete[](void* ptr, std::align_val_t) noexcept
{
    AL::do_free_unsized(ptr);
}

void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
    AL::do_free_unsized(ptr);
}

void operator delete[](void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
    AL::do_free_unsized(ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept
{
    AL::do_free_sized(ptr, size);
}

void operator delete[](void* ptr, std::size_t size) noexcept
{
    AL::do_free_sized(ptr, size);
}

void operator delete(void* ptr, std::size_t size, std::align_val_t align) noexcept
{
    AL::do_free_sized(ptr, size, static_cast<size_t>(align));
}

void operator delete[](void* ptr, std::size_t size, std::align_val_t align) noexcept
{
    AL::do_free_sized(ptr, size, static_cast<size_t>(align));
}
