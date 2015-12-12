#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ContentImporterDescriptor);
//----------------------------------------------------------------------------
class IContentImporter : public RefCountable {
public:
    virtual ~IContentImporter() {}

    virtual const ContentImporterDescriptor* Descriptor() const = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
