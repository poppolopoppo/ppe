#pragma once

#include "VirtualFileSystem_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_VFS_API, VFS)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_VFS_API FVFSModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FVFSModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

public:
    static FVFSModule& Get(const FModularDomain& domain);

    // project data directory
    NODISCARD FMountingPoint DataDir() const;
    // current process executable directory
    NODISCARD FMountingPoint ProcessDir() const;
    // saved directory
    NODISCARD FMountingPoint SavedDir() const;
    // operating system path
    NODISCARD FMountingPoint SystemDir() const;
    // system temporary path
    NODISCARD FMountingPoint TmpDir() const;
    // user profile path
    NODISCARD FMountingPoint UserDir() const;
    // current process working directory
    NODISCARD FMountingPoint WorkingDir() const;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
