#include "stdafx.h"

#include "MetaAtomVisitor.h"

#include "MetaAtom.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FMetaAtomWrapMoveVisitor::Append(FMetaAtom* patom) {
    if (patom)
        patom->Accept(this);
}
//----------------------------------------------------------------------------
void FMetaAtomWrapMoveVisitor::Inspect(IMetaAtomPair* ppair, TPair<PMetaAtom, PMetaAtom>& pair) {
    UNUSED(ppair);

    Append(pair.first.get());
    Append(pair.second.get());
}
//----------------------------------------------------------------------------
void FMetaAtomWrapMoveVisitor::Inspect(IMetaAtomVector* pvector, TVector<PMetaAtom>& vector) {
    UNUSED(pvector);

    for(const PMetaAtom& atom : vector)
        Append(atom.get());
}
//----------------------------------------------------------------------------
void FMetaAtomWrapMoveVisitor::Inspect(IMetaAtomDictionary* pdictionary, TDictionary<PMetaAtom, PMetaAtom>& dictionary) {
    UNUSED(pdictionary);

    for(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair : dictionary) {
        Append(pair.first.get()); // do not call Visit(pair) !
        Append(pair.second.get());
    }
}
//----------------------------------------------------------------------------
void FMetaAtomWrapMoveVisitor::Visit(IMetaAtomPair* ppair) {
    Assert(ppair);
    RTTI::TPair<PMetaAtom, PMetaAtom> pair;
    ppair->WrapMoveTo(pair);
    Inspect(ppair, pair);
    ppair->UnwrapMoveFrom(pair);
}
//----------------------------------------------------------------------------
void FMetaAtomWrapMoveVisitor::Visit(IMetaAtomVector* pvector) {
    Assert(pvector);
    RTTI::TVector<PMetaAtom> vector;
    pvector->WrapMoveTo(vector);
    Inspect(pvector, vector);
    pvector->UnwrapMoveFrom(vector);
}
//----------------------------------------------------------------------------
void FMetaAtomWrapMoveVisitor::Visit(IMetaAtomDictionary* pdictionary) {
    Assert(pdictionary);
    RTTI::TDictionary<PMetaAtom, PMetaAtom> dictionary;
    pdictionary->WrapMoveTo(dictionary);
    Inspect(pdictionary, dictionary);
    pdictionary->UnwrapMoveFrom(dictionary);
}
//----------------------------------------------------------------------------
void FMetaAtomWrapMoveVisitor::Visit(TMetaTypedAtom<PMetaAtom>* patom) {
    Assert(patom);
    Append(patom->Wrapper().get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FMetaAtomWrapCopyVisitor::Append(const FMetaAtom* patom) {
    if (patom)
        patom->Accept(this);
}
//----------------------------------------------------------------------------
void FMetaAtomWrapCopyVisitor::Inspect(const IMetaAtomPair* ppair, const TPair<PMetaAtom, PMetaAtom>& pair) {
    UNUSED(ppair);
    Append(pair.first.get());
    Append(pair.second.get());
}
//----------------------------------------------------------------------------
void FMetaAtomWrapCopyVisitor::Inspect(const IMetaAtomVector* pvector, const TVector<PMetaAtom>& vector) {
    UNUSED(pvector);
    for(const PMetaAtom& atom : vector)
        Append(atom.get());
}
//----------------------------------------------------------------------------
void FMetaAtomWrapCopyVisitor::Inspect(const IMetaAtomDictionary* pdictionary, const TDictionary<PMetaAtom, PMetaAtom>& dictionary) {
    UNUSED(pdictionary);
    for(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair : dictionary) {
        Append(pair.first.get()); // do not call Visit(pair) !
        Append(pair.second.get());
    }
}
//----------------------------------------------------------------------------
void FMetaAtomWrapCopyVisitor::Visit(const IMetaAtomPair* ppair) {
    Assert(ppair);
    RTTI::TPair<PMetaAtom, PMetaAtom> pair;
    ppair->WrapCopyTo(pair);
    Inspect(ppair, pair);
}
//----------------------------------------------------------------------------
void FMetaAtomWrapCopyVisitor::Visit(const IMetaAtomVector* pvector) {
    Assert(pvector);
    RTTI::TVector<PMetaAtom> vector;
    pvector->WrapCopyTo(vector);
    Inspect(pvector, vector);
}
//----------------------------------------------------------------------------
void FMetaAtomWrapCopyVisitor::Visit(const IMetaAtomDictionary* pdictionary) {
    Assert(pdictionary);
    RTTI::TDictionary<PMetaAtom, PMetaAtom> dictionary;
    pdictionary->WrapCopyTo(dictionary);
    Inspect(pdictionary, dictionary);
}
//----------------------------------------------------------------------------
void FMetaAtomWrapCopyVisitor::Visit(const TMetaTypedAtom<PMetaAtom>* patom) {
    Assert(patom);
    Append(patom->Wrapper().get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI {
} //!namespace Core
