#pragma once

#include "RHI_fwd.h"

#include "RHI/Config.h"
#include "RHI/ResourceId.h"
#include "RHI/VertexDesc.h"
#include "RHI/VertexEnums.h"
#include "RHI/VertexInputState.h"

#include "Container/FixedSizeHashTable.h"
#include "Container/FlatMap.h"
#include "Container/FlatSet.h"
#include "Container/Hash.h"
#include "Maths/PackingHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVertexAttribute {
    FVertexID Id;
    u32 Index{UMax};
    EVertexFormat Format{Default};

    bool operator ==(const FVertexAttribute& other) const NOEXCEPT {
        return (Id == other.Id && Index == other.Index && Format == other.Format);
    }
    bool operator !=(const FVertexAttribute& other) const NOEXCEPT { return (not operator ==(other)); }

    friend hash_t hash_value(const FVertexAttribute& attr) NOEXCEPT {
        return hash_tuple(attr.Id, attr.Index, attr.Format);
    }

};
PPE_ASSUME_TYPE_AS_POD(FVertexAttribute);
//----------------------------------------------------------------------------
struct FVertexInput {
    EVertexFormat Format{Default};
    u32 Index{UMax};
    u32 Offset{UMax};
    u32 BufferBinding{UMax};

    FVertexInput() = default;
    FVertexInput(EVertexFormat fmt, u32 offset, u32 bufferBinding) NOEXCEPT
        : Format(fmt), Offset(offset), BufferBinding(bufferBinding) {}

    PPE_RHI_API EVertexFormat DestinationFormat() const;

    bool operator ==(const FVertexInput& other) const NOEXCEPT {
        Assert(Index != UMax);
        Assert(other.Index != UMax);
        return (Format == other.Format && Index == other.Index && Offset == other.Offset && BufferBinding == other.BufferBinding);
    }
    bool operator !=(const FVertexInput& other) const NOEXCEPT { return (not operator ==(other)); }

    friend hash_t hash_value(const FVertexInput& input) NOEXCEPT {
        return hash_tuple(input.Format, input.Index, input.Offset, input.Format);
    }

};
PPE_ASSUME_TYPE_AS_POD(FVertexInput);
//----------------------------------------------------------------------------
struct FVertexBufferBinding {
    u32 Index{UMax};
    u32 Stride{UMax};
    EVertexInputRate Rate{Default};

    FVertexBufferBinding() = default;
    FVertexBufferBinding(u32 index, u32 stride, EVertexInputRate rate) NOEXCEPT
        : Index(index), Stride(stride), Rate(rate) {}

    bool operator ==(const FVertexBufferBinding& other) const NOEXCEPT {
        return (Index == other.Index && Stride == other.Stride && Rate == other.Rate);
    }
    bool operator !=(const FVertexBufferBinding& other) const NOEXCEPT { return (not operator ==(other)); }

    friend hash_t hash_value(const FVertexBufferBinding& binding) NOEXCEPT {
        return hash_tuple(binding.Index, binding.Stride, binding.Rate);
    }

};
PPE_ASSUME_TYPE_AS_POD(FVertexBufferBinding);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVertexInputState {
    STATIC_CONST_INTEGRAL(u32, AutoBindingIndex, UMax);
    STATIC_CONST_INTEGRAL(u32, UnknownVertexIndex, UMax);

    using FVertices = TFixedSizeHashMap<FVertexID, FVertexInput, MaxVertexAttribs>;
    using FBindings = TFixedSizeFlatMap<FVertexBufferID, FVertexBufferBinding, MaxVertexBuffers>;

    FBindings BufferBindings;
    FVertices Vertices;

    template <typename _Class, typename _Value>
    FVertexInputState& Add(const FVertexID& vertexId, _Value _Class::* member, const FVertexBufferID& bufferId = Default) NOEXCEPT {
        return Add(vertexId, VertexAttrib<_Value>(), checked_cast<u32>(Meta::StandardLayoutOffset(member)), bufferId);
    }

    template <typename _Class, typename _Value>
    FVertexInputState& Add(const FVertexID& vertexId, _Value _Class::* member, bool normalized, const FVertexBufferID& bufferId = Default) NOEXCEPT {
        EVertexFormat attrib = VertexAttrib<_Value>();
        attrib = (normalized
            ? attrib + EVertexFormat::NormalizedFlag
            : attrib - EVertexFormat::NormalizedFlag );
        return Add(vertexId, attrib, checked_cast<u32>(Meta::StandardLayoutOffset(member)), bufferId);
    }

    PPE_RHI_API FVertexInputState& Add(const FVertexID& vertexId, EVertexFormat fmt, u32 offset, const FVertexBufferID& bufferId = Default) NOEXCEPT;
    PPE_RHI_API FVertexInputState& Bind(const FVertexBufferID& bufferId, u32 stride, u32 index = AutoBindingIndex, EVertexInputRate rate = EVertexInputRate::Vertex) NOEXCEPT;

    NODISCARD PPE_RHI_API bool CopyAttributes(const TMemoryView<const FVertexAttribute> attribs) NOEXCEPT;

    void Clear() {
        BufferBindings.clear();
        Vertices.clear();
    }

    bool operator ==(const FVertexInputState& other) const NOEXCEPT { return (BufferBindings == other.BufferBindings && Vertices == other.Vertices); }
    bool operator !=(const FVertexInputState& other) const NOEXCEPT { return (not operator ==(other)); }

    friend hash_t hash_value(const FVertexInputState& state) NOEXCEPT {
        return hash_tuple(state.BufferBindings, state.Vertices);
    }

};
PPE_ASSUME_TYPE_AS_POD(FVertexInputState);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
