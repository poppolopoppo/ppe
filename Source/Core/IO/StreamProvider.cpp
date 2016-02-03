#include "stdafx.h"

#include "StreamProvider.h"

#include "Container/RawStorage.h"
#include "IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(std::streamsize, StreamLookup_ChunkSize_, 128);
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
std::streamsize StreamLookup_(  IStreamReader& iss,
                                _Char *const storage, const std::streamsize capacity,
                                _Pred&& pred ) {
    Assert(storage);
    AssertRelease(capacity > 0);

    std::streamsize written = 0;
    do {
        std::streamsize ask = written + StreamLookup_ChunkSize_;
        if (ask > capacity)
            ask = capacity - written;

        std::streamsize read = 0;
        if (ask > 0)
            read = iss.ReadSome(storage, sizeof(_Char), ask);

        if (read == 0)
            break;

        _Char* const p = storage + written;
        forrange(i, 0, read)
            if (true == pred(p[i]) ) {
                iss.SeekI(i - read + 1, SeekOrigin::Relative);
                Assert(i + 1 < capacity);
                p[i + 1] = _Char(0);
                return written + i;
            }

        written += read;
        Assert(written <= capacity);

    } while (capacity != written);

    return written;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Pred>
bool StreamSeekI_(IStreamReader& iss, _Pred&& pred ) {
    STACKLOCAL_POD_ARRAY(_Char, tmp, StreamLookup_ChunkSize_);
    std::streamsize read;
    do {
        read = StreamLookup_(iss, tmp.Pointer(), tmp.size(), pred);
        if (0 < read && StreamLookup_ChunkSize_ > read)
            return true;
    }
    while (read > 0);
    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::streamsize IStreamReader::ReadLine(char *storage, std::streamsize capacity) {
    return StreamLookup_(*this, storage, capacity, [](const char ch) { return '\n' == ch; });
}
//----------------------------------------------------------------------------
std::streamsize IStreamReader::ReadLine(wchar_t *storage, std::streamsize capacity) {
    return StreamLookup_(*this, storage, capacity, [](const wchar_t wch) { return L'\n' == wch; });
}
//----------------------------------------------------------------------------
std::streamsize IStreamReader::ReadWord(char *storage, std::streamsize capacity) {
    return StreamLookup_(*this, storage, capacity, [](const char ch) { return IsSpace(ch); });
}
//----------------------------------------------------------------------------
std::streamsize IStreamReader::ReadWord(wchar_t *storage, std::streamsize capacity) {
    return StreamLookup_(*this, storage, capacity, [](const wchar_t wch) { return IsSpace(wch); });
}
//----------------------------------------------------------------------------
bool IStreamReader::SeekI_FirstOf(char cmp) {
    return StreamSeekI_<char>(*this, [cmp](const char ch) { return ch == cmp; });
}
//----------------------------------------------------------------------------
bool IStreamReader::SeekI_FirstOf(wchar_t cmp) {
    return StreamSeekI_<wchar_t>(*this, [cmp](const wchar_t wch) { return wch == cmp; });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
