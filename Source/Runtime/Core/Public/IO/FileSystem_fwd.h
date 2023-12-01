#pragma once

#include "Core.h"

#include "IO/FileSystemToken_fwd.h"
#include "Memory/PtrRef.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBasename;
class FBasenameNoExt;
class FDirname;
class FDirpath;
class FExtname;
class FFilename;
class FMountingPoint;
//----------------------------------------------------------------------------
using FDirectory = FDirpath; // much friendlier, no?
//----------------------------------------------------------------------------
class FFileSystemNode;
using PFileSystemNode = TPtrRef<const FFileSystemNode>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
