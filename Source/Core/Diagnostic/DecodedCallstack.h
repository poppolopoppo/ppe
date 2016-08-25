#pragma once

#include "Core/Core.h"

#include "Core/IO/String.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Callstack;
//----------------------------------------------------------------------------
class DecodedCallstack {
public:
    friend class Callstack;
    enum { MaxDeph = 46 };

    class Frame {
    public:
        Frame();
        Frame(void* address, const wchar_t* symbol, const wchar_t* filename, size_t line);
        ~Frame();

        Frame(Frame&& rvalue);
        Frame& operator =(Frame&& rvalue);

        Frame(const Frame& other) = delete;
        Frame& operator =(const Frame& other) = delete;

        void* Address() const { return _address; }
        const WString& Symbol() const { return _symbol; }
        const WString& Filename() const { return _filename; }
        size_t Line() const { return _line; }

    private:
        void*   _address;
        WString _symbol;
        WString _filename;
        size_t  _line;
    };

    DecodedCallstack();
    DecodedCallstack(const Callstack& callstack);
    ~DecodedCallstack();

    DecodedCallstack(DecodedCallstack&& rvalue);
    DecodedCallstack& operator =(DecodedCallstack&& rvalue);

    DecodedCallstack(const DecodedCallstack& other) = delete;
    DecodedCallstack& operator =(const DecodedCallstack& other) = delete;

    size_t Hash() const { return _hash; }
    size_t Depth() const { return _depth; }

    MemoryView<const Frame> Frames() const {
        return MemoryView<const Frame>(reinterpret_cast<const Frame*>(&_frames), _depth);
    }

private:
    // no ctor/dtor overhead for unused frames
    typedef ALIGNED_STORAGE(sizeof(Frame) * MaxDeph, std::alignment_of<Frame>::value)
        frames_type;

    size_t _hash;
    size_t _depth;
    frames_type _frames;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits>
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const DecodedCallstack::Frame& frame) {
    return oss  << frame.Symbol() << std::endl
                << "    " << frame.Filename()
                << '(' << frame.Line() << ')';
}
//----------------------------------------------------------------------------
template <typename _Traits>
std::basic_ostream<wchar_t, _Traits>& operator <<(
	std::basic_ostream<wchar_t, _Traits>& oss,
	const DecodedCallstack::Frame& frame) {
	return oss << frame.Symbol() << std::endl
		<< L"    " << frame.Filename()
		<< L'(' << frame.Line() << L')';
}
//----------------------------------------------------------------------------
template <typename _Traits>
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const DecodedCallstack& decoded) {
    const MemoryView<const DecodedCallstack::Frame> frames = decoded.Frames();
    for (size_t i = 0; i < frames.size(); ++i)
        oss << '[' << i << "] " << frames[i] << std::endl;
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Traits>
std::basic_ostream<wchar_t, _Traits>& operator <<(
	std::basic_ostream<wchar_t, _Traits>& oss,
	const DecodedCallstack& decoded) {
	const MemoryView<const DecodedCallstack::Frame> frames = decoded.Frames();
	for (size_t i = 0; i < frames.size(); ++i)
		oss << L'[' << i << L"] " << frames[i] << std::endl;
	return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
