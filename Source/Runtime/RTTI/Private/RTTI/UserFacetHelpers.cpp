﻿#include "stdafx.h"

#include "RTTI/UserFacetHelpers.h"

#include "MetaClass.h"
#include "MetaFunction.h"
#include "MetaProperty.h"
#include "RTTI/AtomHelpers.h"

#include "Container/HashSet.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/regexp.h"
#include "IO/String.h"
#include "IO/TextWriter.h"
#include "RTTI/Exceptions.h"

namespace PPE::RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDescriptionFacet::FDescriptionFacet(FConstChar text) NOEXCEPT
:   Text(text) {
    Assert(text);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
FWTextWriter& TPropertyFacetException<FEnumFacet>::Description(FWTextWriter& oss) const {
    return FPropertyException::Description(oss)
        << L" -> found value '"
        << FAtom{ Data(), Property()->Traits() }
        << L"' which does not belong user-defined set { "
        << Fmt::Join(Facet().Values.MakeView(), MakeStringView(L", "))
        << L" }";
}
#endif
//----------------------------------------------------------------------------
void FEnumFacet::Validate(const FMetaProperty& meta, const void* data) const {
    const FAtom value{ data, meta.Traits() };

    const bool validated = MakeIterable(Values).Any([&value](const FAny& any) {
        return value.Equals(any.InnerAtom());
    });

    if (Unlikely(not validated))
        PPE_THROW_IT(FValidateException("unexpected value in enum facet", meta, data, *this));
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFormatFacet::FFormatFacet(EBuiltinRegexp format) NOEXCEPT
:   Format(format)
{}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
FWTextWriter& TPropertyFacetException<FFormatFacet>::Description(FWTextWriter& oss) const {
    return FPropertyException::Description(oss)
        << L" -> found string value '"
        << FAtom{ Data(), Property()->Traits() }
        << L"' which does not validate requested format <"
        << Facet().Format
        << L">";
}
#endif
//----------------------------------------------------------------------------
void FFormatFacet::Validate(const FMetaProperty& meta, const void* data) const {
    AssertMessage(L"format facet can only be applied on string types",
        meta.Traits()->TypeInfos().IsString() );

    bool validated;
    switch (static_cast<ENativeType>(meta.Traits()->TypeId())) {
    case ENativeType::Name:
        validated = FRegexp::BuiltinFormat(Format, ECase::Insensitive)
            .Match(static_cast<const FName*>(data)->MakeView());
        break;
    case ENativeType::String:
        validated = FRegexp::BuiltinFormat(Format, ECase::Insensitive)
            .Match(static_cast<const FString*>(data)->MakeView());
        break;
    case ENativeType::WString:
        validated = FWRegexp::BuiltinFormat(Format, ECase::Insensitive)
            .Match(static_cast<const FWString*>(data)->MakeView());
        break;
    default:
        AssertNotImplemented();
    }

    if (Unlikely(not validated))
        PPE_THROW_IT(FValidateException("not validated by format facet", meta, data, *this));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLengthFacet::FLengthFacet(size_t minLength, size_t maxLength) NOEXCEPT
:   MinLength(minLength)
,   MaxLength(maxLength) {
    Assert_NoAssume(minLength <= maxLength);
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
FWTextWriter& TPropertyFacetException<FLengthFacet>::Description(FWTextWriter& oss) const {
    return FPropertyException::Description(oss)
        << L" -> found vector value '"
        << FAtom{ Data(), Property()->Traits() }
        << L"' with arity outside requested range [ "
        << Facet().MinLength
        << "; "
        << Facet().MaxLength
        << L" ]";
}
#endif
//----------------------------------------------------------------------------
void FLengthFacet::Validate(const FMetaProperty& meta, const void* data) const {
    const PTypeTraits traits = meta.Traits();

    size_t arity;
    if (const IScalarTraits* const pScalar = traits->AsScalar()) {
        switch (static_cast<ENativeType>(meta.Traits()->TypeId())) {
        case ENativeType::Name:
            arity = static_cast<const FName*>(data)->size();
            break;
        case ENativeType::String:
            arity = static_cast<const FString*>(data)->size();
            break;
        case ENativeType::WString:
            arity = static_cast<const FWString*>(data)->size();
            break;
        case ENativeType::BinaryData:
            arity = static_cast<const FBinaryData*>(data)->size();
            break;
        default:
            AssertNotImplemented();
        }
    }
    else if (const ITupleTraits* const pTuple = traits->AsTuple())
        arity = pTuple->Arity();
    else if (const IListTraits* const pList = traits->AsList())
        arity = pList->Count(data);
    else if (const IDicoTraits* const pDico = traits->AsDico())
        arity = pDico->Count(data);
    else
        AssertNotReached();

    const bool validated = (arity >= MinLength && arity <= MaxLength);

    if (Unlikely(not validated))
        PPE_THROW_IT(FValidateException("not validated by length facet", meta, data, *this));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPatternFacet::FPatternFacet(FConstChar pattern) NOEXCEPT
:   Pattern(pattern) {
    Assert_NoAssume(Pattern);
    Assert_NoAssume(FRegexp::ValidateSyntax(Pattern.MakeView()));
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
FWTextWriter& TPropertyFacetException<FPatternFacet>::Description(FWTextWriter& oss) const {
    return FPropertyException::Description(oss)
        << L" -> found string value '"
        << FAtom{ Data(), Property()->Traits() }
        << L"' which does not validated requested pattern /"
        << Facet().Pattern.MakeView()
        << L"/i";
}
#endif
//----------------------------------------------------------------------------
void FPatternFacet::Validate(const FMetaProperty& meta, const void* data) const {
    AssertMessage(L"format facet can only be applied on string types",
        meta.Traits()->TypeInfos().IsString() );

    const FRegexp re{ Pattern.MakeView(), ECase::Insensitive };

    bool validated;
    switch (static_cast<ENativeType>(meta.Traits()->TypeId())) {
    case ENativeType::Name:
        validated = re.Match(static_cast<const FName*>(data)->MakeView());
        break;
    case ENativeType::String:
        validated = re.Match(static_cast<const FString*>(data)->MakeView());
        break;
    default:
        AssertNotImplemented();
    }

    if (Unlikely(not validated))
        PPE_THROW_IT(FValidateException("not validated by pattern facet", meta, data, *this));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FRangeFacet::Decorate(const FMetaProperty& meta, void* data) const NOEXCEPT {
    AssertMessage(L"range facet can only be applied on arithmetic types",
        meta.Traits()->TypeInfos().IsArithmetic() );

    const IScalarTraits& scalar = meta.Traits()->ToScalar();

    if (meta.Traits() == Minimum.Traits()) {
        if (scalar.Less(data, Minimum.Data()))
            scalar.Copy(Minimum.Data(), data);
        else if (scalar.Greater(data, Maximum.Data()))
            scalar.Copy(Maximum.Data(), data);
    }
    else {
        STACKLOCAL_ATOM(tmp, meta.Traits()); // for polymorphic promotion

        VerifyRelease(Minimum.PromoteCopy(tmp.MakeAtom()));
        if (scalar.Less(data, tmp.MakeAtom().Data()))
            scalar.Copy(tmp.MakeAtom().Data(), data);
        else {
            VerifyRelease(Maximum.PromoteCopy(tmp.MakeAtom()));
            if (scalar.Greater(data, tmp.MakeAtom().Data()))
                scalar.Copy(tmp.MakeAtom().Data(), data);
        }
    }
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
FWTextWriter& TPropertyFacetException<FRangeFacet>::Description(FWTextWriter& oss) const {
    return FPropertyException::Description(oss)
        << L" -> found arithmetic value '"
        << FAtom{ Data(), Property()->Traits() }
        << L"' which lies outside requested range [ "
        << Facet().Minimum
        << "; "
        << Facet().Maximum
        << L" ]";
}
#endif
//----------------------------------------------------------------------------
void FRangeFacet::Validate(const FMetaProperty& meta, const void* data) const {
    const IScalarTraits& scalar = meta.Traits()->ToScalar();

    bool validated = true;
    if (meta.Traits() == Minimum.Traits()) {
        validated &= scalar.GreaterEqual(data, Minimum.Data());
        validated &= scalar.LessEqual(data, Maximum.Data());
    }
    else {
        STACKLOCAL_ATOM(tmp, meta.Traits()); // for polymorphic promotion

        VerifyRelease(Minimum.PromoteCopy(tmp.MakeAtom()));
        validated &= scalar.GreaterEqual(data, tmp.MakeAtom().Data());

        VerifyRelease(Maximum.PromoteCopy(tmp.MakeAtom()));
        validated &= scalar.LessEqual(data, tmp.MakeAtom().Data());
    }

    if (Unlikely(not validated))
        PPE_THROW_IT(FValidateException("not validated by range facet", meta, data, *this));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
FWTextWriter& TPropertyFacetException<FUniqueFacet>::Description(FWTextWriter& oss) const {
    return FPropertyException::Description(oss)
        << L" -> list contains duplicated entries '"
        << FAtom{ Data(), Property()->Traits() }
        << L"'";
}
#endif
//----------------------------------------------------------------------------
void FUniqueFacet::Validate(const FMetaProperty& meta, const void* data) const {
    AssertMessage(L"unique facet can only be applied on list types",
        meta.Traits()->TypeInfos().IsList() );

    struct FAtomEquals_ {
        bool operator ()(const THashMemoizer<FAtom>& lhs, const THashMemoizer<FAtom>& rhs) const NOEXCEPT {
            return (lhs->HashValue() == rhs->HashValue()
                ? lhs->Equals(*rhs) : false );
        }
    };

    THashSet<
        THashMemoizer<FAtom>,
        Meta::THash<>,
        FAtomEquals_,
        ALLOCATOR(UserFacet)
    >   visited;

    const IListTraits& list = meta.Traits()->ToList();
    visited.reserve(list.Count(data));

    const bool validated = list.ForEach(const_cast<void*>(data),
        [&visited](const FAtom& it) -> bool {
            return not visited.insert_ReturnIfExists(it);
        });

    if (Unlikely(not validated))
        PPE_THROW_IT(FValidateException("not validated by unique facet", meta, data, *this));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //namespace PPE::RTTI