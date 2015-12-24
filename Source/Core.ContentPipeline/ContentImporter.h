#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ContentImporterDescriptor);
FWD_INTERFACE_REFPTR(ContentImporter);
//----------------------------------------------------------------------------
class ContentImporterContext {
public:
    ContentImporterContext( ILogger* logger,
                            const Dirname& intermediateDirectory,
                            const Dirname& outputDirectory );
    ~ContentImporterContext();

    ILogger* Logger() const { return _logger; }
    const Dirname& IntermediateDirectory() const { return _intermediateDirectory; }
    const Dirname& OutputDirectory() const { return _outputDirectory; }

    void AddDependency(const Filename& fileanme);

private:
    ILogger* _logger;
    Dirname _intermediateDirectory;
    Dirname _outputDirectory;
};
//----------------------------------------------------------------------------
class IContentImporter : public RefCountable {
public:
    virtual ~IContentImporter() {}

    virtual const ContentImporterDescriptor* Descriptor() const = 0;
};
//----------------------------------------------------------------------------
template <typename T>
class ContentImporter : public IContentImporter {
public:
    typedef T import_type;

    virtual ~ContentImporter() {}

    virtual bool Import(const ContentImporterContext& ctx, const Filename& filename, ) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
