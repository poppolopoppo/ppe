// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "Core/stdafx.h"

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/Heap.h"
#include "Core/Allocator/PoolAllocator.h"

#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniqueView.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Memory/WeakPtr.h"

#include "Core/Container/Token.h"
#include "Core/Container/Vector.h"

#include "Core/Diagnostic/CurrentProcess.h"
#include "Core/Diagnostic/Logger.h"

#include "Core/Thread/ThreadContext.h"
#include "Core/Thread/ThreadLocalStorage.h"
#include "Core/Thread/ThreadPool.h"

#include "Core.RTTI/RTTI.h"
