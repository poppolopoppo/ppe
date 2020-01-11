#pragma once

#include "HAL/TargetPlatform.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FLinuxTargetPlatform : public ITargetPlaftorm {
public:
    static const FLinuxTargetPlatform& Get();

    virtual ETargetPlatform Platform() const override {
        return ETargetPlatform::Linux;
    }

    virtual FString DisplayName() const override;
    virtual FString FullName() const override;
    virtual FString ShortName() const override;

    virtual bool RequiresFeature(EPlatformFeature feature) const override;
    virtual bool SupportsFeature(EPlatformFeature feature) const override;

private:
    FLinuxTargetPlatform() = default;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
