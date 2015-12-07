#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentImporterAttribute;
class ContentImporterContext;
//----------------------------------------------------------------------------
class IContentImporter : public RefCountable {
public:
    virtual ~IContentImporter() {}

    virtual const ContentImporterAttribute& Attribute() const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
