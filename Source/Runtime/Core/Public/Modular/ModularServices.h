#pragma once

#include "Core_fwd.h"

#include "Modular/Modular_fwd.h"

#include "Container/PolymorphicTuple.h"
#include "Diagnostic/Logger_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FModularServices : Meta::FNonCopyableNorMovable {
public:
    explicit FModularServices(const FStringView& name) NOEXCEPT;
    FModularServices(const FStringView& name, const FModularServices* parent) NOEXCEPT;
    ~FModularServices();

    bool empty() const { return _services.empty(); }
    size_t size() const { return _services.size(); }

    NODISCARD void* GetIFP(const Meta::type_info_t& serviceKey) const NOEXCEPT {
        if (auto* const pOpaqueItem = _services.GetIFP(serviceKey))
            return pOpaqueItem->Data;

        return (_parent ? _parent->GetIFP(serviceKey) : nullptr);
    }

    template <typename _Interface>
    NODISCARD _Interface& Get() const NOEXCEPT {
        auto* const p = GetIFP<_Interface>();
        AssertRelease(p);
        return (*p);
    }

    template <typename _Interface>
    NODISCARD Meta::TOptionalReference<_Interface> GetIFP() const NOEXCEPT {
        if (_Interface* const p = _services.GetIFP<_Interface>())
            return p;
        return (_parent ? _parent->GetIFP<_Interface>() : nullptr);
    }

    template <typename _Interface, typename _Service>
    void Add(_Service&& rservice) {
#if USE_PPE_LOGGER
        LogServiceAdd_(Meta::type_info<_Interface>.name.MakeView(), Meta::type_info<_Service>.name.MakeView());
#endif
        _services.Add<_Interface>(std::forward<_Service>(rservice));
    }

    template <typename _Interface>
    void Remove() {
#if USE_PPE_LOGGER
        LogServiceRemove_(Meta::type_info<_Interface>.name.MakeView());
#endif
        _services.Remove<_Interface>();
    }

    void Clear();

    void Run(const FModularDomain& domain);
    void ReleaseMemory() NOEXCEPT;

private:
    const FStringView _name;
    const FModularServices* _parent;

    template <typename T>
    using release_memory_t = Meta::TValue<&T::ReleaseMemory>;
    template <typename T>
    using run_t = Meta::TValue<&T::Run>;

    using release_memory_f = TPolymorphicFunc<release_memory_t, void>;
    using run_f = TPolymorphicFunc<run_t, void, const FModularDomain&>;

    using FPolymorphicServices_ = TPolymorphicTuple<ALLOCATOR(Modular),
        release_memory_f, run_f>;
    FPolymorphicServices_ _services;

#if USE_PPE_LOGGER
    void LogServiceAdd_(FStringView base, FStringView derived) const NOEXCEPT;
    void LogServiceRemove_(FStringView base) const NOEXCEPT;
    static void LogServiceRun_(FStringView base) NOEXCEPT;
    static void LogServiceReleaseMemory_(FStringView base) NOEXCEPT;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
