#pragma once

#include "Core/Core.h"

#ifdef EXPORT_CORE_LOGIC
#   define CORE_LOGIC_API DLL_EXPORT
#else
#   define CORE_LOGIC_API DLL_IMPORT
#endif

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core.RTTI/RTTI_Tag.h"

// CppCon 2015: Vittorio Romeo “Implementation of a component-based entity system in modern C++”
// http://vittorioromeo.info/projects.html
// https://github.com/cppcon/cppcon2015
// https://www.youtube.com/watch?v=NTWSeQtHZ9M

namespace Core {
namespace Logic {
POOL_TAG_DECL(Logic);
RTTI_TAG_DECL(Logic);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_LOGIC_API LogicStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    LogicStartup()  { Start(); }
    ~LogicStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
