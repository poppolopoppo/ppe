#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaType.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(MetaAtom);
class IMetaAtomPair;
class IMetaAtomVector;
class IMetaAtomDictionary;
FWD_REFPTR(MetaObject);
class MetaProperty;
template <typename T>
class MetaTypedAtom;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMetaAtomVisitor {
public:
    virtual ~IMetaAtomVisitor() {}

    virtual void Visit(IMetaAtomPair* ppair) = 0;
    virtual void Visit(IMetaAtomVector* pvector) = 0;
    virtual void Visit(IMetaAtomDictionary* pdictionary) = 0;

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId) \
    virtual void Visit(MetaTypedAtom<T>* scalar) = 0;

#include "Core.RTTI/MetaType.Definitions-inl.h"
#undef DEF_METATYPE_SCALAR
};
//----------------------------------------------------------------------------
class IMetaAtomConstVisitor {
public:
    virtual ~IMetaAtomConstVisitor() {}

    virtual void Visit(const IMetaAtomPair* ppair) = 0;
    virtual void Visit(const IMetaAtomVector* pvector) = 0;
    virtual void Visit(const IMetaAtomDictionary* pdictionary) = 0;

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId) \
    virtual void Visit(const MetaTypedAtom<T>* scalar) = 0;

#include "Core.RTTI/MetaType.Definitions-inl.h"
#undef DEF_METATYPE_SCALAR
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaAtomWrapMoveVisitor : public IMetaAtomVisitor {
public:
    virtual ~MetaAtomWrapMoveVisitor() {}

    void Append(MetaAtom* atom);

    virtual void Inspect(IMetaAtomPair* ppair, Pair<PMetaAtom, PMetaAtom>& pair);
    virtual void Inspect(IMetaAtomVector* pvector, Vector<PMetaAtom>& vector);
    virtual void Inspect(IMetaAtomDictionary* pdictionary, Dictionary<PMetaAtom, PMetaAtom>& dictionary);

    using IMetaAtomVisitor::Visit;

    virtual void Visit(IMetaAtomPair* ppair) override;
    virtual void Visit(IMetaAtomVector* pvector) override;
    virtual void Visit(IMetaAtomDictionary* pdictionary) override;
    virtual void Visit(MetaTypedAtom<PMetaAtom>* patom) override;
};
//----------------------------------------------------------------------------
class MetaAtomWrapCopyVisitor : public IMetaAtomConstVisitor {
public:
    virtual ~MetaAtomWrapCopyVisitor() {}

    void Append(const MetaAtom* atom);

    virtual void Inspect(const IMetaAtomPair* ppair, const Pair<PMetaAtom, PMetaAtom>& pair);
    virtual void Inspect(const IMetaAtomVector* pvector, const Vector<PMetaAtom>& vector);
    virtual void Inspect(const IMetaAtomDictionary* pdictionary, const Dictionary<PMetaAtom, PMetaAtom>& dictionary);

    using IMetaAtomConstVisitor::Visit;

    virtual void Visit(const IMetaAtomPair* ppair) override;
    virtual void Visit(const IMetaAtomVector* pvector) override;
    virtual void Visit(const IMetaAtomDictionary* pdictionary) override;
    virtual void Visit(const MetaTypedAtom<PMetaAtom>* patom) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI {
} //!namespace Core
