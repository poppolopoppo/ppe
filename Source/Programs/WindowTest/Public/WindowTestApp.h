#pragma once

#include "Application/ApplicationWindow.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWindowTestApp : public Application::FApplicationWindow {
    typedef Application::FApplicationWindow parent_type;
public:
    explicit FWindowTestApp(FModularDomain& domain);
    ~FWindowTestApp() override;

    virtual void Start() override;
    virtual void Run() override;
    virtual void Shutdown() override;

protected:
    virtual void Render(RHI::IFrameGraph& fg, FTimespan dt) override;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
