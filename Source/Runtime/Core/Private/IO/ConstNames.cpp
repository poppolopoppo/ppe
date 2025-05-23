﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/ConstNames.h"

#include "IO/FileSystem.h"
#include "Meta/AlignedStorage.h"

#define FOREACH_FILESYSTEMCONSTNAMES(_Macro) \
    _Macro(FDirname,         DotDot,         L"..") \
    \
    _Macro(FExtname,         Raw,            L".raw") \
    _Macro(FExtname,         Z,              L".z") \
    \
    _Macro(FExtname,         Bin,            L".bin") \
    _Macro(FExtname,         Bnx,            L".bnx") \
    \
    _Macro(FExtname,         Csv,            L".csv") \
    _Macro(FExtname,         Json,           L".json") \
    _Macro(FExtname,         Txt,            L".txt") \
    _Macro(FExtname,         Xml,            L".xml") \
    \
    _Macro(FExtname,         Bkm,            L".bkm") \
    _Macro(FExtname,         Dae,            L".dae") \
    _Macro(FExtname,         Fx,             L".fx") \
    _Macro(FExtname,         Obj,            L".obj") \
    _Macro(FExtname,         Ply,            L".ply") \
    \
    _Macro(FExtname,         Bmp,            L".bmp") \
    _Macro(FExtname,         Dds,            L".dds") \
    _Macro(FExtname,         Gif,            L".gif") \
    _Macro(FExtname,         Hdr,            L".hdr") \
    _Macro(FExtname,         Jpg,            L".jpg") \
    _Macro(FExtname,         Pgm,            L".pgm") \
    _Macro(FExtname,         Png,            L".png") \
    _Macro(FExtname,         Ppm,            L".ppm") \
    _Macro(FExtname,         Pic,            L".pic") \
    _Macro(FExtname,         Psd,            L".psd") \
    _Macro(FExtname,         Tga,            L".tga") \
    \
    _Macro(FMountingPoint,   DataDir,       L"Data:") \
    _Macro(FMountingPoint,   ProcessDir,    L"Process:") \
    _Macro(FMountingPoint,   SavedDir,      L"Saved:") \
    _Macro(FMountingPoint,   TmpDir,        L"Tmp:") \
    _Macro(FMountingPoint,   UserDir,       L"User:") \
    _Macro(FMountingPoint,   VirtualDataDir,L"VirtualData:") \
    _Macro(FMountingPoint,   WorkingDir,    L"Working:")

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_FILESYSTEMCONSTNAMES_STORAGE(_Type, _Name, _Content) \
    static POD_STORAGE(_Type) CONCAT(gPod_##_Type##_, _Name);
FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_STORAGE)
#undef DEF_FILESYSTEMCONSTNAMES_STORAGE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_FILESYSTEMCONSTNAMES_ACCESSOR(_Type, _Name, _Content) \
    const _Type& FFSConstNames::_Name() { return *reinterpret_cast<const _Type *>(&CONCAT(gPod_##_Type##_, _Name)); }
FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_ACCESSOR)
#undef DEF_FILESYSTEMCONSTNAMES_ACCESSOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FFSConstNames::Start() {
#define DEF_FILESYSTEMCONSTNAMES_STARTUP(_Type, _Name, _Content) \
    INPLACE_NEW(&CONCAT(gPod_##_Type##_, _Name), _Type)(MakeStringView(_Content));
    FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_STARTUP)
#undef DEF_FILESYSTEMCONSTNAMES_STARTUP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FFSConstNames::Shutdown() {
#define DEF_FILESYSTEMCONSTNAMES_SHUTDOWN(_Type, _Name, _Content) \
    reinterpret_cast<const _Type *>(&CONCAT(gPod_##_Type##_, _Name))->~_Type();
    FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_SHUTDOWN)
#undef DEF_FILESYSTEMCONSTNAMES_SHUTDOWN
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#undef FOREACH_FILESYSTEMCONSTNAMES
