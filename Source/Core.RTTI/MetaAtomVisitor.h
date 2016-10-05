#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core.RTTI/MetaType.Definitions-inl.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(MetaAtom);
class IMetaAtomPair;
class IMetaAtomVector;
class IMetaAtomDictionary;
FWD_REFPTR(MetaObject);
class FMetaProperty;
template <typename T>
class TMetaTypedAtom;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMetaAtomVisitor {
public:
    virtual ~IMetaAtomVisitor() {}

    virtual void Visit(IMetaAtomPair* ppair) = 0;
    virtual void Visit(IMetaAtomVector* pvector) = 0;
    virtual void Visit(IMetaAtomDictionary* pdictionary) = 0;

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) virtual void Visit(TMetaTypedAtom<T>* scalar) = 0;
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR
};
//----------------------------------------------------------------------------
class IMetaAtomConstVisitor {
public:
    virtual ~IMetaAtomConstVisitor() {}

    virtual void Visit(const IMetaAtomPair* ppair) = 0;
    virtual void Visit(const IMetaAtomVector* pvector) = 0;
    virtual void Visit(const IMetaAtomDictionary* pdictionary) = 0;

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) virtual void Visit(const TMetaTypedAtom<T>* scalar) = 0;
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaAtomWrapMoveVisitor : public IMetaAtomVisitor {
public:
    virtual ~FMetaAtomWrapMoveVisitor() {}

    void Append(FMetaAtom* atom);

    virtual void Inspect(IMetaAtomPair* ppair, TPair<PMetaAtom, PMetaAtom>& pair);
    virtual void Inspect(IMetaAtomVector* pvector, TVector<PMetaAtom>& vector);
    virtual void Inspect(IMetaAtomDictionary* pdictionary, TDictionary<PMetaAtom, PMetaAtom>& dictionary);

    using IMetaAtomVisitor::Visit;

    virtual void Visit(IMetaAtomPair* ppair) override;
    virtual void Visit(IMetaAtomVector* pvector) override;
    virtual void Visit(IMetaAtomDictionary* pdictionary) override;
    virtual void Visit(TMetaTypedAtom<PMetaAtom>* patom) override;
};
//----------------------------------------------------------------------------
class FMetaAtomWrapCopyVisitor : public IMetaAtomConstVisitor {
public:
    virtual ~FMetaAtomWrapCopyVisitor() {}

    void Append(const FMetaAtom* atom);

    virtual void Inspect(const IMetaAtomPair* ppair, const TPair<PMetaAtom, PMetaAtom>& pair);
    virtual void Inspect(const IMetaAtomVector* pvector, const TVector<PMetaAtom>& vector);
    virtual void Inspect(const IMetaAtomDictionary* pdictionary, const TDictionary<PMetaAtom, PMetaAtom>& dictionary);

    using IMetaAtomConstVisitor::Visit;

    virtual void Visit(const IMetaAtomPair* ppair) override;
    virtual void Visit(const IMetaAtomVector* pvector) override;
    virtual void Visit(const IMetaAtomDictionary* pdictionary) override;
    virtual void Visit(const TMetaTypedAtom<PMetaAtom>* patom) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI {
} //!namespace Core
