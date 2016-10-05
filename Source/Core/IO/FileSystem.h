#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemProperties.h"

#include "Core/IO/FS/Basename.h"
#include "Core/IO/FS/BasenameNoExt.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Extname.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/FS/MountingPoint.h"

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
POOL_TAG_DECL(FileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFileSystemStartup {
public:
    static void Start();
    static void Shutdown();

    static void Clear();

    FFileSystemStartup() { Start(); }
    ~FFileSystemStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
