#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ContentProcessorDescriptor);
FWD_INTERFACE_REFPTR(ContentProcessor);
//----------------------------------------------------------------------------
struct ContentProcessorContext {
    ILogger*    Logger;
    Dirname     IntermediateDirectory;
    Dirname     OutputDirectory;
    Filename    OutputFilename;
};
//----------------------------------------------------------------------------
class IContentProcessor {
public:
    virtual ~IContentProcessor() {}

    virtual const ContentProcessorDescriptor* Descriptor() const = 0;
};
//----------------------------------------------------------------------------
template <typename _Input, typename _Output>
class ContentProcessor : public IContentProcessor {
public:
    typedef _Input input_type;
    typedef _Output output_type;

    virtual ~ContentProcessor() {}

    virtual bool Process(const ContentProcessorContext& ctx, const Filename& filename, import_type* presult) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
