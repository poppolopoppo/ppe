#pragma once

#include "Core.h"

#include "IO/FS/FileSystemProperties.h"

#include "IO/FS/Basename.h"
#include "IO/FS/BasenameNoExt.h"
#include "IO/FS/Dirname.h"
#include "IO/FS/Dirpath.h"
#include "IO/FS/Extname.h"
#include "IO/FS/Filename.h"
#include "IO/FS/MountingPoint.h"

#include "Allocator/PoolAllocatorTag.h"

namespace PPE {
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
} //!namespace PPE
