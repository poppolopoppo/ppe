// stdafx.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets qui sont utilisés fréquemment,
// et sont rarement modifiés
//

#pragma once

#include "targetver.h"

#include "Graphics.h"

#include "Core/stdafx.h"

#include "Core/Core.h"

#include "Core/Allocation.h"
#include "Core/MemoryView.h"
#include "Core/MemoryStack.h"
#include "Core/Heap.h"

#include "Core/String.h"
#include "Core/StringSlice.h"
#include "Core/Format.h"
#include "Core/Stream.h"

#include "Core/Units.h"

#include "Core/ThreadContext.h"
#include "Core/ThreadLocalStorage.h"

#include "Core/UniqueView.h"
#include "Core/UniquePtr.h"
#include "Core/RefPtr.h"

#include "Core/PoolAllocator.h"
#include "Core/PoolAllocator-impl.h"

#include "Core/ThreadPool.h"

#include "Core/CurrentProcess.h"

#include "Core/Token.h"

#include "Core/FileSystem.h"
#include "Core/VirtualFileSystem_fwd.h"

#include "Core/MetaClassName.h"
#include "Core/MetaObject.h"
#include "Core/MetaTransaction.h"
#include "Core/MetaType.h"
#include "Core/MetaTypeTraits.h"

#include "Core/RTTI.h"

#include "Core/Tuple.h"

#include "Core/MathHelpers.h"
#include "Core/ScalarVector_fwd.h"
#include "Core/ScalarMatrix_fwd.h"
#include "Core/ScalarBoundingBox_fwd.h"
