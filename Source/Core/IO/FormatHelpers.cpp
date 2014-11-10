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
void SizeInBytes::Format(char *buffer, size_t capacity) const {
    BasicOCStrStream<char> oss(buffer, capacity);
    oss.precision(2);
    oss.flags(std::ios_base::fixed);

    const Units::Storage::Bytes bytes(static_cast<double>(this->Value));

    const Units::Storage::Petabytes pb(0.2);
    const Units::Storage::Terabytes tb(0.2);
    const Units::Storage::Gigabytes gb(0.2);
    const Units::Storage::Megabytes mb(0.2);
    const Units::Storage::Kilobytes kb(0.2);

    if (bytes > pb)
        oss << Units::Storage::Petabytes(bytes) << " pb";
    else if (bytes > tb)
        oss << Units::Storage::Terabytes(bytes) << " tb";
    else if (bytes > gb)
        oss << Units::Storage::Gigabytes(bytes) << " gb";
    else if (bytes > mb)
        oss << Units::Storage::Megabytes(bytes) << " mb";
    else if (bytes > kb)
        oss << Units::Storage::Kilobytes(bytes) << " kb";
    else
        oss << bytes << " b";
}
//----------------------------------------------------------------------------
void SizeInBytes::Format(wchar_t *buffer, size_t capacity) const {
    BasicOCStrStream<wchar_t> oss(buffer, capacity);
    oss.precision(2);
    oss.flags(std::ios_base::fixed);

    const Units::Storage::Bytes bytes(static_cast<double>(this->Value));

    const Units::Storage::Petabytes pb(0.2);
    const Units::Storage::Terabytes tb(0.2);
    const Units::Storage::Gigabytes gb(0.2);
    const Units::Storage::Megabytes mb(0.2);
    const Units::Storage::Kilobytes kb(0.2);

    if (bytes > pb)
        oss << Units::Storage::Petabytes(bytes) << L" pb";
    else if (bytes > tb)
        oss << Units::Storage::Terabytes(bytes) << L" tb";
    else if (bytes > gb)
        oss << Units::Storage::Gigabytes(bytes) << L" gb";
    else if (bytes > mb)
        oss << Units::Storage::Megabytes(bytes) << L" mb";
    else if (bytes > kb)
        oss << Units::Storage::Kilobytes(bytes) << L" kb";
    else
        oss << bytes << L" b";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void CountOfElements::Format(char *buffer, size_t capacity) const {
    BasicOCStrStream<char> oss(buffer, capacity);
    oss.precision(2);
    oss.flags(std::ios_base::fixed);

    if (this->Value > 9e5f)
        oss << (this->Value / 1e6f) << " M";
    else if (this->Value > 9e2f)
        oss << (this->Value / 1e3f) << " K";
    else
        oss << this->Value;
}
//----------------------------------------------------------------------------
void CountOfElements::Format(wchar_t *buffer, size_t capacity) const {
    BasicOCStrStream<wchar_t> oss(buffer, capacity);
    oss.precision(2);
    oss.flags(std::ios_base::fixed);

    if (this->Value > 9e5f)
        oss << (this->Value / 1e6f) << L" M";
    else if (this->Value > 9e2f)
        oss << (this->Value / 1e3f) << L" K";
    else
        oss << this->Value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
