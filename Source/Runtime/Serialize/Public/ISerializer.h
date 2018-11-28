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
enum class ESerializeFlag {
    None    = 0,
    Minify  = 1<<0,
};
ENUM_FLAGS(ESerializeFlag);
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API ISerializer : Meta::FNonCopyableNorMovable {
protected: // abstract class :
    explicit ISerializer(ESerializeFlag flags = ESerializeFlag::None)
        : _flags(flags)
    {}

public: // virtual :
    virtual ~ISerializer() {}

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

    static PSerializer FromExtname(const FExtname& ext);

private:
    ESerializeFlag _flags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
