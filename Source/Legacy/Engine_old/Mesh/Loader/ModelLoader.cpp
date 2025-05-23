﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ModelLoader.h"

#include "Core.Engine/Mesh/Model.h"
#include "Core.Engine/Mesh/ModelMesh.h"
#include "Core.Engine/Mesh/ModelMeshSubPart.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/FileSystemConstNames.h"
#include "Core/IO/VirtualFileSystem.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern bool LoadModel_Obj(PModel& pModel, const FFilename& filename, const ModelStream& meshStream);
//----------------------------------------------------------------------------
bool LoadModel(PModel& pModel, const FFilename& filename) {
    Assert(!filename.empty());

    LOG(Info, L"FModel: loading model from '{0}' ...", filename);

    if (!filename.HasExtension())
        AssertNotReached();

    ModelStream meshStream;
    if (!FVirtualFileSystem::Instance().ReadAll(filename, meshStream))
        return false;

    const FExtname& extname = filename.Basename().Extname();
    Assert(!extname.empty());

    if (FFileSystemConstNames::ObjExt() == extname) {
        return LoadModel_Obj(pModel, filename, meshStream);
    }
    else {
        AssertNotImplemented();
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
