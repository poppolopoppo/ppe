#include "stdafx.h"

#include "Malloc.h"

#define CORE_OVERRIDE_NEW_OPERATORS 1

#ifdef PLATFORM_WINDOWS
#   define CORE_RETURN_NOT_NULL     _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
#   define CORE_RETURN_MAYBE_NULL   _Ret_maybenull_ _Post_writable_byte_size_(size) _Success_(return != NULL) _VCRT_ALLOCATOR
#   define CORE_DECLSPEC_CALL       __CRTDECL
#else
#   define CORE_RETURN_NOT_NULL
#   define CORE_RETURN_MAYBE_NULL
#   define CORE_DECLSPEC_CALL
#endif

#if CORE_OVERRIDE_NEW_OPERATORS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL void* CORE_DECLSPEC_CALL operator new(size_t size) {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_MAYBE_NULL void* CORE_DECLSPEC_CALL operator new(size_t size, std::nothrow_t const& ) noexcept  {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL void* CORE_DECLSPEC_CALL operator new[](size_t size) {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_MAYBE_NULL void* CORE_DECLSPEC_CALL operator new[](size_t size,std::nothrow_t const& ) noexcept {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
void CORE_DECLSPEC_CALL operator delete(void* block) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void CORE_DECLSPEC_CALL operator delete(void* block, std::nothrow_t const& ) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void CORE_DECLSPEC_CALL operator delete[](void* block) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void CORE_DECLSPEC_CALL operator delete[](void* block, std::nothrow_t const& ) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void CORE_DECLSPEC_CALL operator delete(void*  block, size_t/* size */) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void CORE_DECLSPEC_CALL operator delete[](void* block, size_t/* size */) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#endif
