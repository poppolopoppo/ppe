#include "stdafx.h"

#include "RTTI/ReferenceCollector.h"

#include "MetaObject.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBaseReferenceCollector::FBaseReferenceCollector() NOEXCEPT
:   FBaseReferenceCollector(Default)
{}
//----------------------------------------------------------------------------
FBaseReferenceCollector::FBaseReferenceCollector(EVisitorFlags flags) NOEXCEPT
:   FBaseAtomVisitor(flags + EVisitorFlags::OnlyObjects)
{}
//----------------------------------------------------------------------------
void FBaseReferenceCollector::Collect(const FMetaObject& root) {
    PMetaObject pobj{ const_cast<FMetaObject*>(&root) };
    FAtom::FromObj(pobj).Accept(this);
    RemoveRef_AssertAlive(pobj);
}
//----------------------------------------------------------------------------
void FBaseReferenceCollector::Collect(const TMemoryView<const PMetaObject>& roots) {
    Assert_NoAssume(not roots.empty());

    for (const PMetaObject& pobj : roots) {
        Assert(pobj);
        FAtom::FromObj(pobj).Accept(this);
    }
}
//----------------------------------------------------------------------------
bool FBaseReferenceCollector::Visit(const IScalarTraits* scalar, PMetaObject& pobj) {
    if (const FMetaObject* o = pobj.get()) {
        ++_numReferences;
        _objectFlags = _objectFlags + o->RTTI_Flags();
    }
    return FBaseAtomVisitor::Visit(scalar, pobj);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDirectReferenceCollector::FDirectReferenceCollector() NOEXCEPT
:   FBaseReferenceCollector(EVisitorFlags::NoRecursion)
{}
//----------------------------------------------------------------------------
FDirectReferenceCollector::FDirectReferenceCollector(EVisitorFlags flags) NOEXCEPT
:   FBaseReferenceCollector(flags + EVisitorFlags::NoRecursion)
{}
//----------------------------------------------------------------------------
void FDirectReferenceCollector::Collect(const FMetaObject& root, FReferences* refs) {
    Assert(refs);
    Assert_NoAssume(nullptr == _refs);

    _refs = refs;

    FBaseReferenceCollector::Collect(root);

    _refs = nullptr;
}
//----------------------------------------------------------------------------
void FDirectReferenceCollector::Collect(const TMemoryView<const PMetaObject>& roots, FReferences* refs) {
    Assert(refs);
    Assert_NoAssume(nullptr == _refs);

    refs->Reserve(refs->size() + roots.size());

    _refs = refs;

    FBaseReferenceCollector::Collect(roots);

    _refs = nullptr;
}
//----------------------------------------------------------------------------
bool FDirectReferenceCollector::Visit(const IScalarTraits* scalar, PMetaObject& pobj) {
    if (pobj) {
        Add_Unique(*_refs, SMetaObject{ pobj });
        return FBaseReferenceCollector::Visit(scalar, pobj);
    }
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLambdaReferenceCollector::Collect(const FMetaObject& root, FOnReference&& prefix, FOnReference&& postfix /* = NoFunction */) {
    Assert((!!prefix) || (!!postfix));
    Assert_NoAssume(not _prefix);
    Assert_NoAssume(not _postfix);

    _prefix = std::move(prefix);
    _postfix = std::move(postfix);

    FBaseReferenceCollector::Collect(root);

    _prefix.Reset();
    _postfix.Reset();
}
//----------------------------------------------------------------------------
void FLambdaReferenceCollector::Collect(const TMemoryView<const PMetaObject>& roots, FOnReference&& prefix, FOnReference&& postfix /* = NoFunction */) {
    Assert((!!prefix) || (!!postfix));
    Assert_NoAssume(not _prefix);
    Assert_NoAssume(not _postfix);

    _prefix = std::move(prefix);
    _postfix = std::move(postfix);

    FBaseReferenceCollector::Collect(roots);

    _prefix.Reset();
    _postfix.Reset();
}
//----------------------------------------------------------------------------
bool FLambdaReferenceCollector::Visit(const IScalarTraits* scalar, PMetaObject& pobj) {
    bool result = true;

    if (pobj) {
        // doesn't call _postfix() if _prefix() failed !
        if (not _prefix || _prefix(*scalar, *pobj)) {
            result = FBaseReferenceCollector::Visit(scalar, pobj);

            if (_postfix && not _postfix(*scalar, *pobj))
                result = false;
        }
    }

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
