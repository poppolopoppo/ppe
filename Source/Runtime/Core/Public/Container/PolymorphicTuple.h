#pragma once

#include "Core_fwd.h"

#include "Container/FlatMap.h"
#include "Container/Pair.h"
#include "Container/Tuple.h"
#include "Memory/RefPtr.h"
#include "Memory/PtrRef.h"
#include "Memory/UniquePtr.h"
#include "Meta/TypeInfo.h"

#define POLYMORPHIC_TUPLE(_DOMAIN) ::PPE::TPolymorphicTuple< ALLOCATOR(_DOMAIN) >

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <template<class> class _Func, typename _Ret, typename... _Args>
struct TPolymorphicFunc {
    using result = _Ret;
    struct type {
        _Ret(*fn)(void*, _Args...){ nullptr };
    };
    template <typename T>
    static CONSTEXPR type Bind() {
        type result{};
        if constexpr (Meta::has_defined_v<_Func, T>)
            result.fn = [](void* p, _Args... args) -> _Ret {
                return (static_cast<T*>(p)->*_Func<T>::value)(std::forward<_Args>(args)...);
            };
        return result;
    }
};
//----------------------------------------------------------------------------
template <auto _Lambda, typename _Ret, typename... _Args>
struct TPolymorphicLambda {
    using result = _Ret;
    struct type {
        _Ret(*fn)(void*, _Args...){ nullptr };
    };
    template <typename T>
    using available_t = decltype(_Lambda(std::declval<T*>(), std::declval<_Args>()...));
    template <typename T>
    static CONSTEXPR type Bind() {
        type result{};
        if constexpr (Meta::has_defined_v<available_t, T>)
            result.fn = [](void* p, _Args... args) -> _Ret {
                return _Lambda(static_cast<T*>(p), std::forward<_Args>(args)...);
            };
        return result;
    }
};
//----------------------------------------------------------------------------
template <typename _Allocator = ALLOCATOR(Container), typename... _Funcs>
class TPolymorphicTuple {
public:
    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    struct FDeleterFunc {
        using type = FDeleterFunc;
        void (*fn)(void*, allocator_type&){nullptr};
    };

    using FVirtualTable = std::tuple<
        FDeleterFunc,
        typename _Funcs::type...
    >;

    using FTypeKey = Meta::type_info_t;

    struct FOpaqueItem {
        void* Data;
        FVirtualTable Callbacks;

        template <typename _Callback, typename... _Args>
        bool Invoke(_Args&&... args) const {
            Assert(Data);
            if (const _Callback callback = std::get<_Callback>(Callbacks); callback.fn) {
                callback.fn(Data, std::forward<_Args>(args)...);
                return true;
            }
            return false;
        }
    };

    TPolymorphicTuple() = default;

    explicit TPolymorphicTuple(const allocator_type& alloc) : _items(alloc) {}
    explicit TPolymorphicTuple(allocator_type&& ralloc) NOEXCEPT : _items(std::move(ralloc)) {}

    TPolymorphicTuple(TPolymorphicTuple&& ) = default;
    TPolymorphicTuple& operator =(TPolymorphicTuple&& ) = default;

    TPolymorphicTuple(const TPolymorphicTuple& ) = delete;
    TPolymorphicTuple& operator =(const TPolymorphicTuple& ) = delete;

    ~TPolymorphicTuple() {
        Clear();
    }

    bool empty() const { return _items.empty(); }
    size_t size() const { return _items.size(); }

    TMemoryView<const TPair<Meta::type_info_t, FOpaqueItem>> MakeView() const {
        return _items.MakeView();
    }

    const FOpaqueItem& Get(const FTypeKey& key) const NOEXCEPT {
        return _items.Get(key);
    }

    Meta::TOptionalReference<const FOpaqueItem> GetIFP(const FTypeKey& key) const NOEXCEPT {
        return _items.GetIFP(key);
    }

    template <typename _Func, typename... _Args, decltype(std::declval<typename _Func::type>().fn(nullptr, std::declval<_Args&&>()...))* = nullptr>
    void Broadcast(_Args&&... args) const {
        using callback_t = typename _Func::type;
        for (const auto& it : _items)
            it.second.template Invoke<callback_t>(std::forward<_Args>(args)...);
    }

    template <typename T>
    T& Get() const NOEXCEPT {
        return (*static_cast<T*>(_items.Get(Meta::type_info<T>).Data));
    }

    template <typename T>
    Meta::TOptionalReference<T> GetIFP() const NOEXCEPT {
        if (const FOpaqueItem* opaque = _items.GetIFP(Meta::type_info<T>))
            return Meta::MakeOptionalRef(static_cast<T*>(opaque->Data));
        return Default;
    }

