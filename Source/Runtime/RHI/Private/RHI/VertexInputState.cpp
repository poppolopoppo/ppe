﻿#include "stdafx.h"

#include "RHI/VertexInputState.h"

#if USE_PPE_RHIDEBUG
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "RHI/EnumToString.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI);
}
}
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EVertexFormat FVertexInput::DestinationFormat() const {
    const EVertexFormat scalarType = BitAnd(Format, EVertexFormat::_TypeMask);
    const EVertexFormat vectorSize = BitAnd(Format, EVertexFormat::_VecMask);

    const bool isFloatingPoint =  (Format & EVertexFormat::NormalizedFlag)|(Format & EVertexFormat::ScaledFlag);
    const EVertexFormat floatType = BitOr(EVertexFormat::_Float, vectorSize);

    switch (scalarType) {
    case EVertexFormat::_Byte:
    case EVertexFormat::_Short:
    case EVertexFormat::_Int:
        return isFloatingPoint ? floatType : BitOr(EVertexFormat::_Int, vectorSize);

    case EVertexFormat::_UByte:
    case EVertexFormat::_UShort:
    case EVertexFormat::_UInt:
        return isFloatingPoint ? floatType : BitOr(EVertexFormat::_UInt, vectorSize);

    case EVertexFormat::_Long:
        return isFloatingPoint ? floatType : BitOr(EVertexFormat::_Long, vectorSize);
    case EVertexFormat::_ULong:
        return isFloatingPoint ? floatType : BitOr(EVertexFormat::_ULong, vectorSize);

    case EVertexFormat::_Half:
    case EVertexFormat::_Float:
        return floatType;

    case EVertexFormat::_Double:
        return BitOr(EVertexFormat::_Double, vectorSize);

    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
FVertexInputState& FVertexInputState::Add(const FVertexID& vertexId, EVertexFormat fmt, u32 offset, const FVertexBufferID& bufferId) {
    Assert(vertexId.Valid());
    Assert(bufferId.Valid());

    const FVertexBufferBinding& binding = Bindings.Get(bufferId);
    Vertices.Add_Overwrite(vertexId, FVertexInput{ fmt, offset, binding.Index });

    return (*this);
}
//----------------------------------------------------------------------------
FVertexInputState& FVertexInputState::Bind(const FVertexBufferID& bufferId, u32 stride, u32 index, EVertexInputRate rate) {
    Assert(bufferId.Valid());

    if (AutoBindingIndex == index)
        index = checked_cast<u32>(Bindings.size());

    Bindings.Add_Overwrite(bufferId, FVertexBufferBinding{ index, stride, rate });

    return (*this);
}
//----------------------------------------------------------------------------
bool FVertexInputState::CopyAttributes(const TMemoryView<const FVertexAttribute> attribs) {
    Assert(attribs.size() == Vertices.size());

    for (const FVertexAttribute& attr: attribs) {
        Assert(attr.Id.Valid());
        Assert_NoAssume(attr.Index != UMax);

        FVertexInput* const pinput = Vertices.GetIFP(attr.Id);
        if (nullptr == pinput) {
#if USE_PPE_RHIDEBUG
            LOG(RHI, Error,
                L"can't find vertex attribute called {1} at index {0}",
                attr.Index, attr.Id );
#endif
            return false;
        }

        pinput->Index = attr.Index;

#if USE_PPE_RHIDEBUG
        const EVertexFormat decl = pinput->DestinationFormat();
        if (attr.Format != decl) {
            LOG(RHI, Error,
                L"vertex attribute at index {0} called {1} mismatch:\n"
                L"    - in shader: {2}\n"
                L"    - in declaration: {3}",
                attr.Index, attr.Id, attr.Format,
                decl );
            return false;
        }
#endif
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE