#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_LOGIC
#   define PPE_LOGIC_API DLL_EXPORT
#else
#   define PPE_LOGIC_API DLL_IMPORT
#endif

#include "Allocator/PoolAllocatorTag.h"
#include "RTTI_Tag.h"

// CppCon 2015: Vittorio Romeo
// Implementation of a component-based entity system in modern C++
// http://vittorioromeo.info/projects.html
// https://github.com/cppcon/cppcon2015
// https://www.youtube.com/watch?v=NTWSeQtHZ9M

namespace PPE {
namespace Logic {
POOL_TAG_DECL(Logic);
RTTI_TAG_DECL(Logic);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_LOGIC_API FLogicModule {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FLogicModule()  { Start(); }
    ~FLogicModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
