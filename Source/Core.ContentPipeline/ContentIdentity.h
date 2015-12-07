#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/FileIdentifier.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ContentIdentity);
//----------------------------------------------------------------------------
class ContentIdentity : public RefCountable {
public:
    ContentIdentity(const Filename& sourceFile, const FileIdentifier& fileId)
        : _sourceFile(sourceFile), _fileId(fileId) {}

    const Filename& SourceFile() const { return _sourceFile; }
    const FileIdentifier& FileId() const { return _fileId; }

    void AddDependency(const PCContentIdentity& pcontent);

    static PContentIdentity Create(const Filename& sourceFile);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    Filename _sourceFile;
    FileIdentifier _fileId;
    VECTORINSITU(Generation, PCContentIdentity, 5) _dependencies;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>&, const ContentIdentity& id) {
    return oss  << id.SourceFile() << ':' << ' '
                << id.FileId().SizeInBytes() << 'b' << ' '
                << id.FileId().LastModified() << ' '
                << id.FileId().Fingerprint() << std::endl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
