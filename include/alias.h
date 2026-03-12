#pragma once

#include "palloc_global.h"

// Convenience aliases in global namespace
template<typename T>
using palloc_vector = AL::palloc_vector<T>;

constexpr size_t ONE_MB = (size_t)1024 * 1024;
