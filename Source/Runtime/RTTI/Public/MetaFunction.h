#pragma once

#include "RTTI.h"

#include "Atom.h"
#include "Typedefs.h"
#include "TypeTraits.h"

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
template <typename T>
FMetaParameter MakeParameter(Meta::TType<T>, const FStringView& name) {
    static_assert(not std::is_pointer<T>::value, "pointers are not supported, use non-const references instead");
    return FMetaParameter(
        FName(name),
        MakeTraits<T>(),
        (std::is_lvalue_reference<T>::value && not std::is_const< Meta::TDecay<T> >::value)
            ? EParameterFlags::Output
            : EParameterFlags::Default );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EFunctionFlags : u32 {
    Const       = 1<<0,
    Public      = 1<<1,
    Protected   = 1<<2,
    Private     = 1<<3,
    Deprecated  = 1<<4,
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
template <typename T> struct TMakeFunction {};
//----------------------------------------------------------------------------
template <typename _Result, class _Class, typename... _Args>
struct TMakeFunction<_Result (_Class::*)(_Args...)> {
    template <_Result (_Class::* _Member)(_Args...)>
    static FMetaFunction Make(const FName& name, EFunctionFlags flags, std::initializer_list<FStringView> parametersName) {
        Assert(sizeof...(_Args) == parametersName.size());
        auto nameIt = std::begin(parametersName);
        NOOP(nameIt);
        return FMetaFunction(
            name,
            flags,
            MakeTraits<_Result>(),
            { MakeParameter(Meta::TType<_Args>{}, *nameIt++)... },
            &TMemberFunction_<_Member, typename std::is_void<_Result>::type>::Invoke
        );
    }

private:
    template <_Result (_Class::* _Member)(_Args...), typename T = std::false_type>
    struct TMemberFunction_ {
        static void Invoke(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) {
            const FAtom* parg = arguments.data();
            NOOP(parg);
            result.TypedData<_Result>() = Call(_Member,
                const_cast<_Class*>(RTTI::CastChecked<_Class>(&obj)),
                TTuple<Meta::TReference<_Args>...>{
                    (*parg++).TypedData<Meta::TDecay<_Args>>()...
                });
        }
    };

    template <_Result (_Class::* _Member)(_Args...)>
    struct TMemberFunction_<_Member, std::true_type> {
        static void Invoke(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) {
            Assert(not result);
            const FAtom* parg = arguments.data();
            NOOP(parg);
            Call(_Member,
                const_cast<_Class*>(RTTI::CastChecked<_Class>(&obj)),
                TTuple<Meta::TReference<_Args>...>{
                    (*parg++).TypedData<Meta::TDecay<_Args>>()...
                });
        }
    };
};
//----------------------------------------------------------------------------
template <typename _Result, class _Class, typename... _Args>
struct TMakeFunction<_Result (_Class::*)(_Args...) const> {
    template <_Result(_Class::* _Member)(_Args...) const>
    static FMetaFunction Make(const FName& name, EFunctionFlags flags, std::initializer_list<FStringView> parametersName) {
        Assert(sizeof...(_Args) == parametersName.size());
        auto nameIt = std::begin(parametersName);
        NOOP(nameIt);
        return FMetaFunction(
            name,
            flags,
            MakeTraits<_Result>(),
            { MakeParameter(Meta::TType<_Args>{}, *nameIt++)... },
            &TMemberFunction_<_Member>::Invoke
        );
    }

private:
    template <_Result(_Class::* _Member)(_Args...) const, bool = std::is_void<_Result>::value >
    struct TMemberFunction_ {
        static void Invoke(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) {
            const FAtom* parg = arguments.data();
            NOOP(parg);
            result.TypedData<_Result>() = Call(_Member,
                RTTI::CastChecked<_Class>(&obj),
                TTuple<Meta::TReference<_Args>...>{
                    (*parg++).TypedData<Meta::TDecay<_Args>>()...
                });
        }
    };

    template <_Result(_Class::* _Member)(_Args...) const>
    struct TMemberFunction_<_Member, true> {
        static void Invoke(const FMetaObject& obj, const FAtom& result, const TMemoryView<const FAtom>& arguments) {
            Assert(not result);
            const FAtom* parg = arguments.data();
            NOOP(parg);
            Call(_Member,
                RTTI::CastChecked<_Class>(&obj),
                TTuple<Meta::TReference<_Args>...>{
                    (*parg++).TypedData<Meta::TDecay<_Args>>()...
                });
        }
    };
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
