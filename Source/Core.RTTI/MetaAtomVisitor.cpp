#include "stdafx.h"

#include "MetaAtomVisitor.h"

#include "MetaAtom.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void MetaAtomWrapMoveVisitor::Append(MetaAtom* patom) {
    if (patom)
        patom->Accept(this);
}
//----------------------------------------------------------------------------
void MetaAtomWrapMoveVisitor::Inspect(IMetaAtomPair* ppair, Pair<PMetaAtom, PMetaAtom>& pair) {
    UNUSED(ppair);

    Append(pair.first.get());
    Append(pair.second.get());
}
//----------------------------------------------------------------------------
void MetaAtomWrapMoveVisitor::Inspect(IMetaAtomVector* pvector, Vector<PMetaAtom>& vector) {
    UNUSED(pvector);

    for(const PMetaAtom& atom : vector)
        Append(atom.get());
}
//----------------------------------------------------------------------------
void MetaAtomWrapMoveVisitor::Inspect(IMetaAtomDictionary* pdictionary, Dictionary<PMetaAtom, PMetaAtom>& dictionary) {
    UNUSED(pdictionary);

    for(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair : dictionary) {
        Append(pair.first.get()); // do not call Visit(pair) !
        Append(pair.second.get());
    }
}
//----------------------------------------------------------------------------
void MetaAtomWrapMoveVisitor::Visit(IMetaAtomPair* ppair) {
    Assert(ppair);
    RTTI::Pair<PMetaAtom, PMetaAtom> pair;
    ppair->WrapMoveTo(pair);
    Inspect(ppair, pair);
    ppair->UnwrapMoveFrom(pair);
}
//----------------------------------------------------------------------------
void MetaAtomWrapMoveVisitor::Visit(IMetaAtomVector* pvector) {
    Assert(pvector);
    RTTI::Vector<PMetaAtom> vector;
    pvector->WrapMoveTo(vector);
    Inspect(pvector, vector);
    pvector->UnwrapMoveFrom(vector);
}
//----------------------------------------------------------------------------
void MetaAtomWrapMoveVisitor::Visit(IMetaAtomDictionary* pdictionary) {
    Assert(pdictionary);
    RTTI::Dictionary<PMetaAtom, PMetaAtom> dictionary;
    pdictionary->WrapMoveTo(dictionary);
    Inspect(pdictionary, dictionary);
    pdictionary->UnwrapMoveFrom(dictionary);
}
//----------------------------------------------------------------------------
void MetaAtomWrapMoveVisitor::Visit(MetaTypedAtom<PMetaAtom>* patom) {
    Assert(patom);
    Append(patom->Wrapper().get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void MetaAtomWrapCopyVisitor::Append(const MetaAtom* patom) {
    if (patom)
        patom->Accept(this);
}
//----------------------------------------------------------------------------
void MetaAtomWrapCopyVisitor::Inspect(const IMetaAtomPair* ppair, const Pair<PMetaAtom, PMetaAtom>& pair) {
    UNUSED(ppair);
    Append(pair.first.get());
    Append(pair.second.get());
}
//----------------------------------------------------------------------------
void MetaAtomWrapCopyVisitor::Inspect(const IMetaAtomVector* pvector, const Vector<PMetaAtom>& vector) {
    UNUSED(pvector);
    for(const PMetaAtom& atom : vector)
        Append(atom.get());
}
//----------------------------------------------------------------------------
void MetaAtomWrapCopyVisitor::Inspect(const IMetaAtomDictionary* pdictionary, const Dictionary<PMetaAtom, PMetaAtom>& dictionary) {
    UNUSED(pdictionary);
    for(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair : dictionary) {
        Append(pair.first.get()); // do not call Visit(pair) !
        Append(pair.second.get());
    }
}
//----------------------------------------------------------------------------
void MetaAtomWrapCopyVisitor::Visit(const IMetaAtomPair* ppair) {
    Assert(ppair);
    RTTI::Pair<PMetaAtom, PMetaAtom> pair;
    ppair->WrapCopyTo(pair);
    Inspect(ppair, pair);
}
//----------------------------------------------------------------------------
void MetaAtomWrapCopyVisitor::Visit(const IMetaAtomVector* pvector) {
    Assert(pvector);
    RTTI::Vector<PMetaAtom> vector;
    pvector->WrapCopyTo(vector);
    Inspect(pvector, vector);
}
//----------------------------------------------------------------------------
void MetaAtomWrapCopyVisitor::Visit(const IMetaAtomDictionary* pdictionary) {
    Assert(pdictionary);
    RTTI::Dictionary<PMetaAtom, PMetaAtom> dictionary;
    pdictionary->WrapCopyTo(dictionary);
    Inspect(pdictionary, dictionary);
}
//----------------------------------------------------------------------------
void MetaAtomWrapCopyVisitor::Visit(const MetaTypedAtom<PMetaAtom>* patom) {
    Assert(patom);
    Append(patom->Wrapper().get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI {
} //!namespace Core