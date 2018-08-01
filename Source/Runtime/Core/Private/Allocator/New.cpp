#include "stdafx.h"

#include "Malloc.h"

#define PPE_OVERRIDE_NEW_OPERATORS 1 // turn to 0 to disable global allocator overriding %_NOCOMMIT%

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
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_RETURN_MAYBE_NULL void* PPE_DECLSPEC_CALL operator new(size_t size, std::nothrow_t const& ) noexcept  {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_RETURN_NOT_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size) {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
PPE_RETURN_MAYBE_NULL void* PPE_DECLSPEC_CALL operator new[](size_t size,std::nothrow_t const& ) noexcept {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void* block) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void* block, std::nothrow_t const& ) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block, std::nothrow_t const& ) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete(void*  block, size_t/* size */) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void PPE_DECLSPEC_CALL operator delete[](void* block, size_t/* size */) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#endif
