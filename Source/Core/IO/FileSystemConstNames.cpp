#include "stdafx.h"

#include "FileSystemConstNames.h"

#include "FileSystem.h"
#include "../Memory/AlignedStorage.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define FOREACH_FILESYSTEMCONSTNAMES_EXTNAME(_Macro) \
    _Macro(DdsExt,          L".dds") \
    _Macro(FxExt,           L".fx") \
    _Macro(ObjExt,          L".obj") \
    _Macro(PlyExt,          L".ply")
//----------------------------------------------------------------------------
#define FOREACH_FILESYSTEMCONSTNAMES_MOUNTINGPOINT(_Macro) \
    _Macro(GameDataDir,     L"GameData:") \
    _Macro(ProcessDir,      L"Process:") \
    _Macro(TmpDir,          L"Tmp:") \
    _Macro(VirtualDataDir,  L"VirtualData:")
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_FILESYSTEMCONSTNAMES_EXTNAME_STORAGE(_Name, _Content) \
    static POD_STORAGE(Extname) CONCAT(gPod_Extname_, _Name);
FOREACH_FILESYSTEMCONSTNAMES_EXTNAME(DEF_FILESYSTEMCONSTNAMES_EXTNAME_STORAGE)
#undef DEF_FILESYSTEMCONSTNAMES_EXTNAME_STORAGE
//----------------------------------------------------------------------------
#define DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_STORAGE(_Name, _Content) \
    static POD_STORAGE(MountingPoint) CONCAT(gPod_MountingPoint_, _Name);
FOREACH_FILESYSTEMCONSTNAMES_MOUNTINGPOINT(DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_STORAGE)
#undef DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_STORAGE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_FILESYSTEMCONSTNAMES_EXTNAME_ACCESSOR(_Name, _Content) \
    const Extname& FileSystemConstNames::_Name() { return *reinterpret_cast<const Extname *>(&CONCAT(gPod_Extname_, _Name)); }
FOREACH_FILESYSTEMCONSTNAMES_EXTNAME(DEF_FILESYSTEMCONSTNAMES_EXTNAME_ACCESSOR)
#undef DEF_FILESYSTEMCONSTNAMES_EXTNAME_ACCESSOR
//----------------------------------------------------------------------------
#define DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_ACCESSOR(_Name, _Content) \
    const MountingPoint& FileSystemConstNames::_Name() { return *reinterpret_cast<const MountingPoint *>(&CONCAT(gPod_MountingPoint_, _Name)); }
FOREACH_FILESYSTEMCONSTNAMES_MOUNTINGPOINT(DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_ACCESSOR)
#undef DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_ACCESSOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FileSystemConstNames::Start() {
#define DEF_FILESYSTEMCONSTNAMES_EXTNAME_STARTUP(_Name, _Content) \
    new ((void *)&CONCAT(gPod_Extname_, _Name)) Extname(_Content);
    FOREACH_FILESYSTEMCONSTNAMES_EXTNAME(DEF_FILESYSTEMCONSTNAMES_EXTNAME_STARTUP)
#undef DEF_FILESYSTEMCONSTNAMES_EXTNAME_STARTUP

#define DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_STARTUP(_Name, _Content) \
    new ((void *)&CONCAT(gPod_MountingPoint_, _Name)) MountingPoint(_Content);
    FOREACH_FILESYSTEMCONSTNAMES_MOUNTINGPOINT(DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_STARTUP)
#undef DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_STARTUP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FileSystemConstNames::Shutdown() {
#define DEF_FILESYSTEMCONSTNAMES_EXTNAME_SHUTDOWN(_Name, _Content) \
    reinterpret_cast<const Extname *>(&CONCAT(gPod_Extname_, _Name))->~Extname();
    FOREACH_FILESYSTEMCONSTNAMES_EXTNAME(DEF_FILESYSTEMCONSTNAMES_EXTNAME_SHUTDOWN)
#undef DEF_FILESYSTEMCONSTNAMES_EXTNAME_SHUTDOWN

#define DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_SHUTDOWN(_Name, _Content) \
    reinterpret_cast<const MountingPoint *>(&CONCAT(gPod_MountingPoint_, _Name))->~MountingPoint();
    FOREACH_FILESYSTEMCONSTNAMES_MOUNTINGPOINT(DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_SHUTDOWN)
#undef DEF_FILESYSTEMCONSTNAMES_MOUNTINGPOINT_SHUTDOWN
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
