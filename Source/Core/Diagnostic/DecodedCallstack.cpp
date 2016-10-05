#include "stdafx.h"

#include "DecodedCallstack.h"

#include "Callstack.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame::FFrame()
: _address(nullptr), _line(0) {}
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame::FFrame(void* address, const wchar_t* symbol, const wchar_t* filename, size_t line)
: _address(address), _symbol(symbol), _filename(filename), _line(line) {}
//----------------------------------------------------------------------------
FDecodedCallstack::FFrame::~FFrame() {}
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
        new ((void*)dst) FFrame(std::move(*src));
        src->~FFrame();
    }

    rvalue._hash = 0;
    rvalue._depth = 0;

    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
