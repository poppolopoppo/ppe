#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Malloc.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE CORE_RETURN_NOT_NULL void* operator new(std::size_t size) CORE_THROW_SPECIFIER((std::bad_alloc)) { return Core::malloc(size); }
FORCE_INLINE void operator delete(void* ptr) CORE_THROW_SPECIFIER(()) { Core::free(ptr); }
//----------------------------------------------------------------------------
FORCE_INLINE CORE_RETURN_MAYBE_NULL void* operator new(std::size_t size, const std::nothrow_t& ) CORE_THROW_SPECIFIER(()) { return Core::malloc(size); }
FORCE_INLINE void operator delete(void* ptr,  const std::nothrow_t& ) CORE_THROW_SPECIFIER(()) { Core::free(ptr); }
//----------------------------------------------------------------------------
FORCE_INLINE CORE_RETURN_NOT_NULL void* operator new[](std::size_t size) CORE_THROW_SPECIFIER((std::bad_alloc)) { return Core::malloc(size); }
FORCE_INLINE void operator delete[](void* ptr) CORE_THROW_SPECIFIER(()) { Core::free(ptr); }
//----------------------------------------------------------------------------
FORCE_INLINE CORE_RETURN_MAYBE_NULL void* operator new[](std::size_t size, const std::nothrow_t& ) CORE_THROW_SPECIFIER(()) { return Core::malloc(size); }
FORCE_INLINE void operator delete[](void* ptr, const std::nothrow_t& ) CORE_THROW_SPECIFIER(()) { Core::free(ptr); }
//----------------------------------------------------------------------------
#if (defined(_MSC_VER) && (_MSC_VER != 1400))
FORCE_INLINE CORE_RETURN_NOT_NULL void *operator new(size_t size, int /*nBlockUse*/, const char* /*szFileName*/, int /*nLine*/) { return Core::malloc(size); }
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
