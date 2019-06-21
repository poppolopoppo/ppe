#pragma once

#include "Serialize.h"

#include "IO/FileSystem_fwd.h"
#include "Memory/InSituPtr.h"
#include "Meta/enum.h"

namespace PPE {
class IStreamReader;
class IStreamWriter;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using PSerializer = TInSituPtr<ISerializer>;
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
class PPE_SERIALIZE_API ISerializer : Meta::FNonCopyableNorMovable {
protected: // abstract class :
    explicit ISerializer(ESerializeFlag flags = ESerializeFlag::None)
        : _flags(flags)
    {}

public: // virtual :
    virtual ~ISerializer() = default;

    virtual void Deserialize(IStreamReader& input, FTransactionLinker* linker) const = 0;
    virtual void Serialize(const FTransactionSaver& saver, IStreamWriter* output) const = 0;

public: // helpers :
    ESerializeFlag Flags() const { return _flags; }
    void SetFlags(ESerializeFlag flags) { _flags = flags; }

    bool Minify() const { return (_flags & ESerializeFlag::Minify); }
    void SetMinify(bool minify) { _flags = (minify ? _flags + ESerializeFlag::Minify : _flags - ESerializeFlag::Minify); }

    static void Deserialize(
        const ISerializer& serializer,
        const TMemoryView<const u8>& rawData,
        FTransactionLinker* linker );

    static bool InteractiveDeserialize(
        const ISerializer& serializer,
        IStreamReader& input, FTransactionLinker* linker );

    static FExtname Extname(ESerializeFormat fmt);
    static PSerializer FromExtname(const FExtname& ext);
    static PSerializer FromFormat(ESerializeFormat fmt);

private:
    ESerializeFlag _flags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
