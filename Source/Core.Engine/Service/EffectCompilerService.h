#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/EffectCompiler.h"
#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
struct Guid;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IEffectCompilerService : public IService {
protected:
    IEffectCompilerService(const char *serviceName, int servicePriority)
    :   IService(serviceName, servicePriority) {}
public:
    virtual ~IEffectCompilerService() {}

    virtual Engine::EffectCompiler *EffectCompiler() = 0;
    virtual const Engine::EffectCompiler *EffectCompiler() const = 0;

    ENGINESERVICE_GUID_DECL(IEffectCompilerService);
};
//----------------------------------------------------------------------------
class DefaultEffectCompilerService : public IEffectCompilerService {
public:
    DefaultEffectCompilerService();
    virtual ~DefaultEffectCompilerService();

    virtual Engine::EffectCompiler *EffectCompiler() override;
    virtual const Engine::EffectCompiler *EffectCompiler() const override;

    virtual void Start(IServiceProvider *provider, const Guid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const Guid& guid) override;

private:
    Engine::EffectCompiler _effectCompiler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
