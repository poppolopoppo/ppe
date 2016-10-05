#include "stdafx.h"

#include "FileSystemConstNames.h"

#include "FileSystem.h"
#include "../Memory/AlignedStorage.h"

#define FOREACH_FILESYSTEMCONSTNAMES(_Macro) \
    _Macro(FDirname,         DotDot,         L"..") \
    \
    _Macro(FExtname,         DaeExt,         L".dae") \
    _Macro(FExtname,         FxExt,          L".fx") \
    _Macro(FExtname,         ObjExt,         L".obj") \
    _Macro(FExtname,         PlyExt,         L".ply") \
    \
    _Macro(FExtname,         BmpExt,         L".bmp") \
    _Macro(FExtname,         DdsExt,         L".dds") \
    _Macro(FExtname,         GifExt,         L".gif") \
    _Macro(FExtname,         HdrExt,         L".hdr") \
    _Macro(FExtname,         JpgExt,         L".jpg") \
    _Macro(FExtname,         PgmExt,         L".pgm") \
    _Macro(FExtname,         PngExt,         L".png") \
    _Macro(FExtname,         PpmExt,         L".ppm") \
    _Macro(FExtname,         PicExt,         L".pic") \
    _Macro(FExtname,         PsdExt,         L".psd") \
    _Macro(FExtname,         TgaExt,         L".tga") \
    \
    _Macro(FMountingPoint,   GameDataDir,    L"GameData:") \
    _Macro(FMountingPoint,   ProcessDir,     L"Process:") \
    _Macro(FMountingPoint,   TmpDir,         L"Tmp:") \
    _Macro(FMountingPoint,   VirtualDataDir, L"VirtualData:")

namespace Core {
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
    const _Type& FFileSystemConstNames::_Name() { return *reinterpret_cast<const _Type *>(&CONCAT(gPod_##_Type##_, _Name)); }
FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_ACCESSOR)
#undef DEF_FILESYSTEMCONSTNAMES_ACCESSOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FFileSystemConstNames::Start() {
#define DEF_FILESYSTEMCONSTNAMES_STARTUP(_Type, _Name, _Content) \
    new ((void *)&CONCAT(gPod_##_Type##_, _Name)) _Type(MakeStringView(_Content));
    FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_STARTUP)
#undef DEF_FILESYSTEMCONSTNAMES_STARTUP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FFileSystemConstNames::Shutdown() {
#define DEF_FILESYSTEMCONSTNAMES_SHUTDOWN(_Type, _Name, _Content) \
    reinterpret_cast<const _Type *>(&CONCAT(gPod_##_Type##_, _Name))->~_Type();
    FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_SHUTDOWN)
#undef DEF_FILESYSTEMCONSTNAMES_SHUTDOWN
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#undef FOREACH_FILESYSTEMCONSTNAMES
