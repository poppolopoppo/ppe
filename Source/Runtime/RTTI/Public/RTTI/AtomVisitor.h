#pragma once

#include "RTTI.h"

#include "RTTI_fwd.h"
#include "RTTI/NativeTypes.Definitions-inl.h"

#include <iosfwd>

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVisitorFlags : u32 {
    Default             = 0,
    KeepDeprecated      = 1<<0,
    KeepTransient       = 1<<1,
    OnlyObjects         = 1<<2,
};
ENUM_FLAGS(EVisitorFlags);
//----------------------------------------------------------------------------
class IAtomVisitor {
public:
    explicit IAtomVisitor(EVisitorFlags flags = EVisitorFlags::Default) : _flags(flags) {}
    virtual ~IAtomVisitor() {}

    EVisitorFlags Flags() const { return _flags; }

    bool KeepDeprecated() const { return (_flags ^ EVisitorFlags::KeepDeprecated); }
    bool KeepTransient() const { return (_flags ^ EVisitorFlags::KeepTransient); }
    bool OnlyObjects() const { return (_flags ^ EVisitorFlags::OnlyObjects); }

    virtual bool Visit(const ITupleTraits* tuple, void* data) = 0;
    virtual bool Visit(const IListTraits* list, void* data) = 0;
    virtual bool Visit(const IDicoTraits* dico, void* data) = 0;

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* scalar, T& value) = 0;
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

public: // helpers
    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const ITupleTraits* tuple, void* data);
    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const IListTraits* list, void* data);
    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const IDicoTraits* dico, void* data);

    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const IScalarTraits* scalar, FAny& any);
    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const IScalarTraits* scalar, PMetaObject& pobj);

    template <typename T>
    static bool Accept(IAtomVisitor* , const IScalarTraits* , T& ) { return true; }

protected:
    EVisitorFlags _flags;
    bool _keepTransient : 1;
};
//----------------------------------------------------------------------------
class FBaseAtomVisitor : public IAtomVisitor {
public:
    explicit FBaseAtomVisitor(EVisitorFlags flags = EVisitorFlags::Default) : IAtomVisitor(flags) {}

    using IAtomVisitor::Visit;

    virtual bool Visit(const ITupleTraits* tuple, void* data) override { return IAtomVisitor::Accept(this, tuple, data); }
    virtual bool Visit(const IListTraits* list, void* data) override { return IAtomVisitor::Accept(this, list, data); }
    virtual bool Visit(const IDicoTraits* dico, void* data) override { return IAtomVisitor::Accept(this, dico, data); }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* scalar, T& value) override { return IAtomVisitor::Accept(this, scalar, value); }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Used for manipulating visitors and atoms with fwd decl only :
//----------------------------------------------------------------------------
FORCE_INLINE bool AtomVisit(IAtomVisitor& visitor, const ITupleTraits* tuple, void* data) {
    return visitor.Visit(tuple, data);
}
//----------------------------------------------------------------------------
FORCE_INLINE bool AtomVisit(IAtomVisitor& visitor, const IListTraits* list, void* data) {
    return visitor.Visit(list, data);
}
//----------------------------------------------------------------------------
FORCE_INLINE bool AtomVisit(IAtomVisitor& visitor, const IDicoTraits* dico, void* data) {
    return visitor.Visit(dico, data);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE bool AtomVisit(IAtomVisitor& visitor, const IScalarTraits* scalar, T& value) {
    return visitor.Visit(scalar, value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& PrettyPrint(FTextWriter& oss, const FAtom& atom, bool showDefaults = false);
PPE_RTTI_API FWTextWriter& PrettyPrint(FWTextWriter& oss, const FAtom& atom, bool showDefaults = false);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
