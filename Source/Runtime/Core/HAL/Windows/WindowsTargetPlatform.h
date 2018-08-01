#pragma once

#include "Core/HAL/TargetPlatform.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FWindowsTargetPlatform : public ITargetPlaftorm {
public:
    static const FWindowsTargetPlatform& Get();

    virtual ETargetPlatform Platform() const override {
        return ETargetPlatform::Windows;
    }

    virtual FString DisplayName() const override;
    virtual FString FullName() const override;
    virtual FString ShortName() const override;

    virtual bool RequiresFeature(EPlatformFeature feature) const override;
    virtual bool SupportsFeature(EPlatformFeature feature) const override;

private:
    FWindowsTargetPlatform() = default;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
