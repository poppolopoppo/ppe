#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/RTTI_fwd.h"
#include "Core.RTTI/NativeTypes.Definitions-inl.h"

#include <iosfwd>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IAtomVisitor {
public:
    virtual ~IAtomVisitor() {}

    virtual bool Visit(const IPairTraits* pair, const FAtom& atom) = 0;
    virtual bool Visit(const IListTraits* list, const FAtom& atom) = 0;
    virtual bool Visit(const IDicoTraits* dico, const FAtom& atom) = 0;

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* scalar, T& value) = 0;
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

public: // helpers
    static CORE_RTTI_API bool Accept(IAtomVisitor* visitor, const IPairTraits* pair, const FAtom& atom);
    static CORE_RTTI_API bool Accept(IAtomVisitor* visitor, const IListTraits* list, const FAtom& atom);
    static CORE_RTTI_API bool Accept(IAtomVisitor* visitor, const IDicoTraits* dico, const FAtom& atom);

    static CORE_RTTI_API bool Accept(IAtomVisitor* visitor, const IScalarTraits* scalar, FAny& any);
    static CORE_RTTI_API bool Accept(IAtomVisitor* visitor, const IScalarTraits* scalar, PMetaObject& pobj);

    template <typename T>
    static bool Accept(IAtomVisitor* , const IScalarTraits* , T& ) { return true; }
};
//----------------------------------------------------------------------------
class FBaseAtomVisitor : public IAtomVisitor {
public:
    using IAtomVisitor::Visit;

    virtual bool Visit(const IPairTraits* pair, const FAtom& atom) override { return IAtomVisitor::Accept(this, pair, atom); }
    virtual bool Visit(const IListTraits* list, const FAtom& atom) override { return IAtomVisitor::Accept(this, list, atom); }
    virtual bool Visit(const IDicoTraits* dico, const FAtom& atom) override { return IAtomVisitor::Accept(this, dico, atom); }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* scalar, T& value) override { return IAtomVisitor::Accept(this, scalar, value); }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API FTextWriter& PrettyPrint(FTextWriter& oss, const FAtom& atom);
CORE_RTTI_API FWTextWriter& PrettyPrint(FWTextWriter& oss, const FAtom& atom);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
