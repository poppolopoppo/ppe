#include "stdafx.h"

#include "Diagnostic/DecodedCallstack.h"

#include "Diagnostic/Callstack.h"
#include "IO/Format.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame::FFrame()
: _address(nullptr), _line(0) {}
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame::FFrame(void* address, const wchar_t* symbol, const wchar_t* filename, size_t line)
: _address(address)
, _symbol(MakeCStringView(symbol))
, _filename(MakeCStringView(filename))
, _line(line) {}
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame::FFrame(void* address, FWString&& symbol, FWString&& filename, size_t line)
: _address(address), _symbol(std::move(symbol)), _filename(std::move(filename)), _line(line) {}
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame::~FFrame() = default;
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame::FFrame(FFrame&& rvalue)
:   _address(rvalue._address)
,   _symbol(std::move(rvalue._symbol))
,   _filename(std::move(rvalue._filename))
,   _line(rvalue._line) {
    rvalue._address = nullptr;
    rvalue._line = 0;
}
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame& FDecodedCallstack::FFrame::operator =(FFrame&& rvalue) {
    _address = rvalue._address;
    _symbol = std::move(rvalue._symbol);
    _filename = std::move(rvalue._filename);
    _line = rvalue._line;

    rvalue._address = nullptr;
    rvalue._line = 0;

    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDecodedCallstack::FDecodedCallstack()
: _hash(0), _depth(0) {}
//----------------------------------------------------------------------------
FDecodedCallstack::FDecodedCallstack(const FCallstack& callstack)
: _hash(0), _depth(0) {
    callstack.Decode(this);
}
//----------------------------------------------------------------------------
FDecodedCallstack::~FDecodedCallstack() {
    // manually call the destructor since it's a raw storage
    FFrame* p = reinterpret_cast<FFrame *>(&_frames);
    for (size_t i = 0; i < _depth; ++i, ++p)
        p->~FFrame();
}
//----------------------------------------------------------------------------
FDecodedCallstack::FDecodedCallstack(FDecodedCallstack&& rvalue)
: _hash(0), _depth(0) {
    operator =(std::move(rvalue));
}
//----------------------------------------------------------------------------
FDecodedCallstack& FDecodedCallstack::operator = (FDecodedCallstack&& rvalue) {
    // manually call the destructor since it's a raw storage
    FFrame* p = reinterpret_cast<FFrame *>(&_frames);
    for (size_t i = 0; i < _depth; ++i, ++p)
        p->~FFrame();

    _hash = rvalue._hash;
    _depth = rvalue._depth;

    // manually move the objects since it's a raw storage
    FFrame* dst = reinterpret_cast<FFrame *>(&_frames);
    FFrame* src = reinterpret_cast<FFrame *>(&rvalue._frames);
    for (size_t i = 0; i < _depth; ++i, ++dst, ++src) {
        INPLACE_NEW(dst, FFrame)(std::move(*src));
        src->~FFrame();
    }

    rvalue._hash = 0;
    rvalue._depth = 0;

    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FDecodedCallstack::FFrame& frame) {
    return oss
        << frame.Symbol() << Eol
        << "    " << frame.Filename()
        << '(' << frame.Line() << ')';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FDecodedCallstack::FFrame& frame) {
    return oss
        << frame.Symbol() << Eol
        << L"    " << frame.Filename()
        << L'(' << frame.Line() << L')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FDecodedCallstack& decoded) {
    const TMemoryView<const FDecodedCallstack::FFrame> frames = decoded.Frames();
    forrange(i, 0, frames.size())
        Format(oss, "[{0:2}] {1}\n", i, frames[i]);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FDecodedCallstack& decoded) {
    const TMemoryView<const FDecodedCallstack::FFrame> frames = decoded.Frames();
    forrange(i, 0, frames.size())
        Format(oss, L"[{0:2}] {1}\n", i, frames[i]);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
