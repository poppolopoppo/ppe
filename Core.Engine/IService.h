#pragma once

#include "Engine.h"

#include "Core/RefPtr.h"
#include "Core/String.h"
#include "Core/ThreadResource.h"

#ifdef _DEBUG
#   define WITH_CORE_ENGINE_SERVICE_DEBUG
#endif

namespace Core {
struct Guid;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IService;
class IServiceProvider;
typedef RefPtr<IService> PService;
typedef RefPtr<const IService> PCService;
//----------------------------------------------------------------------------
class IService : public RefCountable, public Meta::ThreadResource {
protected:
    IService(const char *serviceName, int servicePriority);
public:
    virtual ~IService();

    IService(const IService& ) = delete;
    IService& operator =(const IService& ) = delete;

    IService(IService&& ) = delete;
    IService& operator =(IService&& ) = delete;

#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    bool ServiceAvailable() const { return nullptr != _serviceProvider; }
    const char *ServiceName() const { THIS_THREADRESOURCE_CHECKACCESS(); return _serviceName.c_str(); }
#else
    bool ServiceAvailable() const { return true; }
    const char *ServiceName() const { AssertNotImplemented(); return nullptr; }
#endif

    int ServicePriority() const { return _servicePriority; }

    // should be virtual pure, but debug is inserted inside
    virtual void Start(IServiceProvider *provider, const Guid& guid);
    virtual void Shutdown(IServiceProvider *provider, const Guid& guid);

private:
    int _servicePriority;
#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
    String _serviceName;
    const IServiceProvider *_serviceProvider;
#endif
};
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ENGINE_SERVICE_DEBUG
#define ENGINESERVICE_NAME(_Name) STRINGIZE(_Name)
#else
#define ENGINESERVICE_NAME(_Name) nullptr
#endif
//----------------------------------------------------------------------------
#define ENGINESERVICE_GUID(_Name) \
    _Name::ServiceGuid
//----------------------------------------------------------------------------
#define ENGINESERVICE_GUID_DECL(_Name) \
    static const Guid ServiceGuid
//----------------------------------------------------------------------------
#define ENGINESERVICE_GUID_DEF(_Name) \
    const Guid _Name::ServiceGuid = Guid::FromCStr(STRINGIZE(_Name))
//----------------------------------------------------------------------------
#define ENGINESERVICE_CONSTRUCT(_Name) \
    ENGINESERVICE_NAME(_Name), Core::Engine::ServicePriority::_Name
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
