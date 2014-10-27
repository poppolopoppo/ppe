#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Meta/ThreadResource.h"

namespace Core {
struct Guid;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IService;
//----------------------------------------------------------------------------
class IServiceProvider : public Meta::ThreadResource {
protected:
    IServiceProvider() {}
public:
    virtual ~IServiceProvider() { THIS_THREADRESOURCE_CHECKACCESS(); }

    IServiceProvider(const IServiceProvider& ) = delete;
    IServiceProvider& operator =(const IServiceProvider& ) = delete;

    virtual void RegisterService(const Guid& guid, IService *service) = 0;
    virtual void UnregisterService(const Guid& guid, IService *service) = 0;

    virtual IService *RequestService(const Guid& guid) = 0;

    virtual void Start() = 0;
    virtual void Shutdown() = 0;

public:
    template <typename T>
    T *Service(const Guid& guid) {
        T *const service = checked_cast<T *>(RequestService(guid));
        Assert(service);
        Assert(service->ServiceAvailable());
        return service;
    }

#define ENGINESERVICE_PTR(_Type, _ProviderPtr) \
    (_ProviderPtr)->Service<_Type>(ENGINESERVICE_GUID(_Type))
#define ENGINESERVICE_PROVIDE(_Type, _Name, _ProviderPtr) \
    _Type *const _Name = ENGINESERVICE_PTR(_Type, _ProviderPtr)

#define ENGINESERVICE_REGISTER(_Type, _ProviderPtr, _ServicePtr) \
    (_ProviderPtr)->RegisterService(ENGINESERVICE_GUID(_Type), (_ServicePtr))
#define ENGINESERVICE_UNREGISTER(_Type, _ProviderPtr, _ServicePtr) \
    (_ProviderPtr)->UnregisterService(ENGINESERVICE_GUID(_Type), (_ServicePtr))
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
