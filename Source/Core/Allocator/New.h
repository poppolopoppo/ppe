#pragma once

#include "Core/Core.h"

#if 0
namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL void *operator new(size_t size);
//----------------------------------------------------------------------------
CORE_RETURN_MAYBE_NULL void *operator new(size_t size, std::nothrow_t const& ) throw();
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL void *operator new[](size_t size);
//----------------------------------------------------------------------------
CORE_RETURN_MAYBE_NULL void *operator new[](size_t size,std::nothrow_t const& ) throw();
//----------------------------------------------------------------------------
void operator delete(void* block) throw();
void operator delete(void* block, std::nothrow_t const& ) throw();
//----------------------------------------------------------------------------
void operator delete[](void* block) throw();
void operator delete[](void* block, std::nothrow_t const& ) throw();
//----------------------------------------------------------------------------
void operator delete(void*  block, size_t/* size */) throw();
void operator delete[](void* block, size_t/* size */) throw();
//----------------------------------------------------------------------------
#if (defined(_MSC_VER) && (_MSC_VER != 1400))
CORE_RETURN_NOT_NULL void *operator new(size_t size, int/* nBlockUse */, const char* /* szFileName */, int/* nLine */);
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
#endif