    template <typename T, typename... _Args>
    Meta::TEnableIf<Meta::has_constructor<T, _Args&&...>::value> Emplace(_Args&&... args) {
        T item{ std::forward<_Args>(args)... };
        return Add<T>(std::move(item));
    }

    template <typename T, typename _Derived = T>
    Meta::TEnableIf<std::is_base_of_v<T, _Derived> && std::is_move_assignable_v<T>> Add(_Derived&& rvalue) {
        _Derived* const opaque = allocator_traits::template AllocateOneT<_Derived>(get_allocator());
        Meta::Construct(opaque, std::move(rvalue));
        Insert_AssertUnique_<T>(opaque, [](void* p, allocator_type& al) {
            Meta::Destroy(static_cast<_Derived*>(p));
            allocator_traits::template DeallocateOneT<_Derived>(al, static_cast<T*>(p));
        });
    }

    template <typename T, typename _Derived = T>
    Meta::TEnableIf<std::is_base_of_v<T, _Derived>> Add(const TPtrRef<_Derived>& ptrRef) {
        Assert(ptrRef);
        _Derived* const opaque = ptrRef.get();
        Insert_AssertUnique_<T>(opaque);
    }

    template <typename T, typename _Derived = T>
    Meta::TEnableIf<std::is_base_of_v<T, _Derived>> Add(const TRefPtr<_Derived>& refPtr) {
        Assert(refPtr);
        _Derived* const opaque = refPtr.get();
        AddRef(opaque);
        Insert_AssertUnique_<T>(opaque, [](void* p, allocator_type& ) {
            RemoveRef(static_cast<_Derived*>(p));
        });
    }

    template <typename T, typename _Derived = T>
    Meta::TEnableIf<std::is_base_of_v<T, _Derived>> Add(TUniquePtr<_Derived>&& uniquePtr) {
        Assert(uniquePtr);
        _Derived* const opaque = uniquePtr.get();
        POD_STORAGE(TUniquePtr<_Derived>) no_dtor; // #HACK: steal reference without destroying
        INPLACE_NEW(&no_dtor, TUniquePtr<_Derived>){ std::move(uniquePtr) };
        Insert_AssertUnique_<T>(opaque, [](void* p, allocator_type& ) {
            TUniquePtr<_Derived>::Deleter()(static_cast<_Derived*>(p));
        });
    }

    template <typename T, typename _Derived = T>
    Meta::TEnableIf<std::is_base_of_v<T, _Derived>> Add(const TSafePtr<_Derived>& safePtr) {
        Assert(safePtr);
        _Derived* const opaque = safePtr.get();
#if USE_PPE_SAFEPTR
        AddSafeRef(opaque);
        Insert_AssertUnique_<T>(opaque, [](void* p, allocator_type& ) {
            RemoveSafeRef(static_cast<_Derived*>(p));
        });
#else
        Insert_AssertUnique_<T>(opaque);
#endif
    }

    template <typename T, typename _Derived = T>
    Meta::TEnableIf<std::is_base_of_v<T, _Derived>> Add(_Derived* rawPtr) {
        Assert(rawPtr);
        IF_CONSTEXPR(IsRefCountable<_Derived>::value) {
            Add<T>(MakeSafePtr(rawPtr));
        }
        else {
            Insert_AssertUnique_<T>(rawPtr);
        }
    }

    template <typename... _Facets>
    void Append(_Facets&&... facets) {
        FOLD_EXPR( Add<Meta::TDecay<_Facets>>(std::forward<_Facets>(facets)) );
    }

    template <typename T>
    void Remove() {
        const auto it = _items.find(Meta::type_info<T>);
        AssertRelease(_items.end() != it);
        it->second.template Invoke<FDeleterFunc>(get_allocator());
        _items.Erase(it);
    }

    void Clear() {
        Broadcast<FDeleterFunc>(get_allocator());
        _items.clear_ReleaseMemory();
    }

    allocator_type& get_allocator() { return _items.get_allocator(); }
    const allocator_type& get_allocator() const { return _items.get_allocator(); }

private:
    using FItemMap = TFlatMap<
        FTypeKey, FOpaqueItem,
        Meta::TEqualTo<FTypeKey>,
        Meta::TLess<FTypeKey>,
        TVector<TPair<FTypeKey, FOpaqueItem>, allocator_type>
    >;

    FItemMap _items;

    template <typename T, typename _Derived = T>
    void Insert_AssertUnique_(_Derived* rawPtr, decltype(FDeleterFunc::fn) deleter = nullptr) {
        Assert(rawPtr);
        _items.Insert_AssertUnique(
            FTypeKey{ Meta::type_info<T> },
            FOpaqueItem{
                rawPtr,
                std::make_tuple(
                    FDeleterFunc{deleter},
                    _Funcs::template Bind<T>()... ) });
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //namespace PPE
