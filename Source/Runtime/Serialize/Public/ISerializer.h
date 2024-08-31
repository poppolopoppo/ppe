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
class ISerializer {
protected: // abstract class :
    explicit ISerializer(ESerializeFlag flags = ESerializeFlag::None)
        : _flags(flags)
    {}

public: // virtual :
    virtual ~ISerializer() = default;

    virtual void Deserialize(IStreamReader& input, FTransactionLinker* linker) const = 0;
    virtual void Serialize(const FTransactionSaver& saver, IStreamWriter* output) const = 0;

public: // helpers :
    NODISCARD ESerializeFlag Flags() const { return _flags; }
    void SetFlags(ESerializeFlag flags) { _flags = flags; }

    NODISCARD bool Minify() const { return (_flags & ESerializeFlag::Minify); }
    void SetMinify(bool minify) { _flags = (minify ? _flags + ESerializeFlag::Minify : _flags - ESerializeFlag::Minify); }

    PPE_SERIALIZE_API static void Deserialize(
        const ISerializer& serializer,
        const TMemoryView<const u8>& rawData,
        FTransactionLinker* linker );

    PPE_SERIALIZE_API NODISCARD static bool InteractiveDeserialize(
        const ISerializer& serializer,
        IStreamReader& input, FTransactionLinker* linker );

    PPE_SERIALIZE_API NODISCARD static FExtname Extname(ESerializeFormat fmt);
    PPE_SERIALIZE_API NODISCARD static USerializer FromExtname(const FExtname& ext);
    PPE_SERIALIZE_API NODISCARD static USerializer FromFormat(ESerializeFormat fmt);

private:
    ESerializeFlag _flags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
