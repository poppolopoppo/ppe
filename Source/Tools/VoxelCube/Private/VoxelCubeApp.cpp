#include "stdafx.h"

#include "VoxelCubeApp.h"

#include "ApplicationModule.h"
#include "RHIModule.h"
#include "UI/Imgui.h"
#include "Window/WindowService.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "Maths/Threefy.h"

namespace PPE {
LOG_CATEGORY(, VoxelCube)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVoxelCubeApp::FVoxelCubeApp(FModularDomain& domain)
:   parent_type(domain, "Tools/VoxelCube", true) {

    FRHIModule& rhiModule = FRHIModule::Get(domain);
    rhiModule.SetStagingBufferSize(8_MiB);
}
//----------------------------------------------------------------------------
FVoxelCubeApp::~FVoxelCubeApp() = default;
//----------------------------------------------------------------------------
void FVoxelCubeApp::Start() {
    parent_type::Start();


    ApplicationLoop();
}
//----------------------------------------------------------------------------
void FVoxelCubeApp::Shutdown() {
    parent_type::Shutdown();
}
//----------------------------------------------------------------------------
void FVoxelCubeApp::Render(FTimespan dt) {
    parent_type::Render(dt);

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
