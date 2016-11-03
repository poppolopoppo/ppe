#pragma once

#include "Core/Memory/HashFunctions.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Understanding the Linux Kernel
// https://books.google.fr/books?id=h0lltXyJ8aIC&lpg=PT109&ots=gO2uM_c7FQ&dq=The%20Magic%20Constant%20linux%20hash&hl=fr&pg=PT109#v=onepage&q=The%20Magic%20Constant%20linux%20hash&f=false
//----------------------------------------------------------------------------
/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001ULL
//----------------------------------------------------------------------------
#ifdef ARCH_X64
#   define CORE_HASH_VALUE_SEED (0xdeadbeefabadcafeULL)
#   define CORE_HASH_VALUE_32_TO_64_IFP(_VALUE) (size_t(_VALUE)*GOLDEN_RATIO_PRIME_64)
#   define GOLDEN_RATIO_PRIME_SIZE_T GOLDEN_RATIO_PRIME_64
#else
#   define CORE_HASH_VALUE_SEED (0xdeadbeefUL)
#   define CORE_HASH_VALUE_32_TO_64_IFP(_VALUE) (size_t(_VALUE)*GOLDEN_RATIO_PRIME_32)
#   define GOLDEN_RATIO_PRIME_SIZE_T GOLDEN_RATIO_PRIME_32
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint8_t), size_t>::type hash_as_pod_impl_(T&& value) {
    size_t a = *reinterpret_cast<const uint8_t *>(&value);
    return CORE_HASH_VALUE_32_TO_64_IFP(a^0x55+(a>>4));
}
template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), size_t>::type hash_as_pod_impl_(T&& value) {
    size_t a = *reinterpret_cast<const uint16_t *>(&value);
    a = (a^CORE_HASH_VALUE_SEED) + (a<<4);
    a = a ^ (a>>10);
    a = a + (a<<7);
    a = a ^ (a>>13);
    return CORE_HASH_VALUE_32_TO_64_IFP(a);
}
template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), size_t>::type hash_as_pod_impl_(T&& value) {
    // http://burtleburtle.net/bob/hash/integer.html
    size_t a = *reinterpret_cast<const uint32_t *>(&value);
    a = (a^CORE_HASH_VALUE_SEED) + (a<<4);
    a = a ^ (a>>10);
    a = a + (a>>24);
    a = a + (a<<7);
    a = a ^ (a>>13);
    return CORE_HASH_VALUE_32_TO_64_IFP(a);
}
template <typename T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), size_t>::type hash_as_pod_impl_(T&& value) {
    // http://www.concentric.net/~ttwang/tech/inthash.htm
    uint64_t a = *reinterpret_cast<const uint64_t *>(&value);
    a = a + (a>>32)^0x48655121UL;
    a = (a^CORE_HASH_VALUE_SEED) + (a<<4);
    a = a ^ (a>>10);
    a = a + (a>>24);
    a = a + (a<<7);
    a = a ^ (a>>13);
    return CORE_HASH_VALUE_32_TO_64_IFP(a);
}
template <typename T>
typename std::enable_if<sizeof(T) == sizeof(u128), size_t>::type hash_as_pod_impl_(T&& value) {
    // http://www.concentric.net/~ttwang/tech/inthash.htm
    u128 x = *reinterpret_cast<const u128 *>(&value);
    const uint64_t kMul = 0x9ddfea08eb382d69ULL;
    uint64_t a = (x.lo ^ x.hi) * kMul;
    a ^= (a >> 47);
    uint64_t b = (x.hi ^ a) * kMul;
    b ^= (b >> 44);
    b *= kMul;
    b ^= (b >> 41);
    b *= kMul;
    return size_t(b);
}
template <typename T>
typename std::enable_if<
    sizeof(T) != sizeof(u8) &&
    sizeof(T) != sizeof(u16) &&
    sizeof(T) != sizeof(u32) &&
    sizeof(T) != sizeof(u64) &&
    sizeof(T) != sizeof(u128),
    size_t
>::type hash_as_pod_impl_(T&& pod) {
    return hash_mem((const void *)&pod, sizeof(T));
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE size_t hash_as_pod(T&& pod) {
    return details::hash_as_pod_impl_(std::forward<T>(pod));
}
//----------------------------------------------------------------------------
inline size_t hash_ptr(const void* ptr) {
    return details::hash_as_pod_impl_(intptr_t(ptr));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE size_t hash_as_pod_array(const T *ptr, size_t count) {
    return hash_mem(ptr, sizeof(T) * count);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE size_t hash_as_pod_array(const T (&staticArray)[_Dim]) {
    return hash_mem(&staticArray[0], sizeof(T) * _Dim);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _It, typename _Tag>
size_t hash_as_pod_range_impl_(_It&& first, _It&& last, _Tag ) {
    size_t h = CORE_HASH_VALUE_SEED;
    for (; first != last; ++first)
        h = hash_mem(*first, sizeof(*first), h);
    return h;
}
template <typename _It>
size_t hash_as_pod_range_impl_(_It&& first, _It&& last, std::random_access_iterator_tag ) {
    return hash_as_pod_array(&(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename _It>
FORCE_INLINE size_t hash_as_pod_range(_It&& first, _It&& last) {
    typedef typename std::iterator_traits<_It>::iterator_category category;
    return details::hash_as_pod_range_impl_(std::forward<_It>(first), std::forward<_It>(last), category());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}
