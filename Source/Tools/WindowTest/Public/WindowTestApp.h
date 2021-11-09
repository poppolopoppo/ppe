#pragma once

#include "Application/ApplicationWindow.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWindowTestApp : public Application::FApplicationWindow {
    typedef Application::FApplicationWindow parent_type;
public:
    explicit FWindowTestApp(const FModularDomain& domain);
    ~FWindowTestApp();

    virtual void Start() override;
    virtual void Shutdown() override;

private:
    bool Test_Draw1_(RHI::IFrameGraph& fg);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
