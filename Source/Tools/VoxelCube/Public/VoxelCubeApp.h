#pragma once

#include "Application/ApplicationWindow.h"

#include "Memory/UniquePtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVoxelCubeApp : public Application::FApplicationWindow {
    typedef Application::FApplicationWindow parent_type;
public:
    explicit FVoxelCubeApp(FModularDomain& domain);
    ~FVoxelCubeApp() override;

    virtual void Start() override;
    virtual void Shutdown() override;

protected:
    virtual void Render(FTimespan dt) override;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
