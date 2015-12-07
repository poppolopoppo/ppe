#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"

// CppCon 2015: Vittorio Romeo “Implementation of a component-based entity system in modern C++”
// http://vittorioromeo.info/projects.html
// https://github.com/cppcon/cppcon2015
// https://www.youtube.com/watch?v=NTWSeQtHZ9M

namespace Core {
namespace Logic {
POOLTAG_DECL(Logic);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LogicStartup {
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
