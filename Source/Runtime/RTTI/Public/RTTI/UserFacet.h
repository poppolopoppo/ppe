#pragma once

#include "RTTI_fwd.h"

#include "Container/PolymorphicTuple.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_RTTI_API FGenericUserFacet {};
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool is_user_facet = std::is_base_of_v<FGenericUserFacet, T>;
//----------------------------------------------------------------------------
namespace details {
template <typename T>
using TUserFacetHasDecorate = Meta::TValue<&T::Decorate>;
template <typename _Meta>
using TUserFacetDecorate = TPolymorphicFunc<TUserFacetHasDecorate, void, const _Meta&, void*>;
} //!details
//----------------------------------------------------------------------------
namespace details {
template <typename T>
using TUserFacetHasValidate = Meta::TValue<&T::Validate>;
template <typename _Meta>
using TUserFacetValidate = TPolymorphicFunc<TUserFacetHasValidate, void, const _Meta&, const void*>;
} //!details
//----------------------------------------------------------------------------
template <typename _Meta>
class TUserFacet : public TPolymorphicTuple<
    ALLOCATOR(UserFacet),
    details::TUserFacetDecorate<_Meta>,
    details::TUserFacetValidate<_Meta>
> {
public:
    using parent_type = TPolymorphicTuple<
        ALLOCATOR(UserFacet),
        details::TUserFacetDecorate<_Meta>,
        details::TUserFacetValidate<_Meta>
    >;

    using parent_type::parent_type;
    using parent_type::Broadcast;

    TUserFacet() = default;

    void Decorate(const _Meta& meta, void* data) const {
        Assert(data);
        parent_type::template Broadcast<details::TUserFacetDecorate<_Meta>>(meta, data);
    }

    void Validate(const _Meta& meta, const void* data) const {
        Assert(data);
        parent_type::template Broadcast<details::TUserFacetValidate<_Meta>>(meta, data);
    }
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
} //namespace RTTI
} //namespace PPE
