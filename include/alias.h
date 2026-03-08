#pragma once

#include "allocator.h"
#include <string>
#include <vector>

template<typename Allocator>
using basic_string_custom = std::basic_string<char, std::char_traits<char>, Allocator>;

using arena_string = basic_string_custom<AL::arena_allocator<char>>;
using slab_string = basic_string_custom<AL::slab_allocator<char>>;

template<typename T>
using slab_vector = std::vector<T, AL::slab_allocator<T>>;

constexpr size_t ONE_MB = (size_t)1024 * 1024;
