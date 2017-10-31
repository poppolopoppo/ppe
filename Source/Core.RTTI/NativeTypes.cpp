#include "stdafx.h"

#include "NativeTypes.h"

#include "TypeInfos.h"

#include "AtomVisitor.h" // needed for Accept() & PrettyPrint()
#include "MetaObject.h" // needed for PMetaObject manipulation
#include "MetaObjectHelpers.h" // needed for DeepEquals()

#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TNativeTypeTraits : public TBaseTypeTraits<T, IScalarTraits> {
    using base_traits = TBaseTypeTraits<T, IScalarTraits>;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public:
    // specialized explicitly
    virtual FTypeId TypeId() const override final;
    virtual FTypeInfos TypeInfos() const override final;
    // !specialized explicitly

    virtual bool DeepEquals(const FAtom& lhs, const FAtom& rhs) const override final {
        return base_traits::Equals(lhs, rhs); // deep equals <=> equals for native types
    }

    virtual void DeepCopy(const FAtom& src, const FAtom& dst) const override final {
        base_traits::Copy(src, dst); // deep copy <=> copy for native types
    }

    virtual bool PromoteCopy(const FAtom& from, const FAtom& to) const override final {
        if (from.Traits() == to.Traits()) {
            to.TypedData<T>() = from.TypedConstData<T>();
            return true;
        }
        else {
            return false;
        }
    }

    virtual bool PromoteMove(const FAtom& from, const FAtom& to) const override final {
        if (from.Traits() == to.Traits()) {
            to.TypedData<T>() = std::move(from.TypedData<T>());
            return true;
        }
        else {
            return false;
        }
    }

    virtual bool Accept(IAtomVisitor* visitor, const FAtom& atom) const override final {
        return visitor->Visit(static_cast<const IScalarTraits*>(this), atom.TypedData<T>());
    }

    virtual void Format(std::basic_ostream<char>& oss, const FAtom& atom) const override final {
        base_traits::Format(oss, atom);
    }

    virtual void Format(std::basic_ostream<wchar_t>& oss, const FAtom& atom) const override final {
        base_traits::Format(oss, atom);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// PMetaObject specializations
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::DeepEquals(const FAtom& lhs, const FAtom& rhs) const {
    const PMetaObject& lhsObj = lhs.TypedConstData<PMetaObject>();
    const PMetaObject& rhsObj = rhs.TypedConstData<PMetaObject>();

    if (lhsObj == rhsObj)
        return true;
    else if (!lhsObj || !rhsObj)
        return false;
    else
        return RTTI::DeepEquals(*lhsObj, *rhsObj);
}
//----------------------------------------------------------------------------
template <>
void TNativeTypeTraits<PMetaObject>::DeepCopy(const FAtom& src, const FAtom& dst) const {
    const PMetaObject& srcObj = src.TypedConstData<PMetaObject>();
    const PMetaObject& dstObj = dst.TypedConstData<PMetaObject>();

    if (srcObj != dstObj)
        RTTI::DeepCopy(*srcObj, *dstObj);
}
//----------------------------------------------------------------------------
template <>
void TNativeTypeTraits<PMetaObject>::Format(std::basic_ostream<char>& oss, const FAtom& atom) const override final {
    PrettyPrint(oss, atom);
}
//----------------------------------------------------------------------------
template <>
void TNativeTypeTraits<PMetaObject>::Format(std::basic_ostream<wchar_t>& oss, const FAtom& atom) const override final {
    PrettyPrint(oss, atom);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
} //!namespace RTTI
} //!namespace Core
