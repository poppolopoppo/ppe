// stdafx.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets qui sont utilisés fréquemment,
// et sont rarement modifiés
//

#pragma once

#include "targetver.h"

#include "Core/stdafx.h"

#include "Core/Core.h"
#include "Core/Core_extern.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/Heap.h"
#include "Core/Allocator/PoolAllocator.h"

#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniqueView.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Memory/WeakPtr.h"

#include "Core/IO/FileSystem_fwd.h"
#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/IO/String.h"
#include "Core/IO/StringSlice.h"
#include "Core/IO/VirtualFileSystem_fwd.h"

#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarBoundingBox_fwd.h"
#include "Core/Maths/ScalarMatrix_fwd.h"
#include "Core/Maths/Units.h"

#include "Core/Container/Token.h"
#include "Core/Container/Vector.h"

#include "Core/Diagnostic/Logger.h"

#include "Core/Thread/ThreadContext.h"

#include "Core.Graphics/Graphics.h"
