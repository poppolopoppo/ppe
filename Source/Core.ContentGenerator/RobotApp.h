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
    virtual void Start() override;
    virtual void Draw(const FTimeline& time) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
