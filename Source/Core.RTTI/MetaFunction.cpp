#include "stdafx.h"

#include "MetaFunction.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaFunction::FMetaFunction(const FName& name, EFlags attributes, size_t argCount)
:   _name(name)
,   _data(0) {
    Assert(!_name.empty());

    attributes_type::InplaceSet(_data, u32(attributes));
    argscount_type::InplaceSet(_data, argCount);
}
//----------------------------------------------------------------------------
FMetaFunction::~FMetaFunction() {}
//----------------------------------------------------------------------------
void FMetaFunction::SetOutputFlags_(size_t value) {
    outputflags_type::InplaceSet(_data, value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
