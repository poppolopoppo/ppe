#include "stdafx.h"

#include "FormatHelpers.h"

#include "Format.h"
#include "StreamProvider.h"

#include "Maths/Units.h"
#include "Memory/MemoryProvider.h"


namespace Core {
namespace Fmt {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, Fmt::FCountOfElements count) {
    FFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));

    oss.Format().SetPrecision(2);
    oss << FTextFormat::FixedFloat;

    if (count > 9e5f)
        oss << (count / 1e6f) << " M";
    else if (count > 9e2f)
        oss << (count / 1e3f) << " K";
    else
        oss << count.Value;
}
//----------------------------------------------------------------------------
void Format(wchar_t *buffer, size_t capacity, Fmt::FCountOfElements count) {
    FWFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));

    oss.Format().SetPrecision(2);
    oss << FTextFormat::FixedFloat;

    if (count > 9e5f)
        oss << (count / 1e6f) << L" M";
    else if (count > 9e2f)
        oss << (count / 1e3f) << L" K";
    else
        oss << count.Value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, Fmt::FPointer ptr) {
    FFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));
    oss.Write((const void*)ptr.Value);
}
//----------------------------------------------------------------------------
void Format(wchar_t *buffer, size_t capacity, Fmt::FPointer ptr) {
    FWFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));
    oss.Write((const void*)ptr.Value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, Fmt::FSizeInBytes size) {
    FFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));

    oss.Format().SetPrecision(2);
    oss << FTextFormat::FixedFloat;

    const Units::Storage::FBytes bytes(static_cast<double>(size));

    const Units::Storage::FPetabytes pb(0.8);
    const Units::Storage::FTerabytes tb(0.8);
    const Units::Storage::FGigabytes gb(0.8);
    const Units::Storage::FMegabytes mb(0.8);
    const Units::Storage::FKilobytes kb(0.8);

    if (bytes > pb)
        oss << Units::Storage::FPetabytes(bytes);
    else if (bytes > tb)
        oss << Units::Storage::FTerabytes(bytes);
    else if (bytes > gb)
        oss << Units::Storage::FGigabytes(bytes);
    else if (bytes > mb)
        oss << Units::Storage::FMegabytes(bytes);
    else if (bytes > kb)
        oss << Units::Storage::FKilobytes(bytes);
    else
        oss << bytes;
}
//----------------------------------------------------------------------------
void Format(wchar_t *buffer, size_t capacity, Fmt::FSizeInBytes size) {
    FWFixedSizeTextWriter oss(MakeView(buffer, buffer + capacity));

    oss.Format().SetPrecision(2);
    oss << FTextFormat::FixedFloat;

    const Units::Storage::FBytes bytes(static_cast<double>(size));

    const Units::Storage::FPetabytes pb(0.8);
    const Units::Storage::FTerabytes tb(0.8);
    const Units::Storage::FGigabytes gb(0.8);
    const Units::Storage::FMegabytes mb(0.8);
    const Units::Storage::FKilobytes kb(0.8);

    if (bytes > pb)
        oss << Units::Storage::FPetabytes(bytes);
    else if (bytes > tb)
        oss << Units::Storage::FTerabytes(bytes);
    else if (bytes > gb)
        oss << Units::Storage::FGigabytes(bytes);
    else if (bytes > mb)
        oss << Units::Storage::FMegabytes(bytes);
    else if (bytes > kb)
        oss << Units::Storage::FKilobytes(bytes);
    else
        oss << bytes;
}

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Fmt
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const Fmt::FHexDump& hexDump) {
    const size_t totalBytes = hexDump.RawData.SizeInBytes();
    for (size_t offset = 0; offset < totalBytes; ) {
        Core::Format(oss, "0x{0:#8X} ", offset);
        const size_t origin = offset;
        for (size_t row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                Core::Format(oss, " {0:#2X}", (unsigned)hexDump.RawData[offset]);
            else
                oss << "   ";
        }
        oss << "  ";
        offset = origin;
        for (size_t row = 0; row < hexDump.BytesPerRow && offset < totalBytes; ++row, ++offset)
            oss << (IsPrint(char(hexDump.RawData[offset])) ? char(hexDump.RawData[offset]) : '.');
        oss << Eol;
    }
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FHexDump& hexDump) {
    const size_t totalBytes = hexDump.RawData.SizeInBytes();
    for (size_t offset = 0; offset < totalBytes; ) {
        Core::Format(oss, L"0x{0:#8X} ", offset);
        const size_t origin = offset;
        for (size_t row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                Core::Format(oss, L" {0:#2X}", (unsigned)hexDump.RawData[offset]);
            else
                oss << L"   ";
        }
        oss << L"  ";
        offset = origin;
        for (size_t row = 0; row < hexDump.BytesPerRow && offset < totalBytes; ++row, ++offset)
            oss << (IsPrint(char(hexDump.RawData[offset])) ? char(hexDump.RawData[offset]) : '.');
        oss << Eol;
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const Fmt::FIndent& indent) {
    Assert(indent.Level >= 0);
    forrange(i, 0, indent.Level)
        oss << indent.Tab;
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FIndent& indent) {
    Assert(indent.Level >= 0);
    forrange(i, 0, indent.Level)
        for(char ch : indent.Tab)
            oss << ch;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
