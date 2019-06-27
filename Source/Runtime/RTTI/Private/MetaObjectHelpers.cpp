#include "stdafx.h"

#include "MetaObjectHelpers.h"

#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/ReferenceCollector.h"
#include "RTTI/TypeTraits.h"

#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "Container/Hash.h"
#include "Container/HashSet.h"
#include "Container/Stack.h"
#include "Container/Vector.h"
#include "Memory/HashFunctions.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "RTTI/Exceptions.h"

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Equals(const FMetaObject& lhs, const FMetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);

    if (rhs.RTTI_Class() != metaClass)
        return false;

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        if (not lhsValue.Equals(rhsValue))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool DeepEquals(const FMetaObject& lhs, const FMetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);

    if (rhs.RTTI_Class() != metaClass)
        return false;

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        if (not lhsValue.DeepEquals(rhsValue))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Move(dstValue);
    }
}
//----------------------------------------------------------------------------
void Copy(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Copy(dstValue);
    }
}
//----------------------------------------------------------------------------
void Swap(FMetaObject& lhs, FMetaObject& rhs) {
    if (&lhs == &rhs)
        return;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);
    Assert(rhs.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        lhsValue.SwapValue(rhsValue);
    }
}
//----------------------------------------------------------------------------
void Clone(const FMetaObject& src, PMetaObject& pdst) {
    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);

    if (not metaClass->CreateInstance(pdst))
        AssertNotReached();

    FMetaObject& dst = *pdst;
    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Copy(dstValue);
    }
}
//----------------------------------------------------------------------------
void DeepCopy(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.DeepCopy(dstValue);
    }
}
//----------------------------------------------------------------------------
void DeepClone(const FMetaObject& src, PMetaObject& pdst) {
    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);

    if (not metaClass->CreateInstance(pdst))
        AssertNotReached();

    FMetaObject& dst = *pdst;
    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.DeepCopy(dstValue);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ResetToDefaultValue(FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties())
        prop->ResetToDefaultValue(obj);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    hash_t h(PPE_HASH_VALUE_SEED);
    for (const FMetaProperty* prop : metaClass->AllProperties())
        hash_combine(h, prop->Get(obj));

    return h;
}
//----------------------------------------------------------------------------
u128 Fingerprint128(const FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    STACKLOCAL_POD_STACK(hash_t, hashValues, metaClass->NumProperties(true));
    for (const FMetaProperty* prop : metaClass->AllProperties())
        hashValues.Push(prop->Get(obj).HashValue());

    return PPE::Fingerprint128(hashValues.MakeConstView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RTTI_CHECKS
void CheckMetaClassAllocation(const FMetaClass* metaClass) {
    if (nullptr == metaClass) {
        LOG(RTTI, Error, L"trying to allocate with a null metaclass");
        PPE_THROW_IT(FClassException("null metaclass", metaClass));
    }

    if (metaClass->IsAbstract()) {
        LOG(RTTI, Error, L"can't allocate a new object with abstract metaclass \"{0}\" ({1})",
            metaClass->Name(),
            metaClass->Flags());
        PPE_THROW_IT(FClassException("abstract metaclass", metaClass));
    }

    if (metaClass->IsDeprecated()) {
        LOG(RTTI, Warning, L"allocate a new object using deprecated metaclass \"{0}\" ({1})",
            metaClass->Name(),
            metaClass->Flags() );
    }
}
#endif //!USE_PPE_RTTI_CHECKS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t CollectReferences(
    const FMetaObject& root,
    TFunction<bool(const IScalarTraits&, FMetaObject&)>&& prefix,
    TFunction<bool(const IScalarTraits&, FMetaObject&)>&& postfix,
    EVisitorFlags flags) {
    FLambdaReferenceCollector collector{ flags };
    collector.Collect(root, std::move(prefix), std::move(postfix));
    return collector.NumReferences();
}
//----------------------------------------------------------------------------
size_t CollectReferences(
    const TMemoryView<const PMetaObject>& roots,
    TFunction<bool(const IScalarTraits&, FMetaObject&)>&& prefix,
    TFunction<bool(const IScalarTraits&, FMetaObject&)>&& postfix,
    EVisitorFlags flags) {
    FLambdaReferenceCollector collector{ flags };
    collector.Collect(roots, std::move(prefix), std::move(postfix));
    return collector.NumReferences();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FCheckCircularReferences_ : private FBaseReferenceCollector {
public:
    FCheckCircularReferences_() NOEXCEPT
    :   FBaseReferenceCollector(
            EVisitorFlags::KeepDeprecated +
            EVisitorFlags::KeepTransient )
    ,   _circular(false)
    {}

    bool Check(const FMetaObject& root) {
        _circular = false;
        FBaseReferenceCollector::Collect(root);
        Assert_NoAssume(_chain.empty());
        return (not _circular);
    }

    bool Check(const TMemoryView<const PMetaObject>& roots) {
        _circular = false;
        FBaseReferenceCollector::Collect(roots);
        Assert_NoAssume(_chain.empty());
        return (not _circular);
    }

protected:
    virtual bool Visit(const IScalarTraits* scalar, PMetaObject& pobj) override final {
        bool result = true;
        if (const FMetaObject * const ref = pobj.get()) {
            const bool circular = Contains(_chain, ref);
            _chain.push_back(ref);

            if (circular)
                OnCircularReference_();
            else
                result = FBaseReferenceCollector::Visit(scalar, pobj);

            Verify(_chain.pop_back_ReturnBack() == ref);
        }
        return result;
    }

private:
    bool _circular;
    VECTORINSITU(MetaObject, const FMetaObject*, 8) _chain;

    void OnCircularReference_() {
        _circular = true;

#if USE_PPE_LOGGER
        FWStringBuilder oss;
        oss << L"found a circular reference !" << Eol;

        Fmt::FWIndent indent = Fmt::FWIndent::UsingTabs();
        forrange(i, 0, _chain.size()) {
            const FMetaObject* const ref = _chain[i];
            const FMetaClass* const metaClass = ref->RTTI_Class();

            Format(oss, L"[{0:#2}]", i);
            oss << indent;

            if (i + 1 == _chain.size())
                oss << L" <=- ";
            else if (ref == _chain.back())
                oss << L" -=> ";
            else
                oss << L" --- ";

            oss << Fmt::Pointer(ref)
                << L" \"" << ref->RTTI_Name()
                << L"\" : " << metaClass->Name()
                << L" (" << metaClass->Flags()
                << L")" << Eol;

            indent.Inc();
        }

        FLogger::Log(
            GLogCategory_RTTI,
            FLogger::EVerbosity::Error,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString() );

#else
        AssertNotReached();
#endif
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool CheckCircularReferences(const FMetaObject& root) {
    FCheckCircularReferences_ circular;
    return circular.Check(root);
}
//----------------------------------------------------------------------------
bool CheckCircularReferences(const TMemoryView<const PMetaObject>& roots) {
    FCheckCircularReferences_ circular;
    return circular.Check(roots);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FMetaObject& obj) {
    RTTI::PMetaObject pobj(const_cast<RTTI::FMetaObject*>(&obj));
    PrettyPrint(oss, RTTI::FAtom::FromObj(pobj));
    RemoveRef_AssertAlive(pobj);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaObject& obj) {
    RTTI::PMetaObject pobj(const_cast<RTTI::FMetaObject*>(&obj));
    PrettyPrint(oss, RTTI::FAtom::FromObj(pobj));
    RemoveRef_AssertAlive(pobj);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
