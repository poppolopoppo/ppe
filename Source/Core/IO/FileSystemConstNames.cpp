#include "stdafx.h"

#include "FileSystemConstNames.h"

#include "FileSystem.h"
#include "../Memory/AlignedStorage.h"

#define FOREACH_FILESYSTEMCONSTNAMES(_Macro) \
    _Macro(Dirname,         DotDot,         L"..") \
    \
    _Macro(Extname,         DaeExt,         L".dae") \
    _Macro(Extname,         FxExt,          L".fx") \
    _Macro(Extname,         ObjExt,         L".obj") \
    _Macro(Extname,         PlyExt,         L".ply") \
    \
    _Macro(Extname,         BmpExt,         L".bmp") \
    _Macro(Extname,         DdsExt,         L".dds") \
    _Macro(Extname,         GifExt,         L".gif") \
    _Macro(Extname,         HdrExt,         L".hdr") \
    _Macro(Extname,         JpgExt,         L".jpg") \
    _Macro(Extname,         PgmExt,         L".pgm") \
    _Macro(Extname,         PngExt,         L".png") \
    _Macro(Extname,         PpmExt,         L".ppm") \
    _Macro(Extname,         PicExt,         L".pic") \
    _Macro(Extname,         PsdExt,         L".psd") \
    _Macro(Extname,         TgaExt,         L".tga") \
    \
    _Macro(MountingPoint,   GameDataDir,    L"GameData:") \
    _Macro(MountingPoint,   ProcessDir,     L"Process:") \
    _Macro(MountingPoint,   TmpDir,         L"Tmp:") \
    _Macro(MountingPoint,   VirtualDataDir, L"VirtualData:")

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
    const _Type& FileSystemConstNames::_Name() { return *reinterpret_cast<const _Type *>(&CONCAT(gPod_##_Type##_, _Name)); }
FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_ACCESSOR)
#undef DEF_FILESYSTEMCONSTNAMES_ACCESSOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FileSystemConstNames::Start() {
#define DEF_FILESYSTEMCONSTNAMES_STARTUP(_Type, _Name, _Content) \
    new ((void *)&CONCAT(gPod_##_Type##_, _Name)) _Type(MakeStringSlice(_Content));
    FOREACH_FILESYSTEMCONSTNAMES(DEF_FILESYSTEMCONSTNAMES_STARTUP)
#undef DEF_FILESYSTEMCONSTNAMES_STARTUP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FileSystemConstNames::Shutdown() {
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
