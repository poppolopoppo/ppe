#pragma once

#include "Core.Application/ApplicationGraphics.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRobotApp : public Application::FApplicationGraphics {
    typedef Application::FApplicationGraphics parent_type;
public:
    FRobotApp();
    ~FRobotApp();

    virtual void Start() override;
    virtual void Shutdown() override;

    virtual void Draw(const FTimeline& time) override;

private:
    TUniquePtr<class FRobotAppImpl> _pimpl;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
