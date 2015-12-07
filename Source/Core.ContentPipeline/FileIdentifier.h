#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core/Container/HashMap.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Time/Timestamp.h"

#include <atomic>
#include <mutex>

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FileIdentifier {
public:
    FileIdentifier() : _sizeInBytes(0), _fingerprint{0,0} {}
    FileIdentifier(u64 sizeInbytes, const Timestamp& lastModified, const u128& fingerprint);

    FileIdentifier(const FileIdentifier& other) { operator =(other); }
    FileIdentifier& FileIdentifier::operator =(const FileIdentifier& other) {
        _sizeInBytes  = other._sizeInBytes;
        _lastModified = other._lastModified;
        _fingerprint  = other._fingerprint;
        return *this;
    }

    u64 SizeInBytes() const { return _sizeInBytes; }
    const Timestamp& LastModified() const { return _lastModified; }
    const u128& Fingerprint() const { return _fingerprint; }

    bool operator ==(const FileIdentifier& other) const {
        return  _sizeInBytes  == other._sizeInBytes &&
                _lastModified == other._lastModified &&
                _fingerprint  == other._fingerprint;
    }
    bool operator !=(const FileIdentifier& other) const { return operator ==(other); }

    static bool CreateFromFile(FileIdentifier *pidentifier, const Filename& sourceFile);

private:
    u64 _sizeInBytes;
    Timestamp _lastModified;
    u128 _fingerprint;
};
STATIC_ASSERT(sizeof(u64)+sizeof(Timestamp)+sizeof(u128) == sizeof(FileIdentifier));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
