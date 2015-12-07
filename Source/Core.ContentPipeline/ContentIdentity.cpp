#include "stdafx.h"

#include "ContentIdentity.h"

#include "Exceptions.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(ContentPipeline, ContentIdentity, );
//----------------------------------------------------------------------------
void ContentIdentity::AddDependency(const PCContentIdentity& pcontent) {
    Assert(pcontent);
    Add_AssertUnique(_dependencies, pcontent);
}
//----------------------------------------------------------------------------
PContentIdentity ContentIdentity::Create(const Filename& sourceFile) {
    FileIdentifier fileId;
    if (false == FileIdentifier::CreateFromFile(&fileId, sourceFile))
        throw ContentPipelineException("cannot read file", sourceFile);

    PContentIdentity pid = new ContentIdentity(sourceFile, fileId);
    return std::move(pid);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core