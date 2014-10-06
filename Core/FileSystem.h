#pragma once

#include "Core.h"

#include "FileSystemProperties.h"

#include "BasenameNoExt.h"
#include "Basename.h"
#include "Dirname.h"
#include "Dirpath.h"
#include "Extname.h"
#include "Filename.h"
#include "MountingPoint.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FileSystemStartup {
public:
    static void Start();
    static void Shutdown();

    static void Clear();

    FileSystemStartup() { Start(); }
    ~FileSystemStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
