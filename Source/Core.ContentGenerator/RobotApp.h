#pragma once

#include "Core.Application/ApplicationGraphics.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class RobotApp : public Application::ApplicationGraphics {
    typedef Application::ApplicationGraphics parent_type;
public:
    RobotApp();
    virtual void Start() override;
    virtual void Draw(const Timeline& time) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
