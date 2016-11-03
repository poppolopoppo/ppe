#pragma once

#include "Core/Core.h"

#include "Core/IO/FileSystem_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFSConstNames {
private:
    FFSConstNames();

public:
    // Tokens :
    static const FDirname& DotDot();

    // Extnames :
    static const FExtname& DaeExt();
    static const FExtname& FxExt();
    static const FExtname& ObjExt();
    static const FExtname& PlyExt();

    static const FExtname& BmpExt();
    static const FExtname& DdsExt();
    static const FExtname& GifExt();
    static const FExtname& HdrExt();
    static const FExtname& JpgExt();
    static const FExtname& PgmExt();
    static const FExtname& PngExt();
    static const FExtname& PpmExt();
    static const FExtname& PicExt();
    static const FExtname& PsdExt();
    static const FExtname& TgaExt();

    // Mounting points :
    static const FMountingPoint& GameDataDir();
    static const FMountingPoint& ProcessDir();
    static const FMountingPoint& TmpDir();
    static const FMountingPoint& VirtualDataDir();

    static void Start();
    static void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
