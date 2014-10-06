#pragma once

#include "Engine.h"

#include "EffectCompiler.h"
#include "IService.h"
#include "Service_fwd.h"

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
