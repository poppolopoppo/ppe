#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/FileIdentity.h"

#include "Core.RTTI/RTTIMacros.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FileIdentityCache : public RTTI::MetaObject {
public:
    FileIdentityCache();
    ~FileIdentityCache();

    FileIdentityCache(const FileIdentityCache&) = delete;
    FileIdentityCache& operator =(const FileIdentityCache&) = delete;

    RTTI_CLASS_HEADER(FileIdentityCache, RTTI::MetaObject);

private:
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
