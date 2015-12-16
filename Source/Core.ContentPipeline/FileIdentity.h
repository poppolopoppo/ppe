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
class FileIdentity {
public:
    FileIdentity() : _sizeInBytes(0), _fingerprint{0,0} {}
    FileIdentity(u64 sizeInbytes, const Timestamp& lastModified, const u128& fingerprint);

    FileIdentity(const FileIdentity& other) { operator =(other); }
    FileIdentity& FileIdentity::operator =(const FileIdentity& other) {
        _sizeInBytes  = other._sizeInBytes;
        _lastModified = other._lastModified;
        _fingerprint  = other._fingerprint;
        return *this;
    }

    u64 SizeInBytes() const { return _sizeInBytes; }
    const Timestamp& LastModified() const { return _lastModified; }
    const u128& Fingerprint() const { return _fingerprint; }

    bool operator ==(const FileIdentity& other) const {
        return  _sizeInBytes  == other._sizeInBytes &&
                _lastModified == other._lastModified &&
                _fingerprint  == other._fingerprint;
    }
    bool operator !=(const FileIdentity& other) const { return operator ==(other); }

    static bool CreateFromFile(FileIdentity *pidentifier, const Filename& sourceFile);

private:
    u64 _sizeInBytes;
    Timestamp _lastModified;
    u128 _fingerprint;
};
STATIC_ASSERT(sizeof(u64)+sizeof(Timestamp)+sizeof(u128) == sizeof(FileIdentity));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
