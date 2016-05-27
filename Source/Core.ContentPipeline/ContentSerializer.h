#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"
#include "Core.ContentPipeline/ContentPipelineNode.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/String.h"

#include "Core.RTTI/RTTIMacros.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentSerializer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentSerializerContext {
public:
    explicit ContentSerializerContext(const Filename& outputFilename) : _outputFilename(outputFilename) {}
    virtual ~ContentSerializerContext() {}

    ContentSerializerContext(const ContentSerializerContext&) = delete;
    ContentSerializerContext& operator=(const ContentSerializerContext&) = delete;

    const Filename& OutputFilename() const { return _outputFilename; }

    virtual ILogger* Logger() const = 0;

private:
    Filename _outputFilename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Asset>
class ContentSerializer;
//----------------------------------------------------------------------------
class IContentSerializer : public ContentPipelineNode {
public:
    virtual ~IContentSerializer() {}

    template <typename _Asset>
    bool Serialize(ContentSerializerContext& ctx, const _Asset& src) const {
        const ContentSerializer<_Asset>* const serializer = dynamic_cast<const ContentSerializer<_Asset>*>(this);
        if (nullptr == serializer)
            throw ContentSerializerException("invalid serializer type", ctx.OutputFilename(), this);
        else
            return serializer->Serialize(ctx, src);
    }

    template <typename _Asset>
    bool Deserialize(ContentSerializerContext& ctx, _Asset& dst) const {
        const ContentSerializer<_Asset>* const serializer = dynamic_cast<const ContentSerializer<_Asset>*>(this);
        if (nullptr == serializer)
            throw ContentSerializerException("invalid serializer type", ctx.OutputFilename(), this);
        else
            return serializer->Deserialize(ctx, dst);
    }

    RTTI_CLASS_HEADER(IContentSerializer, ContentPipelineNode);
};
//----------------------------------------------------------------------------
template <typename _Asset>
class ContentSerializer : public IContentSerializer {
public:
    typedef _Asset asset_type;

    virtual ~ContentSerializer() {}

    virtual bool Serialize(ContentSerializerContext& ctx, const asset_type& src) const = 0;
    virtual bool Deserialize(ContentSerializerContext& ctx, asset_type& dst) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
