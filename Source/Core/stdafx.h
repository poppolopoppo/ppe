// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#if defined(_MSC_VER) && defined(_DEBUG)
//  Finding memory leaks with CRT FHeap
//  https://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
#   define _CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
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

#include <unordered_map>
#include <type_traits>

#include "Core/Core.h"
