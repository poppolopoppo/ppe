#pragma once

#include "RTTI_fwd.h"

#include "Container/PolymorphicTuple.h"

namespace PPE::RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_RTTI_API FGenericUserFacet {};
template <typename T>
CONSTEXPR bool is_user_facet = std::is_base_of_v<FGenericUserFacet, T>;
//----------------------------------------------------------------------------
template <typename _Meta>
struct TUserFacetTraits {
    using decorate_f = void (*)(void*, const _Meta& meta, void* data) NOEXCEPT;
    using validate_f = void (*)(void*, const _Meta& meta, const void* data);

    template <typename T>
    using has_decorator_t = decltype(std::declval<T&>().Decorate(nullptr, std::declval<const _Meta&>(), std::declval<void*>()));
    template <typename T>
    using has_validator_t = decltype(std::declval<T&>().Validate(nullptr, std::declval<const _Meta&>(), std::declval<const void*>()));

    using meta = _Meta;
    using type = TTuple<decorate_f, validate_f>;

    template <typename T>
    static type BindCallbacks(T*) NOEXCEPT {
        STATIC_ASSERT(is_user_facet<T>);
        type result{};
        IF_CONSTEXPR(Meta::has_defined_v<has_decorator_t, T>)
            std::get<decorate_f>(result) = [](void* user, const _Meta& meta, void* data) NOEXCEPT {
                static_cast<T*>(user)->Decorate(meta, data);
            };
        IF_CONSTEXPR(Meta::has_defined_v<has_validator_t, T>)
            std::get<validate_f>(result) = [](void* user, const _Meta& meta, const void* data) {
                static_cast<T*>(user)->Validate(meta, data);
            };
        return result;
    }
};
//----------------------------------------------------------------------------
template <typename _Meta>
class TUserFacet : public TPolymorphicTuple<ALLOCATOR(UserFacet), TUserFacetTraits<_Meta> > {
    using pmr_traits_type = TUserFacetTraits<_Meta>;
    using pmr_tuple_type = TPolymorphicTuple<ALLOCATOR(UserFacet), pmr_traits_type>;

public:
    TUserFacet() = default;

    using pmr_tuple_type::empty;
    using pmr_tuple_type::size;

    using pmr_tuple_type::Get;
    using pmr_tuple_type::GetIFP;

    using pmr_tuple_type::Add;
    using pmr_tuple_type::Remove;
    using pmr_tuple_type::Broadcast;
    using pmr_tuple_type::Clear;

    void Decorate(const _Meta& meta, void* data) const NOEXCEPT {
        Assert(data);
        _tuple.template Broadcast<typename pmr_traits_type::decorate_f>(meta, data);
    }

    void Validate(const _Meta& meta, const void* data) const NOEXCEPT {
        Assert(data);
        _tuple.template Broadcast<typename pmr_traits_type::validate_f>(meta, data);
    }

private:
    pmr_tuple_type _tuple;
};
//----------------------------------------------------------------------------
using FMetaClassFacet = TUserFacet<FMetaClass>;
using FMetaEnumFacet = TUserFacet<FMetaEnum>;
using FMetaFunctionFacet = TUserFacet<FMetaFunction>;
using FMetaParameterFacet = TUserFacet<FMetaParameter>;
using FMetaModuleFacet = TUserFacet<FMetaModule>;
using FMetaPropertyFacet = TUserFacet<FMetaProperty>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TUserFacet<FMetaClass>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TUserFacet<FMetaEnum>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TUserFacet<FMetaFunction>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TUserFacet<FMetaParameter>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TUserFacet<FMetaModule>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TUserFacet<FMetaProperty>;
//----------------------------------------------------------------------------
template <typename _Facet, typename _Meta>
const _Facet* UserFacetIFP(const _Meta& meta) NOEXCEPT {
    return meta.Facets().template GetIFP<_Facet>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //namespace PPE::RTTI
