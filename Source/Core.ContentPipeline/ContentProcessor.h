#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Misc/TargetPlatform.h"

#include "Core.RTTI/RTTIMacros.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentProcessor);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentProcessorContext {
public:
    explicit ContentProcessorContext(const ContentIdentity& source) : _source(source) {}
    virtual ~ContentProcessorContext() {}

    ContentProcessorContext(const ContentProcessorContext&) = delete;
    ContentProcessorContext& operator=(const ContentProcessorContext&) = delete;

    const ContentIdentity& Source() const { return _source; }

    virtual ILogger* Logger() const = 0;

    virtual TargetPlatform Platform() const = 0;

    virtual const Dirname& IntermediateDir() const = 0;
    virtual const Dirname& OutputDir() const = 0;
    virtual const Filename& OutputFilename() const = 0;

    virtual void AddDependency(const Filename& filename) = 0;
    virtual void AddOutputFile(const Filename& filename) = 0;

private:
    const ContentIdentity& _source;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Input, typename _Output>
class ContentProcessor;
//----------------------------------------------------------------------------
class IContentProcessor : public RTTI::MetaObject {
public:
    virtual ~IContentProcessor() {}

    IContentProcessor(const IContentProcessor&) = delete;
    IContentProcessor& operator=(const IContentProcessor&) = delete;

    virtual const String& Name() const = 0;
    virtual u128 Fingerprint() const = 0;

    template <typename _Input, typename _Output>
    bool Process(ContentProcessorContext& ctx, _Output& dst, const _Input& src) const {
        const ContentProcessor<_Input, _Output>* const processor = dynamic_cast<const ContentProcessor<_Input, _Output>*>(this);
        if (nullptr == processor)
            throw ContentProcessorException("invalid importer type", ctx.Identity(), this);
        else
            return processor->Process(ctx, dst, src);
    }

    RTTI_CLASS_HEADER(IContentProcessor, RTTI::MetaObject);
};
//----------------------------------------------------------------------------
template <typename _Input, typename _Output>
class ContentProcessor : public IContentProcessor {
public:
    typedef _Input input_type;
    typedef _Input output_type;

    virtual ~ContentProcessor() {}

    virtual bool Process(ContentProcessorContext& ctx, output_type& dst, const input_type& src) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
