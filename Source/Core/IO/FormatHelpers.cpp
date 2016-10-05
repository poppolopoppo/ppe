#include "stdafx.h"

#include "FormatHelpers.h"

#include "Format.h"
#include "Maths/Units.h"
#include "Stream.h"

#include <iomanip>
#include <ostream>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, SizeInBytes size) {
    TBasicOCStrStream<char> oss(buffer, capacity);
    oss.precision(2);
    oss.flags(std::ios_base::fixed);

    const Units::Storage::Bytes bytes(static_cast<double>(size));

    const Units::Storage::Petabytes pb(0.8);
    const Units::Storage::Terabytes tb(0.8);
    const Units::Storage::Gigabytes gb(0.8);
    const Units::Storage::Megabytes mb(0.8);
    const Units::Storage::Kilobytes kb(0.8);

    if (bytes > pb)
        oss << Units::Storage::Petabytes(bytes);
    else if (bytes > tb)
        oss << Units::Storage::Terabytes(bytes);
    else if (bytes > gb)
        oss << Units::Storage::Gigabytes(bytes);
    else if (bytes > mb)
        oss << Units::Storage::Megabytes(bytes);
    else if (bytes > kb)
        oss << Units::Storage::Kilobytes(bytes);
    else
        oss << bytes;
}
//----------------------------------------------------------------------------
void Format(wchar_t *buffer, size_t capacity, SizeInBytes size) {
    TBasicOCStrStream<wchar_t> oss(buffer, capacity);
    oss.precision(2);
    oss.flags(std::ios_base::fixed);

    const Units::Storage::Bytes bytes(static_cast<double>(size));

    const Units::Storage::Petabytes pb(0.8);
    const Units::Storage::Terabytes tb(0.8);
    const Units::Storage::Gigabytes gb(0.8);
    const Units::Storage::Megabytes mb(0.8);
    const Units::Storage::Kilobytes kb(0.8);

    if (bytes > pb)
        oss << Units::Storage::Petabytes(bytes);
    else if (bytes > tb)
        oss << Units::Storage::Terabytes(bytes);
    else if (bytes > gb)
        oss << Units::Storage::Gigabytes(bytes);
    else if (bytes > mb)
        oss << Units::Storage::Megabytes(bytes);
    else if (bytes > kb)
        oss << Units::Storage::Kilobytes(bytes);
    else
        oss << bytes;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, CountOfElements count) {
    TBasicOCStrStream<char> oss(buffer, capacity);
    oss.precision(2);
    oss.flags(std::ios_base::fixed);

    if (count > 9e5f)
        oss << (count / 1e6f) << " M";
    else if (count > 9e2f)
        oss << (count / 1e3f) << " K";
    else
        oss << count.Value;
}
//----------------------------------------------------------------------------
void Format(wchar_t *buffer, size_t capacity, CountOfElements count) {
    TBasicOCStrStream<wchar_t> oss(buffer, capacity);
    oss.precision(2);
    oss.flags(std::ios_base::fixed);

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
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const FHexDump& hexDump) {
    const size_t totalBytes = hexDump.RawData.SizeInBytes();
    for (size_t offset = 0; offset < totalBytes; ) {
        Format(oss, "0x{0:#8X} ", offset);
        const size_t origin = offset;
        for (size_t row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                Format(oss, " {0:#2X}", (unsigned)hexDump.RawData[offset]);
            else
                oss << "   ";
        }
        oss << "  ";
        offset = origin;
        for (size_t row = 0; row < hexDump.BytesPerRow && offset < totalBytes; ++row, ++offset)
            oss << (IsPrint(char(hexDump.RawData[offset])) ? char(hexDump.RawData[offset]) : '.');
        oss << std::endl;
    }
    return oss;
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const FHexDump& hexDump) {
    const size_t totalBytes = hexDump.RawData.SizeInBytes();
    for (size_t offset = 0; offset < totalBytes; ) {
        Format(oss, L"0x{0:#8X} ", offset);
        const size_t origin = offset;
        for (size_t row = 0; row < hexDump.BytesPerRow; ++row, ++offset) {
            if (offset < totalBytes)
                Format(oss, L" {0:#2X}", (unsigned)hexDump.RawData[offset]);
            else
                oss << L"   ";
        }
        oss << L"  ";
        offset = origin;
        for (size_t row = 0; row < hexDump.BytesPerRow && offset < totalBytes; ++row, ++offset)
            oss << (IsPrint(char(hexDump.RawData[offset])) ? char(hexDump.RawData[offset]) : '.');
        oss << std::endl;
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
