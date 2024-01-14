#pragma once

#include "Application/ApplicationWindow.h"

#include "Memory/UniquePtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FShaderToyApp : public Application::FApplicationWindow {
    typedef Application::FApplicationWindow parent_type;
public:
    explicit FShaderToyApp(FModularDomain& domain);
    ~FShaderToyApp() override;

    virtual void Start() override;
    virtual void Shutdown() override;

protected:
    virtual void Update(FTimespan dt) override;
    virtual void Render(FTimespan dt) override;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
