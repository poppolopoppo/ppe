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
TMemoryView<_Char> StreamLookup_(
    IBufferedStreamReader& iss,
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
        for ( ; read < capacity && iss.Peek(storage[read]); ) {
            iss.ReadPOD(&storage[read]);
            read++;
            if (pred(storage[read - 1]))
                break;
        }
    }

    return TMemoryView<_Char>(storage, checked_cast<size_t>(read));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
bool StreamSeekI_(IBufferedStreamReader& iss, _Pred&& pred ) {
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
TMemoryView<char> IBufferedStreamReader::ReadUntil(const TMemoryView<char>& storage, char expected) {
    return StreamLookup_(*this, storage.data(), storage.size(), [expected](const char ch) { return (expected == ch); });
}
//----------------------------------------------------------------------------
TMemoryView<wchar_t> IBufferedStreamReader::ReadUntil(const TMemoryView<wchar_t>& storage, wchar_t expected) {
    return StreamLookup_(*this, storage.data(), storage.size(), [expected](const wchar_t wch) { return (expected == wch); });
}
//----------------------------------------------------------------------------
TMemoryView<char> IBufferedStreamReader::ReadUntil(const TMemoryView<char>& storage, const TMemoryView<const char>& any) {
    Assert(!any.empty());
    return StreamLookup_(*this, storage.data(), storage.size(), [&any](const char ch) { return any.Contains(ch); });
}
//----------------------------------------------------------------------------
TMemoryView<wchar_t> IBufferedStreamReader::ReadUntil(const TMemoryView<wchar_t>& storage, const TMemoryView<const wchar_t>& any) {
    Assert(!any.empty());
    return StreamLookup_(*this, storage.data(), storage.size(), [&any](const wchar_t wch) { return any.Contains(wch); });
}
//----------------------------------------------------------------------------
TMemoryView<char> IBufferedStreamReader::ReadLine(const TMemoryView<char>& storage) {
    return StreamLookup_(*this, storage.data(), storage.size(), [](const char ch) { return '\n' == ch; });
}
//----------------------------------------------------------------------------
TMemoryView<wchar_t> IBufferedStreamReader::ReadLine(const TMemoryView<wchar_t>& storage) {
    return StreamLookup_(*this, storage.data(), storage.size(), [](const wchar_t wch) { return L'\n' == wch; });
}
//----------------------------------------------------------------------------
TMemoryView<char> IBufferedStreamReader::ReadWord(const TMemoryView<char>& storage) {
    return StreamLookup_(*this, storage.data(), storage.size(), [](const char ch) { return IsSpace(ch); });
}
//----------------------------------------------------------------------------
TMemoryView<wchar_t> IBufferedStreamReader::ReadWord(const TMemoryView<wchar_t>& storage) {
    return StreamLookup_(*this, storage.data(), storage.size(), [](const wchar_t wch) { return IsSpace(wch); });
}
//----------------------------------------------------------------------------
bool IBufferedStreamReader::SeekI_FirstOf(char cmp) {
    Assert(IsSeekableI());
    return StreamSeekI_<char>(*this, [cmp](const char ch) { return ch == cmp; });
}
//----------------------------------------------------------------------------
bool IBufferedStreamReader::SeekI_FirstOf(wchar_t cmp) {
    Assert(IsSeekableI());
    return StreamSeekI_<wchar_t>(*this, [cmp](const wchar_t wch) { return wch == cmp; });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
