#include "stdafx.h"

#include "New.h"
#include "Malloc.h"

#if CORE_OVERRIDE_NEW_OPERATORS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL void *operator new(size_t size) {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_MAYBE_NULL void *operator new(size_t size, std::nothrow_t const& ) throw()  {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL void *operator new[](size_t size)  {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_MAYBE_NULL void *operator new[](size_t size,std::nothrow_t const& ) throw() {
    return (Core::malloc)(size);
}
//----------------------------------------------------------------------------
void operator delete(void* block) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void operator delete(void* block, std::nothrow_t const& ) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void operator delete[](void* block) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void operator delete[](void* block, std::nothrow_t const& ) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void operator delete(void*  block, size_t/* size */) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
void operator delete[](void* block, size_t/* size */) throw() {
    (Core::free)(block);
}
//----------------------------------------------------------------------------
#if (defined(_MSC_VER) && (_MSC_VER != 1400) && defined(_DEBUG))
_Check_return_ _Ret_notnull_ _Post_writable_byte_size_(_Size)
_VCRT_ALLOCATOR void* operator new(_In_ size_t size, _In_ int nBlockUse, _In_z_ const char* szFileName, _In_ int nLine) {
#if 1
    return (Core::malloc)(size);
#else
    return _malloc_dbg(size, nBlockUse, szFileName, nLine);
#endif
}
#endif
//----------------------------------------------------------------------------
#if (defined(_MSC_VER) && (_MSC_VER != 1400) && defined(_DEBUG))
_Check_return_ _Ret_notnull_ _Post_writable_byte_size_(_Size)
_VCRT_ALLOCATOR void* operator new[](_In_ size_t size, _In_ int nBlockUse, _In_z_ const char* szFileName, _In_ int nLine) {
#if 1
    return (Core::malloc)(size);
#else
    return _malloc_dbg(size, nBlockUse, szFileName, nLine);
#endif
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#endif
