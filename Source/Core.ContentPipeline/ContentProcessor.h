#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"
#include "Core.ContentPipeline/ContentPipelineNode.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Misc/TargetPlatform.h"

#include "Core.RTTI/RTTI_Macros.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentProcessor);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FContentProcessorContext {
public:
    explicit FContentProcessorContext(   const FContentIdentity& source,
                                        ETargetPlatform platform,
                                        bool debug)
        : _source(source), _platform(platform), _debug(debug) {}
    virtual ~FContentProcessorContext() {}

    FContentProcessorContext(const FContentProcessorContext&) = delete;
    FContentProcessorContext& operator=(const FContentProcessorContext&) = delete;

    const FContentIdentity& Source() const { return _source; }
    ETargetPlatform Platform() const { return _platform; }
    bool Debug() const { return _debug; }

    virtual ILogger* Logger() const = 0;

    virtual const FDirname& IntermediateDir() const = 0;
    virtual const FDirname& OutputDir() const = 0;
    virtual const FFilename& OutputFilename() const = 0;

    virtual void AddDependency(const FFilename& filename) = 0;
    virtual void AddAssetToBuild(const FFilename& filename) = 0;

private:
    const FContentIdentity& _source;
    const ETargetPlatform _platform;
    const bool _debug;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Input, typename _Output>
class TContentProcessor;
//----------------------------------------------------------------------------
class IContentProcessor : public FContentPipelineNode, public Meta::TDynamicCastable<TContentProcessor> {
public:
    virtual ~IContentProcessor() {}

    template <typename _Input, typename _Output>
    bool Process(FContentProcessorContext& ctx, _Output& dst, const _Input& src) const {
        const TContentProcessor<_Input, _Output>* const processor = As<_Input, _Output>();
        if (nullptr == processor)
            throw FContentProcessorException("invalid processor type", ctx.Identity(), this);
        else
            return processor->Process(ctx, dst, src);
    }

    RTTI_CLASS_HEADER(IContentProcessor, FContentPipelineNode);
};
//----------------------------------------------------------------------------
template <typename _Input, typename _Output>
class TContentProcessor : public IContentProcessor {
public:
    typedef _Input input_type;
    typedef _Input output_type;

    virtual ~TContentProcessor() {}

    virtual bool Process(FContentProcessorContext& ctx, output_type& dst, const input_type& src) const = 0;

    META_DYNAMIC_CASTABLE_IMPL(TContentProcessor);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
