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

    virtual bool Visit(const IPairTraits* traits, const FAtom& atom) = 0;
    virtual bool Visit(const IListTraits* traits, const FAtom& atom) = 0;
    virtual bool Visit(const IDicoTraits* traits, const FAtom& atom) = 0;

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const IScalarTraits* traits, T& value) = 0;
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT
};
//----------------------------------------------------------------------------
class CORE_RTTI_API FBaseAtomVisitor : public IAtomVisitor {
public:
    virtual bool Visit(const IPairTraits* traits, const FAtom& atom) override;
    virtual bool Visit(const IListTraits* traits, const FAtom& atom) override;
    virtual bool Visit(const IDicoTraits* traits, const FAtom& atom) override;
    virtual bool Visit(const IScalarTraits* traits, PMetaObject& pobj) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API std::basic_ostream<char>& PrettyPrint(std::basic_ostream<char>& oss, const FAtom& atom);
CORE_RTTI_API std::basic_ostream<wchar_t>& PrettyPrint(std::basic_ostream<wchar_t>& oss, const FAtom& atom);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core