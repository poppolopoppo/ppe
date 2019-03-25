#pragma once

#include "Allocator/New.h"

#if PPE_OVERRIDE_NEW_OPERATORS

#   include "Allocator/Malloc.h"

#   ifdef PLATFORM_WINDOWS
#      define PPE_NEW_API
#      define PPE_CHECK_RETURN_NOT_NULL _Check_return_ _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
#      define PPE_RETURN_NOT_NULL       _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
#      define PPE_RETURN_MAYBE_NULL     _Ret_maybenull_ _Post_writable_byte_size_(size) _Success_(return != NULL) _VCRT_ALLOCATOR
#      define PPE_DECLSPEC_CALL         __CRTDECL
#   else
#      define PPE_NEW_API               PPE_CORE_API
#      define PPE_CHECK_RETURN_NOT_NULL
#      define PPE_RETURN_NOT_NULL
#      define PPE_RETURN_MAYBE_NULL
#      define PPE_DECLSPEC_CALL
#   endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_NEW_API PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new(size_t size) {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_NEW_API PPE_RETURN_MAYBE_NULL void* PPE_DECLSPEC_CALL operator new(size_t size, std::nothrow_t const&) noexcept {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_NEW_API PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size) {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_NEW_API PPE_RETURN_MAYBE_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size, std::nothrow_t const&) noexcept {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete(void* block) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete(void* block, std::nothrow_t const&) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete[](void* block) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete[](void* block, std::nothrow_t const&) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete(void* block, size_t/* size */) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete[](void* block, size_t/* size */) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Special overloads frequently used for augmenting allocations in debug builds
#if defined(_DEBUG) or defined(_CRTDBG_MAP_ALLOC)
//----------------------------------------------------------------------------
PPE_NEW_API PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new(size_t size, const char* /* fname */, size_t /* line */) {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_NEW_API PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size, const char* /* fname */, size_t /* line */) {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_NEW_API PPE_CHECK_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new(
    _In_   size_t      size,
    _In_   int         /*blockUse*/,
    _In_z_ char const* /*fileName*/,
    _In_   int         /*lineNumber*/ ) {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_NEW_API PPE_CHECK_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new[](
    _In_   size_t      size,
    _In_   int         /*blockUse*/,
    _In_z_ char const* /*fileName*/,
    _In_   int         /*lineNumber*/ ) {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete(
    void*       block,
    int         /*blockUse*/,
    char const* /*fileName*/,
    int         /*lineNumber*/) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete[](
    void*       block,
    int         /*blockUse*/,
    char const* /*fileName*/,
    int         /*lineNumber*/) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
#endif //!defined(_DEBUG) or defined(_CRTDBG_MAP_ALLOC)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// C++17 adds new overloads for user-defined new/delete overloads
#if _HAS_CXX17
//----------------------------------------------------------------------------
PPE_NEW_API PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new(size_t size, std::align_val_t al) {
    return (PPE::aligned_malloc)(size, static_cast<size_t>(al));
}
//----------------------------------------------------------------------------
PPE_NEW_API PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size, std::align_val_t al) {
    return (PPE::aligned_malloc)(size, static_cast<size_t>(al));
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete(void* block, std::align_val_t) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete[](void* block, std::align_val_t) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete(void* block, std::size_t, std::align_val_t) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete[](void* block, std::size_t, std::align_val_t) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete(void* block, std::align_val_t, std::nothrow_t const&) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
PPE_NEW_API void PPE_DECLSPEC_CALL operator delete[](void* block, std::align_val_t, std::nothrow_t const&) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
#endif //!_HAS_CXX17
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

#endif //!PPE_OVERRIDE_NEW_OPERATORS
