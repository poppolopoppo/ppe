#include "stdafx.h"

#include "NativeTypes.h"

#include "TypeInfos.h"

#include "Any.h"
#include "AtomVisitor.h" // needed for Accept() & PrettyPrint()
#include "MetaObject.h" // needed for PMetaObject manipulation
#include "MetaObjectHelpers.h" // needed for DeepEquals()

#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter.h"
#include "Core/Maths/MathHelpers.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarMatrixHelpers.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool IsAssignableObject_(const ITypeTraits& src, const ITypeTraits& dst, const PMetaObject& pobj) {
    if (src == dst)
        return true;

    const ETypeFlags srcFlags = src.TypeInfos().Flags();
    const ETypeFlags dstFlags = dst.TypeInfos().Flags();

    if (srcFlags & ETypeFlags::Object &&
        dstFlags & ETypeFlags::Object ) {

        const FMetaClass* srcClass = checked_cast<const FBaseObjectTraits*>(&src)->MetaClass();
        const FMetaClass* dstClass = checked_cast<const FBaseObjectTraits*>(&dst)->MetaClass();

        return (dstClass->IsAssignableFrom(*srcClass));
    }
    else if (dstFlags & ETypeFlags::Object) {
        if (nullptr == pobj)
            return true;

        const FMetaClass* srcClass = pobj->RTTI_Class();
        const FMetaClass* dstClass = checked_cast<const FBaseObjectTraits*>(&dst)->MetaClass();

        return (dstClass->IsAssignableFrom(*srcClass));
    }
    else if (srcFlags & ETypeFlags::Object) {
        // can always down cast from T* to FMetaObject* when T inherits from FMetaObject

        return true;
    }
    else {
        AssertNotReached(); // (src == dst) should have returned true at the top ^^^
    }
}
//----------------------------------------------------------------------------
static bool DeepEqualsObject_(const void* lhs, const void* rhs) {
    const PMetaObject& lhsObj = (*reinterpret_cast<const PMetaObject*>(lhs));
    const PMetaObject& rhsObj = (*reinterpret_cast<const PMetaObject*>(rhs));

    if (lhsObj == rhsObj)
        return true;
    else if (!lhsObj || !rhsObj)
        return false;
    else
        return RTTI::DeepEquals(*lhsObj, *rhsObj);
}
//----------------------------------------------------------------------------
static void DeepCopyObject_(const ITypeTraits& self, const void* src, void* dst) {
    const PMetaObject& srcObj = (*reinterpret_cast<const PMetaObject*>(src));
    PMetaObject& dstObj = (*reinterpret_cast<PMetaObject*>(dst));

    if (srcObj == dstObj) {
        return;
    }
    else if (nullptr == srcObj) {
        dstObj = nullptr;
        return;
    }
    else if (nullptr == dstObj) {
        Verify(srcObj->RTTI_Class()->CreateInstance(dstObj, false/* don't reset since we deep copy */));
    }

    RTTI::DeepCopy(*srcObj, *dstObj);
}
//----------------------------------------------------------------------------
static bool PromoteCopyObject_(const ITypeTraits& self, const void* src, const FAtom& dst) {
    if (self == *dst.Traits()) {
        self.Copy(src, dst.Data());
        return true;
    }
    else if (dst.TypeId() == FTypeId(ENativeType::MetaObject)) {
        const PMetaObject& srcObj = (*reinterpret_cast<const PMetaObject*>(src));

        if (IsAssignableObject_(self, *dst.Traits(), srcObj)) {
            (*reinterpret_cast<PMetaObject*>(dst.Data())) = srcObj;
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------------------------
static bool PromoteMoveObject_(const ITypeTraits& self, void* src, const FAtom& dst) {
    if (self == *dst.Traits()) {
        self.Move(src, dst.Data());
        return true;
    }
    else if (dst.TypeId() == FTypeId(ENativeType::MetaObject)) {
        PMetaObject& srcObj = (*reinterpret_cast<PMetaObject*>(src));

        if (IsAssignableObject_(self, *dst.Traits(), srcObj)) {
            (*reinterpret_cast<PMetaObject*>(dst.Data())) = std::move(srcObj);
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------------------------
static void* CastObject_(const ITypeTraits& self, void* data, const PTypeTraits& dst) {
    if (self == *dst) {
        return data;
    }
    else if (dst->TypeId() == FTypeId(ENativeType::MetaObject)) {
        const PMetaObject& pobj = (*reinterpret_cast<const PMetaObject*>(data));

        if (IsAssignableObject_(self, *dst, pobj))
            return data;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TNativeTypeTraits : public TBaseTypeTraits<T, TBaseScalarTraits<T> > {
    using base_traits = TBaseTypeTraits<T, TBaseScalarTraits<T> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public:
    // specialized explicitly
    virtual FTypeId TypeId() const override final;
    virtual FTypeInfos TypeInfos() const override final;
    // !specialized explicitly

    virtual bool IsDefaultValue(const void* data) const override final {
        Assert(data);

        return (*reinterpret_cast<const value_type*>(data) == Meta::MakeForceInit<T>());
    }

    virtual void ResetToDefaultValue(void* data) const override final {
        Assert(data);

        *reinterpret_cast<value_type*>(data) = Meta::MakeForceInit<T>();
    }

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final {
        return base_traits::Equals(lhs, rhs); // deep equals <=> equals for native types
    }

    virtual void DeepCopy(const void* src, void* dst) const override final {
        base_traits::Copy(src, dst); // deep copy <=> copy for native types
    }

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final {
        Assert(src);
        Assert(dst);

        if (*dst.Traits() == *this) {
            Copy(src, dst.Data());
            return true;
        }

        return false;
    }

    virtual bool PromoteMove(void* src, const FAtom& dst) const override final {
        Assert(src);
        Assert(dst);

        if (*dst.Traits() == *this) {
            Move(src, dst.Data());
            return true;
        }

        return false;
    }

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final {
        return base_traits::Cast(data, dst);
    }

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final {
        Assert(data);

        return visitor->Visit(
            static_cast<const IScalarTraits*>(this),
            *reinterpret_cast<value_type*>(data) );
    }
};
//----------------------------------------------------------------------------
// FAny specializations
//----------------------------------------------------------------------------
template <>
void* TNativeTypeTraits<FAny>::Cast(void* data, const PTypeTraits& dst) const {
    Assert(data);

    return (*dst != *this
        ? reinterpret_cast<FAny*>(data)->Traits()->Cast(
            reinterpret_cast<FAny*>(data)->InnerAtom().Data(), dst)
        : data );
}
//----------------------------------------------------------------------------
// PMetaObject specializations
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::DeepEquals(const void* lhs, const void* rhs) const {
    return DeepEqualsObject_(lhs, rhs);
}
//----------------------------------------------------------------------------
template <>
void TNativeTypeTraits<PMetaObject>::DeepCopy(const void* src, void* dst) const {
    DeepCopyObject_(*this, src, dst);
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::PromoteCopy(const void* src, const FAtom& dst) const {
    return PromoteCopyObject_(*this, src, dst);
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::PromoteMove(void* src, const FAtom& dst) const {
    return PromoteMoveObject_(*this, src, dst);
}
//----------------------------------------------------------------------------
template <>
void* TNativeTypeTraits<PMetaObject>::Cast(void* data, const PTypeTraits& dst) const {
    return CastObject_(*this, data, dst);
}
//----------------------------------------------------------------------------
// Specialize Traits(), TypeId() & TypeInfos() for each native type
//----------------------------------------------------------------------------
#define DEF_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) \
    /* TNativeTypeTraits<T> */ \
    template <> \
    FTypeId TNativeTypeTraits<T>::TypeId() const { \
        return FTypeId(ENativeType::_Name); \
    } \
    \
    template <> \
    FTypeInfos TNativeTypeTraits<T>::TypeInfos() const { \
        return FTypeInfos( \
            STRINGIZE(_Name), \
            FTypeId(ENativeType::_Name), \
            ETypeFlags::Scalar|ETypeFlags::Native, \
            sizeof(T)); \
    } \
    \
    /* Global helper for MakeTraits<T>() */ \
    PTypeTraits Traits(Meta::TType<T>) { \
        return PTypeTraits::Make< TNativeTypeTraits<T> >(); \
    }

FOREACH_RTTI_NATIVETYPES(DEF_RTTI_NATIVETYPE_TRAITS)

#undef DEF_RTTI_NATIVETYPE_TRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PTypeTraits MakeTraits(ENativeType nativeType) {
    switch (nativeType) {
#define DEF_RTTI_MAKETRAITS(_Name, T, _TypeId) \
    case ENativeType::_Name: return MakeTraits<T>();
    FOREACH_RTTI_NATIVETYPES(DEF_RTTI_MAKETRAITS)
#undef DEF_RTTI_MAKETRAITS
    default:
        AssertNotImplemented();
        break;
    }
    return PTypeTraits();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTypeId FBaseObjectTraits::TypeId() const {
    return FTypeId(ENativeType::MetaObject);
}
//----------------------------------------------------------------------------
FTypeInfos FBaseObjectTraits::TypeInfos() const {
    return FTypeInfos(
        MetaClass()->Name().MakeView(),
        FTypeId(ENativeType::MetaObject),
        ETypeFlags::Scalar | ETypeFlags::Native | ETypeFlags::Object,
        sizeof(PMetaObject) );
}
//----------------------------------------------------------------------------
bool FBaseObjectTraits::DeepEquals(const void* lhs, const void* rhs) const {
    return DeepEqualsObject_(lhs, rhs);
}
//----------------------------------------------------------------------------
void FBaseObjectTraits::DeepCopy(const void* src, void* dst) const {
    DeepCopyObject_(*this, src, dst);
}
//----------------------------------------------------------------------------
bool FBaseObjectTraits::PromoteCopy(const void* src, const FAtom& dst) const {
    return PromoteCopyObject_(*this, src, dst);
}
//----------------------------------------------------------------------------
bool FBaseObjectTraits::PromoteMove(void* src, const FAtom& dst) const {
    return PromoteMoveObject_(*this, src, dst);
}
//----------------------------------------------------------------------------
void* FBaseObjectTraits::Cast(void* data, const PTypeTraits& dst) const {
    return CastObject_(*this, data, dst);
}
//----------------------------------------------------------------------------
bool FBaseObjectTraits::Accept(IAtomVisitor* visitor, void* data) const {
    Assert(data);

    return visitor->Visit(static_cast<const IScalarTraits*>(this), *reinterpret_cast<PMetaObject*>(data));
}
//----------------------------------------------------------------------------
bool FBaseObjectTraits::IsDefaultValue(const void* data) const {
    Assert(data);

    const PMetaObject& pobj = (*reinterpret_cast<const PMetaObject*>(data));
    return(nullptr == pobj);
}
//----------------------------------------------------------------------------
void FBaseObjectTraits::ResetToDefaultValue(void* data) const {
    Assert(data);

    PMetaObject& pobj = (*reinterpret_cast<PMetaObject*>(data));
    pobj.reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DECL_RTTI_NATIVETYPE_ISSUPPORTED(_Name, T, _TypeId) STATIC_ASSERT(TIsSupportedType<T>::value);
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_ISSUPPORTED)
#undef DECL_RTTI_NATIVETYPE_ISSUPPORTED
//----------------------------------------------------------------------------
STATIC_ASSERT(not TIsSupportedType<void>::value);
STATIC_ASSERT(not TIsSupportedType<FAtom>::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
