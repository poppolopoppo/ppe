#pragma once

#include "Core.h"

#include "IO/FileSystem_fwd.h"

namespace PPE {
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
    static const FExtname& Raw();
    static const FExtname& Z();

    static const FExtname& Bnz();
    static const FExtname& Json();
    static const FExtname& Otxt();
    static const FExtname& Xml();

    static const FExtname& Bkm();
    static const FExtname& Dae();
    static const FExtname& Fx();
    static const FExtname& Obj();
    static const FExtname& Ply();

    static const FExtname& Bmp();
    static const FExtname& Dds();
    static const FExtname& Gif();
    static const FExtname& Hdr();
    static const FExtname& Jpg();
    static const FExtname& Pgm();
    static const FExtname& Png();
    static const FExtname& Ppm();
    static const FExtname& Pic();
    static const FExtname& Psd();
    static const FExtname& Tga();

    // Mounting points :
    static const FMountingPoint& DataDir();
    static const FMountingPoint& ProcessDir();
    static const FMountingPoint& SavedDir();
    static const FMountingPoint& TmpDir();
    static const FMountingPoint& UserDir();
    static const FMountingPoint& VirtualDataDir();
    static const FMountingPoint& WorkingDir();

    static void Start();
    static void Shutdown();
};
//----------------------------------------------------------------------------
using FFS = FFSConstNames;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
