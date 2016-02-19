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
    BasicOCStrStream<char> oss(buffer, capacity);
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
    BasicOCStrStream<wchar_t> oss(buffer, capacity);
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
    BasicOCStrStream<char> oss(buffer, capacity);
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
    BasicOCStrStream<wchar_t> oss(buffer, capacity);
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
} //!namespace Core
