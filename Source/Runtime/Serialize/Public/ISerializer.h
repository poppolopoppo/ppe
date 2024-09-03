#pragma once

#include "Serialize.h"

#include "IO/FileSystem_fwd.h"
#include "Memory/InSituPtr.h"
#include "Meta/Enum.h"
#include "Meta/Optional.h"

namespace PPE {
class IStreamReader;
class IStreamWriter;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ESerializeFlag : u32 {
    None    = 0,
    Minify  = 1<<0,

    Default = None,
};
ENUM_FLAGS(ESerializeFlag);
//----------------------------------------------------------------------------
enum class ESerializeFormat : u32 {
    Binary = 0,
    Json = 1,
    Markup = 2,
    Script = 3,

    Default = Script
};
//----------------------------------------------------------------------------
struct FSerializeContext {
    TPtrRef<const FTransactionSaver> Saver;
    TPtrRef<IStreamWriter> Output;

    ESerializeFlag Flags{ Default };

    NODISCARD bool Minify() const { return (Flags & ESerializeFlag::Minify); }
    void SetMinify(bool minify) { Flags = (minify ? Flags + ESerializeFlag::Minify : Flags - ESerializeFlag::Minify); }
};
//----------------------------------------------------------------------------
struct FDeserializeContext {

};
//----------------------------------------------------------------------------
class ISerializer {
protected: // abstract class :
    ISerializer() = default;

public: // virtual :
    virtual ~ISerializer() = default;

    virtual void Deserialize(const FDeserializeContext& ctx, IStreamReader& input, FTransactionLinker* linker) const = 0;
    virtual void Serialize(const FSerializeContext& ctx, const FTransactionSaver& saver, IStreamWriter* output) const = 0;

public: // helpers :
    PPE_SERIALIZE_API static void Deserialize(
        const FDeserializeContext& ctx,
        const ISerializer& serializer,
        const TMemoryView<const u8>& rawData,
        FTransactionLinker* linker );

    NODISCARD PPE_SERIALIZE_API static bool InteractiveDeserialize(
        const FDeserializeContext& ctx,
        const ISerializer& serializer,
        IStreamReader& input, FTransactionLinker* linker );

    NODISCARD PPE_SERIALIZE_API static FExtname Extname(ESerializeFormat fmt);
    NODISCARD PPE_SERIALIZE_API static USerializer FromExtname(const FExtname& ext);
    NODISCARD PPE_SERIALIZE_API static USerializer FromFormat(ESerializeFormat fmt);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
