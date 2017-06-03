#pragma once

#include "Core/Core.h"

#define CORE_OVERRIDE_NEW_OPERATORS 1

#ifdef CPP_VISUALSTUDIO
#   define CORE_RETURN_NOT_NULL     _Check_return_ _Ret_notnull_   _Post_writable_byte_size_(size)
#   define CORE_RETURN_MAYBE_NULL   _Check_return_ _Ret_maybenull_ _Post_writable_byte_size_(size) _Success_(return != NULL)

//  Annotations differs between vcruntime_new.h and vcruntime_new_debug.h
//  Incredible enough : vcruntime_new_debug.h has less checks than vcruntime_new.h
#   pragma warning( push )
#   pragma warning( disable : 28253 ) // warning C28253: Annotation incohérente pour 'XXX', only active with /analyze

#else
#   define CORE_RETURN_NOT_NULL
#   define CORE_RETURN_MAYBE_NULL
#endif

#if CORE_OVERRIDE_NEW_OPERATORS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL    void* CORE_API operator new(size_t size);
CORE_RETURN_MAYBE_NULL  void* CORE_API operator new(size_t size, std::nothrow_t const& ) throw();
//----------------------------------------------------------------------------
CORE_RETURN_NOT_NULL    void* CORE_API operator new[](size_t size);
CORE_RETURN_MAYBE_NULL  void* CORE_API operator new[](size_t size, std::nothrow_t const& ) throw();
//----------------------------------------------------------------------------
void CORE_API operator delete(void* block) throw();
void CORE_API operator delete(void* block, std::nothrow_t const& ) throw();
//----------------------------------------------------------------------------
void CORE_API operator delete[](void* block) throw();
void CORE_API operator delete[](void* block, std::nothrow_t const& ) throw();
//----------------------------------------------------------------------------
void CORE_API operator delete(void*  block, size_t/* size */) throw();
void CORE_API operator delete[](void* block, size_t/* size */) throw();
//----------------------------------------------------------------------------
#if (defined(_MSC_VER) && (_MSC_VER != 1400))
_Check_return_ _Ret_notnull_ _Post_writable_byte_size_(size)
void* CORE_API operator new(_In_ size_t size, _In_ int nBlockUse, _In_z_ const char* szFileName, _In_ int nLine);
_Check_return_ _Ret_notnull_ _Post_writable_byte_size_(size)
void* CORE_API operator new[](_In_ size_t size, _In_ int nBlockUse, _In_z_ const char* szFileName, _In_ int nLine);
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#endif

#ifdef CPP_VISUALSTUDIO
#   pragma warning( pop )
#endif
