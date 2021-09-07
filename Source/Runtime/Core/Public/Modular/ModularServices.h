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

    template <typename _Interface>
    NODISCARD _Interface& Get() const NOEXCEPT {
        auto* const p = GetIFP<_Interface>();
        AssertRelease(p);
        return (*p);
    }

    template <typename _Interface>
    NODISCARD _Interface* GetIFP() const NOEXCEPT {
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
    void ReleaseMemory() NOEXCEPT;

private:
    const FStringView _name;
    const FModularServices* _parent;

    template <typename T>
    using THasReleaseMemoryFunc_ = decltype(std::declval<T&>().ReleaseMemory());
    using FReleaseMemoryFunc_ = void (*)(void*) NOEXCEPT;

    template <typename T>
    static FReleaseMemoryFunc_ MakeReleaseFunc_(T*) NOEXCEPT {
        IF_CONSTEXPR(Meta::has_defined_v<THasReleaseMemoryFunc_, T>) {
            return [](void* p) NOEXCEPT {
                static_cast<T*>(p)->ReleaseMemory();
            };
        }
        return nullptr;
    }

    struct FPolymorphicServiceTraits_ {
        using type = TTuple<FReleaseMemoryFunc_>;
        template <typename T>
        static type BindCallbacks(T* p) NOEXCEPT {
            return std::make_tuple(MakeReleaseFunc_(p));
        }
    };

    using FPolymorphicServices_ = TPolymorphicTuple<ALLOCATOR(Modular), FPolymorphicServiceTraits_>;
    FPolymorphicServices_ _services;

#if USE_PPE_LOGGER
    void LogServiceAdd_(FStringView base, FStringView derived) const NOEXCEPT;
    void LogServiceRemove_(FStringView base) const NOEXCEPT;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
