#include "stdafx.h"

#include "MetaObjectHelpers.h"

#include "Atom.h"
#include "AtomVisitor.h"
#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "NativeTypes.h"
#include "TypeTraits.h"

#include "Container/Hash.h"
#include "Container/Stack.h"
#include "Memory/HashFunctions.h"

namespace PPE {
namespace RTTI {
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
namespace {
//----------------------------------------------------------------------------
class FReferenceCollector_ : FBaseAtomVisitor {
public:
    FReferenceCollector_(size_t maxDepth)
        : _maxDepth(maxDepth), _depth(0), _references(nullptr)
    {}

    void Collect(const FMetaObject& root, FReferencedObjects& references) {
        Assert(0 == _depth);
        Assert(nullptr == _references);

        _references = &references;

        PMetaObject src(const_cast<FMetaObject*>(&root));

        if (not InplaceAtom(src).Accept(static_cast<FBaseAtomVisitor*>(this)))
            AssertNotReached();

        Assert(0 == _depth);

        _references = nullptr;
        RemoveRef_AssertAlive(src);
    }

private:
    const size_t _maxDepth;
    size_t _depth;
    FReferencedObjects* _references;

    virtual bool Visit(const IScalarTraits* scalar, PMetaObject& pobj) override final {
        if (pobj) {
            if (not Contains(*_references, pobj.get())) {

                _references->push_back(pobj.get());

                ++_depth;

                if (_depth <= _maxDepth)
                    FBaseAtomVisitor::Visit(scalar, pobj); // visits recursively

                Assert(_depth);
                --_depth;
            }
        }
        return true;
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
void CollectReferencedObjects(const FMetaObject& root, FReferencedObjects& references, size_t maxDepth/* = INDEX_NONE */) {
    Assert(references.empty());
    FReferenceCollector_ collector(maxDepth);
    collector.Collect(root, references);
    Assert(references.front() == &root);
    references.erase_DontPreserveOrder(references.begin());
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
    PrettyPrint(oss, InplaceAtom(pobj));
    RemoveRef_AssertReachZero_KeepAlive(pobj);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaObject& obj) {
    RTTI::PMetaObject pobj(const_cast<RTTI::FMetaObject*>(&obj));
    PrettyPrint(oss, InplaceAtom(pobj));
    RemoveRef_AssertReachZero_KeepAlive(pobj);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE