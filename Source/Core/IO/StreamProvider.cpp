#include "stdafx.h"

#include "StreamProvider.h"

#include "Container/RawStorage.h"
#include "IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
MemoryView<_Char> StreamLookup_(  IStreamReader& iss,
                                _Char *const storage, const std::streamsize capacity,
                                _Pred&& pred ) {
    Assert(storage);
    AssertRelease(capacity > 0);

    std::streamsize read = 0;
    if (iss.IsSeekableI()) {
        for (   const std::streamsize offset = iss.TellI();
                read < capacity && iss.Peek(storage[read]); ) {
            read++;
            iss.SeekI(offset + read);
            if (pred(storage[read - 1]))
                break;
        }
    }
    else {
        for (   const std::streamsize offset = iss.TellI();
                read < capacity && iss.Peek(storage[read]); ) {
            iss.ReadPOD(&storage[read]);
            read++;
            if (pred(storage[read - 1]))
                break;
        }
    }

    return MemoryView<_Char>(storage, checked_cast<size_t>(read));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
bool StreamSeekI_(IStreamReader& iss, _Pred&& pred ) {
    _Char ch;
    if (iss.IsSeekableI()) {
        for (std::streamsize offset = iss.TellI(); iss.Peek(ch); iss.SeekI(++offset) ) {
            if (pred(ch))
                return true;
        }
    }
    else {
        for ( ; iss.Peek(ch); iss.ReadPOD(&ch) ) {
            if (pred(ch))
                return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryView<char> IStreamReader::ReadUntil(const MemoryView<char>& storage, char expected) {
    return StreamLookup_(*this, storage.data(), storage.size(), [expected](const char ch) { return (expected == ch); });
}
//----------------------------------------------------------------------------
MemoryView<wchar_t> IStreamReader::ReadUntil(const MemoryView<wchar_t>& storage, wchar_t expected) {
    return StreamLookup_(*this, storage.data(), storage.size(), [expected](const wchar_t wch) { return (expected == wch); });
}
//----------------------------------------------------------------------------
MemoryView<char> IStreamReader::ReadUntil(const MemoryView<char>& storage, const MemoryView<const char>& any) {
    Assert(!any.empty());
    return StreamLookup_(*this, storage.data(), storage.size(), [&any](const char ch) { return any.Contains(ch); });
}
//----------------------------------------------------------------------------
MemoryView<wchar_t> IStreamReader::ReadUntil(const MemoryView<wchar_t>& storage, const MemoryView<const wchar_t>& any) {
    Assert(!any.empty());
    return StreamLookup_(*this, storage.data(), storage.size(), [&any](const wchar_t wch) { return any.Contains(wch); });
}
//----------------------------------------------------------------------------
MemoryView<char> IStreamReader::ReadLine(const MemoryView<char>& storage) {
    return StreamLookup_(*this, storage.data(), storage.size(), [](const char ch) { return '\n' == ch; });
}
//----------------------------------------------------------------------------
MemoryView<wchar_t> IStreamReader::ReadLine(const MemoryView<wchar_t>& storage) {
    return StreamLookup_(*this, storage.data(), storage.size(), [](const wchar_t wch) { return L'\n' == wch; });
}
//----------------------------------------------------------------------------
MemoryView<char> IStreamReader::ReadWord(const MemoryView<char>& storage) {
    return StreamLookup_(*this, storage.data(), storage.size(), [](const char ch) { return IsSpace(ch); });
}
//----------------------------------------------------------------------------
MemoryView<wchar_t> IStreamReader::ReadWord(const MemoryView<wchar_t>& storage) {
    return StreamLookup_(*this, storage.data(), storage.size(), [](const wchar_t wch) { return IsSpace(wch); });
}
//----------------------------------------------------------------------------
bool IStreamReader::SeekI_FirstOf(char cmp) {
    Assert(IsSeekableI());
    return StreamSeekI_<char>(*this, [cmp](const char ch) { return ch == cmp; });
}
//----------------------------------------------------------------------------
bool IStreamReader::SeekI_FirstOf(wchar_t cmp) {
    Assert(IsSeekableI());
    return StreamSeekI_<wchar_t>(*this, [cmp](const wchar_t wch) { return wch == cmp; });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
