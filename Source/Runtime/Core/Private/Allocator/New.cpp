#include "stdafx.h"

#include "Allocator/Malloc.h"

#ifdef DYNAMIC_LINK
#   define PPE_OVERRIDE_NEW_OPERATORS 0 // #TODO very hard to override operator new/delete with dlls
#else
#   define PPE_OVERRIDE_NEW_OPERATORS 1 // turn to 0 to disable global allocator overriding %_NOCOMMIT%
#endif

#if PPE_OVERRIDE_NEW_OPERATORS

#   ifdef PLATFORM_WINDOWS
#      define PPE_RETURN_NOT_NULL     _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
#      define PPE_RETURN_MAYBE_NULL   _Ret_maybenull_ _Post_writable_byte_size_(size) _Success_(return != NULL) _VCRT_ALLOCATOR
#      define PPE_DECLSPEC_CALL       __CRTDECL
#   else
#      define PPE_RETURN_NOT_NULL
#      define PPE_RETURN_MAYBE_NULL
#      define PPE_DECLSPEC_CALL
#   endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new(size_t size) {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_RETURN_MAYBE_NULL void* PPE_DECLSPEC_CALL operator new(size_t size, std::nothrow_t const&) noexcept  {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size) {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_RETURN_MAYBE_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size, std::nothrow_t const&) noexcept {
    return (PPE::malloc)(size);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void* block) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void* block, std::nothrow_t const& ) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block, std::nothrow_t const& ) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void* block, size_t/* size */) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block, size_t/* size */) noexcept {
    (PPE::free)(block);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// C++17 adds new overloads for user-defined new/delete overloads
//----------------------------------------------------------------------------
#if _HAS_CXX17
//----------------------------------------------------------------------------
PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new(size_t size, std::align_val_t al) {
    return (PPE::aligned_malloc)(size, static_cast<size_t>(al));
}
//----------------------------------------------------------------------------
PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size, std::align_val_t al) {
    return (PPE::aligned_malloc)(size, static_cast<size_t>(al));
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void* block, std::align_val_t) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block, std::align_val_t) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void* block, std::size_t, std::align_val_t) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block, std::size_t, std::align_val_t) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void* block, std::align_val_t, std::nothrow_t const&) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block, std::align_val_t, std::nothrow_t const&) noexcept {
    (PPE::aligned_free)(block);
}
//----------------------------------------------------------------------------
#endif //!_HAS_CXX17
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#endif
