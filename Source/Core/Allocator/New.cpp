#include "stdafx.h"

#include "Core/Allocator/New.h"
#include "Core/Allocator/Malloc.h"

#if 0
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL void *operator new(size_t size) {
    return Core::malloc(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_MAYBE_NULL void *operator new(size_t size, std::nothrow_t const& ) throw()  {
    return Core::malloc(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL void *operator new[](size_t size)  {
    return Core::malloc(size);
}
//----------------------------------------------------------------------------
CORE_RETURN_MAYBE_NULL void *operator new[](size_t size,std::nothrow_t const& ) throw() {
    return Core::malloc(size);
}
//----------------------------------------------------------------------------
void operator delete(void* block) throw() {
    Core::free(block);
}
//----------------------------------------------------------------------------
void operator delete(void* block, std::nothrow_t const& ) throw() {
    Core::free(block);
}
//----------------------------------------------------------------------------
void operator delete[](void* block) throw() {
    Core::free(block);
}
//----------------------------------------------------------------------------
void operator delete[](void* block, std::nothrow_t const& ) throw() {
    Core::free(block);
}
//----------------------------------------------------------------------------
void operator delete(void*  block, size_t/* size */) throw() {
    Core::free(block);
}
//----------------------------------------------------------------------------
void operator delete[](void* block, size_t/* size */) throw() {
    Core::free(block);
}
//----------------------------------------------------------------------------
#if (defined(_MSC_VER) && (_MSC_VER != 1400))
CORE_RETURN_NOT_NULL void *operator new(size_t size, int/* nBlockUse */, const char* /* szFileName */, int/* nLine */) {
    return Core::malloc(size);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif
