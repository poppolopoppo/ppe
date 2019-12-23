#include "stdafx.h"

#include "BuildCache.h"

#include "IO/Dirpath.h"
#include "IO/Extname.h"
#include "IO/Filename.h"
#include "IO/FileSystemProperties.h"
#include "IO/Format.h"
#include "IO/TextWriter.h"
#include "Time/Timestamp.h"
#include "VirtualFileSystem_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define PPE_FILESYSTEMCACHE_EXTNAME L".cache"
//----------------------------------------------------------------------------
class FFilesystemBuildCache : public IBuildCache {
public:
    FFilesystemBuildCache(const FDirpath& path, bool writable)
    :   _path(path)
    ,   _extname(MakeStringView(PPE_FILESYSTEMCACHE_EXTNAME))
    ,   _cacheExpiration(FTimestamp::Now().DaysAgo(30)) // clears everything older than 30 days
    ,   _writable(writable)
    {}

    virtual void Initialize(const FTimestamp&) override final
    {}

    virtual UStreamReader Read(FBuildFingerpint fingerprint) override final {
        return VFS_OpenBinaryReadable(PathFromFingerprint_(fingerprint),
            EAccessPolicy::ShareRead | EAccessPolicy::Sequential );
    }

    virtual bool Write(FBuildFingerpint fingerprint, const FRawMemoryConst& rawdata) override final {
        if (_writable)
            return VFS_WriteAll(PathFromFingerprint_(fingerprint), rawdata,
                EAccessPolicy::Exclusive );
        else
            return false;
    }

    virtual void Cleanup() override final {
        if (_writable) {
            VFS_GlobFiles(_path, L"*" PPE_FILESYSTEMCACHE_EXTNAME, true, [this](const FFilename& fname) {
                if (fname.Extname() == _extname) {
                    FTimestamp cacheDate;
                    if (VFS_FileCreatedAt(&cacheDate, fname) && cacheDate < _cacheExpiration)
                        VFS_RemoveFile(fname);
                }
            });
        }
    }

private:
    FDirpath _path;
    FExtname _extname;
    FTimestamp _cacheExpiration;
    bool _writable;

    FFilename PathFromFingerprint_(FBuildFingerpint fingerprint) const NOEXCEPT {
        wchar_t tmp[20];
        FWFixedSizeTextWriter oss(tmp);

        FDirpath dirname = _path;

        Format(oss, L"{0:#2x}", (fingerprint.lo >> 0) & 0xFF);
        dirname.Concat(FDirname(oss.Written()));
        oss.Reset();

        Format(oss, L"{0:#2x}", (fingerprint.lo >> 8) & 0xFF);
        dirname.Concat(FDirname(oss.Written()));
        oss.Reset();

        Format(oss, L"{0:#8x}-{1:#8x}", fingerprint.lo, fingerprint.hi);
        FBasenameNoExt basename{ oss.Written() };

        return FFilename(dirname, basename, _extname);
    }
};
//----------------------------------------------------------------------------
#undef PPE_FILESYSTEMCACHE_EXTNAME
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
UBuildCache MakeLocalBuildCache(const FDirpath& path, bool writable) {
    return UBuildCache{ new FFilesystemBuildCache(path, writable) };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE

