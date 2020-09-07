#pragma once

#include "RTTI_fwd.h"

#include "RTTI/NativeTypes.Definitions-inl.h"
#include "RTTI/TypeTraits.h"

#include <iosfwd>

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVisitorFlags : u32 {
    // /!\ Report changes to MetaEnumHelpers.cpp

    Default             = 0,
    KeepDeprecated      = 1<<0,
    KeepTransient       = 1<<1,
    OnlyObjects         = 1<<2,
    NoRecursion         = 1<<3,
};
ENUM_FLAGS(EVisitorFlags);
//----------------------------------------------------------------------------
class IAtomVisitor {
public:
    explicit IAtomVisitor(EVisitorFlags flags = EVisitorFlags::Default) NOEXCEPT
        : _flags(flags) {}
    virtual ~IAtomVisitor() = default;

    EVisitorFlags Flags() const { return _flags; }
    void SetFlags(EVisitorFlags flags) { _flags = flags; }

    bool KeepDeprecated() const { return (_flags ^ EVisitorFlags::KeepDeprecated); }
    bool KeepTransient() const { return (_flags ^ EVisitorFlags::KeepTransient); }
    bool OnlyObjects() const { return (_flags ^ EVisitorFlags::OnlyObjects); }
    bool NoRecursion() const { return (_flags ^ EVisitorFlags::NoRecursion); }

    virtual bool Visit(const ITupleTraits* tuple, void* data) = 0;
    virtual bool Visit(const IListTraits* list, void* data) = 0;
    virtual bool Visit(const IDicoTraits* dico, void* data) = 0;

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* scalar, T& value) = 0;
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

public: // helpers
    static PPE_RTTI_API bool ShouldSkipTraits(IAtomVisitor* visitor, const ITypeTraits& traits) NOEXCEPT;

    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const ITupleTraits* tuple, void* data);
    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const IListTraits* list, void* data);
    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const IDicoTraits* dico, void* data);

    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const IScalarTraits* scalar, FAny& any);
    static PPE_RTTI_API bool Accept(IAtomVisitor* visitor, const IScalarTraits* scalar, PMetaObject& pobj);

    template <typename T>
    static bool Accept(IAtomVisitor* , const IScalarTraits* , T& ) NOEXCEPT { return true; }

protected:
    EVisitorFlags _flags;
    bool _keepTransient : 1;
};
//----------------------------------------------------------------------------
class FBaseAtomVisitor : public IAtomVisitor {
public:
    explicit FBaseAtomVisitor(EVisitorFlags flags = EVisitorFlags::Default) NOEXCEPT
        : IAtomVisitor(flags) {}

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
enum class EPrettyPrintFlags : u32 {
    None            = 0,
    ShowDefaults    = 1<<0,
    ShowEnumNames   = 1<<1,
    ShowTypeNames   = 1<<2,
    NoRecursion     = 1<<3,

    Minimal         = NoRecursion,
    Full            = ShowDefaults|ShowEnumNames|ShowTypeNames,

    Default         = Minimal|ShowEnumNames
};
ENUM_FLAGS(EPrettyPrintFlags);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& PrettyPrint(FTextWriter& oss, const FAtom& atom, EPrettyPrintFlags flags = EPrettyPrintFlags::Default);
PPE_RTTI_API FWTextWriter& PrettyPrint(FWTextWriter& oss, const FAtom& atom, EPrettyPrintFlags flags = EPrettyPrintFlags::Default);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
