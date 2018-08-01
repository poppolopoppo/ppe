#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Effect/EffectCompiler.h"
#include "Core.Engine/Service/IService.h"
#include "Core.Engine/Service/Service_fwd.h"

namespace Core {
struct FGuid;
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

    virtual Engine::FEffectCompiler *FEffectCompiler() = 0;
    virtual const Engine::FEffectCompiler *FEffectCompiler() const = 0;

    ENGINESERVICE_GUID_DECL(IEffectCompilerService);
};
//----------------------------------------------------------------------------
class FDefaultEffectCompilerService : public IEffectCompilerService {
public:
    FDefaultEffectCompilerService();
    virtual ~FDefaultEffectCompilerService();

    virtual Engine::FEffectCompiler *FEffectCompiler() override;
    virtual const Engine::FEffectCompiler *FEffectCompiler() const override;

    virtual void Start(IServiceProvider *provider, const FGuid& guid) override;
    virtual void Shutdown(IServiceProvider *provider, const FGuid& guid) override;

private:
    Engine::FEffectCompiler _effectCompiler;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
