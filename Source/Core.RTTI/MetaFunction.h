#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypeTraits.h"
#include "Core.RTTI/MetaTypeVirtualTraits.h"
#include "Core.RTTI/Typedefs.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/BitSet.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
FWD_UNIQUEPTR(MetaFunction);
class FMetaFunction {
public:
    enum EFlags {
        Public       = 1<<0,
        Protected    = 1<<1,
        Private      = 1<<2,
        Const        = 1<<3,
        Deprecated   = 1<<4,
    };

    FMetaFunction(const FName& name, EFlags attributes, size_t argCount);
    virtual ~FMetaFunction();

    FMetaFunction(const FMetaFunction&) = delete;
    FMetaFunction& operator =(const FMetaFunction&) = delete;

    const FName& Name() const { return _name; }
    size_t ArgCount()   const { return argscount_type::Get(_data); }
    EFlags Attributes() const { return EFlags(attributes_type::Get(_data)); }
    FWordBitSet OutputFlags() const { return FWordBitSet(outputflags_type::Get(_data), ArgCount()); }

    bool IsPublic()     const { return Meta::HasFlag(Attributes(), Public); }
    bool IsProtected()  const { return Meta::HasFlag(Attributes(), Protected); }
    bool IsPrivate()    const { return Meta::HasFlag(Attributes(), Private); }
    bool IsConst()      const { return Meta::HasFlag(Attributes(), Const); }
    bool IsDeprecated() const { return Meta::HasFlag(Attributes(), Deprecated); }

    bool IsOutput(size_t argIndex) const {
        Assert(argIndex < ArgCount());
        return (outputflags_type::Get(_data) & (1<<argIndex));
    }

    virtual TMemoryView<const FMetaTypeInfo> SignatureInfos() const = 0;
    virtual TMemoryView<const IMetaTypeVirtualTraits* const> SignatureTraits() const = 0;

    virtual bool Invoke(FMetaObject* src, PMetaAtom& presult, const TMemoryView<const PMetaAtom>& args) const = 0;

protected:
    void SetOutputFlags_(size_t value);

private:
    typedef Meta::TBit<u32>::TFirst<5>::type attributes_type;
    typedef Meta::TBit<u32>::TAfter<attributes_type>::TField<4>::type argscount_type;
    typedef Meta::TBit<u32>::TAfter<argscount_type>::TField<15>::type outputflags_type;

    FName _name;
    u32 _data;

public:
    STATIC_CONST_INTEGRAL(size_t, MaxArgCount, argscount_type::MaxValue);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Result, typename... _Args>
class TMetaFunctionImpl : public FMetaFunction {
public:
    TMetaFunctionImpl(const FName& name, EFlags attributes, size_t argCount)
        : FMetaFunction(name, attributes, argCount) {}

    virtual TMemoryView<const FMetaTypeInfo> SignatureInfos() const override final;
    virtual TMemoryView<const IMetaTypeVirtualTraits* const> SignatureTraits() const override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Result, typename _Class, typename... _Args>
class TMetaTypedFunction : public TMetaFunctionImpl<
    typename TMetaTypeTraits< Meta::TDecay<_Result> >::wrapper_type,
    typename TMetaTypeTraits< Meta::TDecay<_Args>   >::wrapper_type...
> {
public:
    typedef TMetaFunctionImpl<
        typename TMetaTypeTraits< Meta::TDecay<_Result> >::wrapper_type,
        typename TMetaTypeTraits< Meta::TDecay<_Args>   >::wrapper_type...
    >   parent_type;

    typedef _Result (_Class::* func_type)(_Args...);

    TMetaTypedFunction(const FName& name, EFlags attributes, func_type func);

    virtual bool Invoke(FMetaObject* src, PMetaAtom& presult, const TMemoryView<const PMetaAtom>& args) const override final;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    func_type _func;
};
//----------------------------------------------------------------------------
template <typename _Result, typename _Class, typename... _Args>
TMetaTypedFunction<_Result, _Class, _Args...>* MakeFunction(
    const FName& name, FMetaFunction::EFlags attributes, _Result(_Class::* func)(_Args...)) {
    return new TMetaTypedFunction<_Result, _Class, _Args...>(name, attributes, func);
}
//----------------------------------------------------------------------------
template <typename _Result, typename _Class, typename... _Args>
TMetaTypedFunction<_Result, _Class, _Args...>* MakeFunction(
    const FName& name, FMetaFunction::EFlags attributes, _Result(_Class::* func_const)(_Args...) const) {
    typedef _Result(_Class::* func_type)(_Args...);
    return new TMetaTypedFunction<_Result, _Class, _Args...>(name, attributes, reinterpret_cast<func_type>(func_const));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaFunction-inl.h"
