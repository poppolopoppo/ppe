#pragma once

#include "Core/Core.h"

#include "Core/IO/FileSystem_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FileSystemConstNames {
private:
    FileSystemConstNames();

public:
    // Extnames :
    static const Extname& DdsExt();
    static const Extname& FxExt();
    static const Extname& ObjExt();
    static const Extname& PlyExt();

    // Mounting points :
    static const MountingPoint& GameDataDir();
    static const MountingPoint& ProcessDir();
    static const MountingPoint& TmpDir();
    static const MountingPoint& VirtualDataDir();

    static void Start();
    static void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
