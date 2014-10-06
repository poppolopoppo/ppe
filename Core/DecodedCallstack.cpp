#include "stdafx.h"

#include "DecodedCallstack.h"

#include "Callstack.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DecodedCallstack::Frame::Frame()
: _address(nullptr), _line(0) {}
//----------------------------------------------------------------------------
DecodedCallstack::Frame::Frame(void* address, const wchar_t* symbol, const wchar_t* filename, size_t line)
: _address(address), _symbol(symbol), _filename(filename), _line(line) {}
//----------------------------------------------------------------------------
DecodedCallstack::Frame::~Frame() {}
//----------------------------------------------------------------------------
DecodedCallstack::Frame::Frame(Frame&& rvalue)
:   _address(rvalue._address)
,   _symbol(std::move(rvalue._symbol))
,   _filename(std::move(rvalue._filename))
,   _line(rvalue._line) {
    rvalue._address = nullptr;
    rvalue._line = 0;
}
//----------------------------------------------------------------------------
DecodedCallstack::Frame& DecodedCallstack::Frame::operator =(Frame&& rvalue) {
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
DecodedCallstack::DecodedCallstack()
: _hash(0), _depth(0) {}
//----------------------------------------------------------------------------
DecodedCallstack::DecodedCallstack(const Callstack& callstack)
: _hash(0), _depth(0) {
    callstack.Decode(this);
}
//----------------------------------------------------------------------------
DecodedCallstack::~DecodedCallstack() {
    // manually call the destructor since it's a raw storage
    Frame* p = reinterpret_cast<Frame *>(&_frames);
    for (size_t i = 0; i < _depth; ++i, ++p)
        p->~Frame();
}
//----------------------------------------------------------------------------
DecodedCallstack::DecodedCallstack(DecodedCallstack&& rvalue)
: _hash(0), _depth(0) {
    operator =(std::move(rvalue));
}
//----------------------------------------------------------------------------
DecodedCallstack& DecodedCallstack::operator = (DecodedCallstack&& rvalue) {
    _hash = rvalue._hash;
    _depth = rvalue._depth;

    // manually move the objects since it's a raw storage
    Frame* dst = reinterpret_cast<Frame *>(&_frames);
    Frame* src = reinterpret_cast<Frame *>(&rvalue._frames);
    for (size_t i = 0; i < _depth; ++i, ++dst, ++src)
        *dst = std::move(*src);

    rvalue._hash = 0;
    rvalue._depth = 0;

    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core