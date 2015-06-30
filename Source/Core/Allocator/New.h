#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Malloc.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE CORE_RETURN_NOT_NULL void *operator new(size_t size) { return Core::malloc(size); }
//----------------------------------------------------------------------------
FORCE_INLINE CORE_RETURN_MAYBE_NULL void *operator new(size_t size, std::nothrow_t const& ) throw()  { return Core::malloc(size); }
//----------------------------------------------------------------------------
FORCE_INLINE CORE_RETURN_NOT_NULL void *operator new[](size_t size)  { return Core::malloc(size); }
//----------------------------------------------------------------------------
FORCE_INLINE CORE_RETURN_MAYBE_NULL void *operator new[](size_t size,std::nothrow_t const& ) throw() { return Core::malloc(size); }
//----------------------------------------------------------------------------
FORCE_INLINE void operator delete(void* block) throw() { Core::free(block); }
FORCE_INLINE void operator delete(void* block, std::nothrow_t const& ) throw() { Core::free(block); }
//----------------------------------------------------------------------------
FORCE_INLINE void operator delete[](void* block) throw() { Core::free(block); }
FORCE_INLINE void operator delete[](void* block, std::nothrow_t const& ) throw() { Core::free(block); }
//----------------------------------------------------------------------------
FORCE_INLINE void operator delete(void*  block, size_t/* size */) throw() { Core::free(block); }
FORCE_INLINE void operator delete[](void* block, size_t/* size */) throw() { Core::free(block); }
//----------------------------------------------------------------------------
#if (defined(_MSC_VER) && (_MSC_VER != 1400))
FORCE_INLINE CORE_RETURN_NOT_NULL void *operator new(size_t size, int/* nBlockUse */, const char* /* szFileName */, int/* nLine */) { return Core::malloc(size); }
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
