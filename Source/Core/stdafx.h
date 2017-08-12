// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WITH_CORE_CRTDBG_MAP_ALLOC 0

#if defined(_MSC_VER) && defined(_DEBUG) && WITH_CORE_CRTDBG_MAP_ALLOC
//  Finding memory leaks with CRT FHeap
//  https://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
#   define _CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#endif

#if defined(_MSC_VER)
//  Include this asap to avoid M$ macro substitutions bullshit
#   include "Misc/Platform_Windows.h"
#endif

#include <stdio.h>
#include <tchar.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <exception>
#include <limits>
#include <mutex>
#include <utility>
#include <type_traits>

#include <iomanip>
#include <iosfwd>
#include <iostream>

#include <locale>
#include <string>

#include <type_traits>

#include "Core/Core.h"
