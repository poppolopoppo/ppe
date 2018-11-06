#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Atom.h"
#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"

#include "Container/Tuple.h"
#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/PointerWFlags.h"

namespace PPE {
namespace RTTI {
class FMetaObject;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EParameterFlags : u32 {
    Default     = 0,
    Output      = 1<<0,
    Optional    = 1<<1,

    All         = UINT32_MAX
};
ENUM_FLAGS(EParameterFlags);
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaParameter {
public:
    FMetaParameter();
    FMetaParameter(const FName& name, const PTypeTraits& traits, EParameterFlags flags);

    const FName& Name() const { return _name; }
    PTypeTraits Traits() const { return union_cast_t{ _traitsAndFlags.Get() }.Traits; }
    EParameterFlags Flags() const { return EParameterFlags(_traitsAndFlags.Flag01()); }

    bool IsOutput() const       { return _traitsAndFlags.Flag0(); }
    bool IsOptional() const     { return _traitsAndFlags.Flag1(); }

private:
    union union_cast_t {
        void* Raw;
        PTypeTraits Traits;
        union_cast_t(void* raw) : Raw(raw) {}
        union_cast_t(const PTypeTraits& traits) : Traits(traits) {}
    };

    FName _name;
    Meta::TPointerWFlags<void> _traitsAndFlags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EFunctionFlags : u32 {
    Const       = 1<<0,
    Public      = 1<<1,
    Protected   = 1<<2,
    Private     = 1<<3,
    Deprecated  = 1<<4,

    All         = UINT32_MAX
};
ENUM_FLAGS(EFunctionFlags);
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaFunction {
public:
    typedef void (*invoke_func)(
            const FMetaObject& obj,
            const FAtom& result,
            const TMemoryView<const FAtom>& arguments );

    FMetaFunction();
    FMetaFunction(
        const FName& name,
        EFunctionFlags flags,
        const PTypeTraits& result,
        std::initializer_list<FMetaParameter> parameters,
        invoke_func invoke );
    ~FMetaFunction();

    const FName& Name() const { return _name; }
    EFunctionFlags Flags() const { return _flags; }
    const PTypeTraits& Result() const { return _result; }
    TMemoryView<const FMetaParameter> Parameters() const { return _parameters.MakeConstView(); }

    bool IsConst() const        { return (_flags ^ EFunctionFlags::Const        ); }
    bool IsPublic() const       { return (_flags ^ EFunctionFlags::Public       ); }
    bool IsProtected() const    { return (_flags ^ EFunctionFlags::Protected    ); }
    bool IsPrivate() const      { return (_flags ^ EFunctionFlags::Private      ); }
    bool IsDeprecated() const   { return (_flags ^ EFunctionFlags::Deprecated   ); }

    bool HasReturnValue() const { return (_result.Valid()); }

    void Invoke(
        const FMetaObject& obj,
        const FAtom& result,
        const TMemoryView<const FAtom>& arguments ) const;

private:
    FName _name;
    invoke_func _invoke;
    EFunctionFlags _flags;
    PTypeTraits _result;
    VECTORINSITU(MetaFunction, FMetaParameter, 4) _parameters;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
PPE_ASSUME_TYPE_AS_POD(RTTI::FMetaParameter);
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::EParameterFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EParameterFlags flags);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::EFunctionFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EFunctionFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#undef USE_MSVC17_WORKAROUND_FOR_INTERNALERROR
