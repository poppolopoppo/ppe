#pragma once

#include "Core.h"

#include "Meta/ThreadResource.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FModule;
//----------------------------------------------------------------------------
class PPE_CORE_API FModuleManager : Meta::FThreadResource {
public:
    FModuleManager(
        void* appHandle, int showCmd,
        const wchar_t* filename,
        size_t argc, const wchar_t** argv );
    ~FModuleManager();

    FModuleManager(const FModuleManager&) = delete;
    FModuleManager& operator =(const FModuleManager&) = delete;

    FModuleManager(FModuleManager&&) = delete;
    FModuleManager& operator =(FModuleManager&&) = delete;

    void* AppHandle() const { return _appHandle; }
    int ShowCmd() const { return _showCmd; }
    const wchar_t* Filename() const { return _filename; }
    size_t Argc() const { return _argc; }
    const wchar_t** Argv() const { return _argv; }

    void PreInit(IModuleStartup& startup);
    void PostDestroy(IModuleStartup& startup);

    void Start(FModule& module);
    void Shutdown(FModule& module);
    void ReleaseMemory(FModule& module);

    void ReleaseMemoryInModules() const;

private:
    void* const _appHandle;
    const int _showCmd;
    const wchar_t* const _filename;
    const size_t _argc;
    const wchar_t** const _argv;
    IModuleStartup* _startup;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
