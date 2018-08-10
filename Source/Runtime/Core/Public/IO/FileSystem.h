#pragma once

#include "Core.h"

#include "IO/FileSystemProperties.h"

#include "IO/Basename.h"
#include "IO/BasenameNoExt.h"
#include "IO/Dirname.h"
#include "IO/Dirpath.h"
#include "IO/Extname.h"
#include "IO/Filename.h"
#include "IO/MountingPoint.h"

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
