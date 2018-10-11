#pragma once

#include "Serialize.h"

#include "IO/FileSystem_fwd.h"
#include "IO/StringView.h"
#include "Memory/InSituPtr.h"
#include "Meta/enum.h"
#include "RTTI_fwd.h"

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
public: // virtual :
    explicit ISerializer(ESerializeFlag flags = ESerializeFlag::None)
        : _flags(flags)
    {}

    virtual ~ISerializer() {}

    virtual void Deserialize(
        const FWStringView& fragment,
        IStreamReader& input,
        RTTI::FMetaTransaction* loaded ) const = 0;

    virtual void Serialize(
        const FWStringView& fragment,
        const RTTI::FMetaTransaction& saved,
        IStreamWriter* output ) const = 0;

public: // helpers :

    ESerializeFlag Flags() const { return _flags; }
    void SetFlags(ESerializeFlag flags) { _flags = flags; }

    bool Minify() const { return (_flags & ESerializeFlag::Minify); }
    void SetMinify(bool minify) { _flags = (minify ? _flags + ESerializeFlag::Minify : _flags - ESerializeFlag::Minify); }

    static void Deserialize(
        const ISerializer& serializer,
        const FWStringView& fragment,
        const TMemoryView<const u8>& rawData,
        RTTI::FMetaTransaction* loaded );

    static PSerializer FromExtname(const FExtname& ext);

private:
    ESerializeFlag _flags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
