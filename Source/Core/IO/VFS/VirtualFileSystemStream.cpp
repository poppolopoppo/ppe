#include "stdafx.h"

#include "VirtualFileSystemStream.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::streamsize IVirtualFileSystemIStream::ReadLine(char *storage, std::streamsize capacity) {
    Assert(storage);
    Assert(capacity);

    std::streamsize p = 0;
    while (p + 1 < capacity) {
        char ch;
        if (!ReadSome(&ch, 1))
            break;

        storage[p++] = ch;
        if ('\n' == ch)
            break;
    }

    Assert(p < capacity);
    storage[p] = '\0';
    return p;
}
//----------------------------------------------------------------------------
std::streamsize IVirtualFileSystemIStream::ReadWord(char *storage, std::streamsize capacity) {
    Assert(storage);
    Assert(capacity);

    const std::locale& locale = std::locale::classic();

    std::streamsize p = 0;
    while (p + 1 < capacity) {
        char ch;
        if (!ReadSome(&ch, 1))
            break;

        storage[p++] = ch;
        if (std::isspace(ch, locale))
            break;
    }

    Assert(p < capacity);
    storage[p] = '\0';
    return p;
}
//----------------------------------------------------------------------------
bool IVirtualFileSystemIStream::SeekI_FirstOf(char ch) {
    char read;
    do {
        if (!ReadSome(&read, 1))
            return false;
    }
    while (ch != read);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
