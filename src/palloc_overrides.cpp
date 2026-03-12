#include "palloc_global.h"
#include "platform.h"

#include <cstddef>
#include <cstdint>
#include <new>

// global new / delete overrides backed by Palloc.
//
// small allocations (user size + 16-byte header ≤ 4096) go through the global
// dynamic_slab. large allocations fall back to platform_mem (mmap/VirtualAlloc).
//
// a 16-byte header (alloc_meta) is prepended to every allocation so that the
// unsized operator delete can recover the allocation size and source.

namespace
{

struct alloc_meta
{
    void* raw_ptr;     // original allocation pointer (for aligned case, may differ from user - META_SIZE)
    size_t total_size; // total bytes allocated; high bit = mmap flag
};

static_assert(sizeof(alloc_meta) == 16, "alloc_meta must be 16 bytes for alignment");

constexpr size_t META_SIZE = sizeof(alloc_meta);
constexpr size_t MMAP_FLAG = size_t(1) << 63;
constexpr size_t MAX_SLAB_TOTAL = AL::SLAB_MAX_SIZE;

void* do_alloc(size_t size, size_t align = alignof(std::max_align_t))
{
    if (size == 0)
        size = 1;

    // add alignment padding for over-aligned requests
    size_t extra = META_SIZE;
    if (align > META_SIZE)
        extra += align;

    size_t total = size + extra;
    void* raw;
    bool is_mmap;

    if (total <= MAX_SLAB_TOTAL && align <= alignof(std::max_align_t))
    {
        raw = AL::get_global_slab().palloc(total);
        is_mmap = false;
    }
    else
    {
        raw = AL::platform_mem::alloc(total);
        is_mmap = true;
    }

    if (!raw)
        return nullptr;

    // find the aligned position for user data
    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t user_min = raw_addr + META_SIZE;
    uintptr_t user_aligned = (user_min + align - 1) & ~(align - 1);

    // store metadata just before the user pointer
    alloc_meta* meta = reinterpret_cast<alloc_meta*>(user_aligned - META_SIZE);
    meta->raw_ptr = raw;
    meta->total_size = total | (is_mmap ? MMAP_FLAG : 0);

    return reinterpret_cast<void*>(user_aligned);
}

void do_free(void* ptr) noexcept
{
    if (!ptr)
        return;

    alloc_meta* meta = reinterpret_cast<alloc_meta*>(static_cast<char*>(ptr) - META_SIZE);

    void* raw = meta->raw_ptr;
    size_t total = meta->total_size & ~MMAP_FLAG;

    if (meta->total_size & MMAP_FLAG)
        AL::platform_mem::free(raw, total);
    else
        AL::get_global_slab().free(raw, total);
}

} // anonymous namespace

void* operator new(std::size_t size)
{
    void* p = do_alloc(size);
    if (!p)
        throw std::bad_alloc();
    return p;
}

void* operator new[](std::size_t size)
{
    void* p = do_alloc(size);
    if (!p)
        throw std::bad_alloc();
    return p;
}

void* operator new(std::size_t size, std::align_val_t align)
{
    void* p = do_alloc(size, static_cast<size_t>(align));
    if (!p)
        throw std::bad_alloc();
    return p;
}

void* operator new[](std::size_t size, std::align_val_t align)
{
    void* p = do_alloc(size, static_cast<size_t>(align));
    if (!p)
        throw std::bad_alloc();
    return p;
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    return do_alloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    return do_alloc(size);
}

void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept
{
    return do_alloc(size, static_cast<size_t>(align));
}

void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept
{
    return do_alloc(size, static_cast<size_t>(align));
}

void operator delete(void* ptr) noexcept
{
    do_free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    do_free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept
{
    do_free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept
{
    do_free(ptr);
}

void operator delete(void* ptr, std::align_val_t) noexcept
{
    do_free(ptr);
}

void operator delete[](void* ptr, std::align_val_t) noexcept
{
    do_free(ptr);
}

void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept
{
    do_free(ptr);
}

void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept
{
    do_free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    do_free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    do_free(ptr);
}

void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
    do_free(ptr);
}

void operator delete[](void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
    do_free(ptr);
}
