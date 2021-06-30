#pragma once

#include "RHI_fwd.h"

#include "RHI/Config.h"
#include "RHI/ResourceId.h"
#include "RHI/VertexDesc.h"
#include "RHI/VertexEnums.h"
#include "RHI/VertexInputState.h"

#include "Container/FixedSizeHashTable.h"
#include "Container/Hash.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVertexAttribute {
    FVertexID Id;
    u32 Index{UMax};
    EVertexFormat Format{Default};

    bool operator ==(const FVertexAttribute& other) const {
        return (Id == other.Id && Index == other.Index && Format == other.Format);
    }
    bool operator !=(const FVertexAttribute& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVertexAttribute& attr) {
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
    FVertexInput(EVertexFormat fmt, u32 offset, u32 bufferBinding) : Format(fmt), Offset(offset), BufferBinding(bufferBinding) {}

    PPE_RHI_API EVertexFormat DestinationFormat() const;

    bool operator ==(const FVertexInput& other) const {
        Assert(Index != UMax);
        Assert(other.Index != UMax);
        return (Format == other.Format && Index == other.Index && Offset == other.Offset && BufferBinding == other.BufferBinding);
    }
    bool operator !=(const FVertexInput& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVertexInput& input) {
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
    FVertexBufferBinding(u32 index, u32 stride, EVertexInputRate rate) : Index(index), Stride(stride), Rate(rate) {}

    bool operator ==(const FVertexBufferBinding& other) const {
        return (Index == other.Index && Stride == other.Stride && Rate == other.Rate);
    }
    bool operator !=(const FVertexBufferBinding& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVertexBufferBinding& binding) {
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
    using FBindings = TFixedSizeHashMap<FVertexBufferID, FVertexBufferBinding, MaxVertexBuffers>;

    FBindings BufferBindings;
    FVertices Vertices;

    template <typename _Class, typename _Value>
    FVertexInputState& Add(const FVertexID& vertexId, _Value* _Class::* member, const FVertexBufferID& bufferId = Default) {
        return Add(vertexId, VertexAttrib<_Value>(), reinterpret_cast<size_t>(&(static_cast<_Class*>(nullptr)->*member)), bufferId);
    }

    template <typename _Class, typename _Value>
    FVertexInputState& Add(const FVertexID& vertexId, _Value* _Class::* member, bool normalized, const FVertexBufferID& bufferId = Default) {
        EVertexFormat attrib = VertexAttrib<_Value>();
        attrib = (normalized
            ? attrib + EVertexFormat::NormalizedFlag
            : attrib - EVertexFormat::NormalizedFlag );
        return Add(vertexId, attrib, reinterpret_cast<size_t>(&(static_cast<_Class*>(nullptr)->*member)), bufferId);
    }

    PPE_RHI_API FVertexInputState& Add(const FVertexID& vertexId, EVertexFormat fmt, u32 offset, const FVertexBufferID& bufferId = Default);
    PPE_RHI_API FVertexInputState& Bind(const FVertexBufferID& bufferId, u32 stride, u32 index = AutoBindingIndex, EVertexInputRate rate = EVertexInputRate::Vertex);

    PPE_RHI_API NODISCARD bool CopyAttributes(const TMemoryView<const FVertexAttribute> attribs);

    void Clear() {
        BufferBindings.clear();
        Vertices.clear();
    }

    bool operator ==(const FVertexInputState& other) const { return (BufferBindings == other.BufferBindings && Vertices == other.Vertices); }
    bool operator !=(const FVertexInputState& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FVertexInputState& state) {
        return hash_tuple(state.BufferBindings, state.Vertices);
    }

};
PPE_ASSUME_TYPE_AS_POD(FVertexInputState);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
