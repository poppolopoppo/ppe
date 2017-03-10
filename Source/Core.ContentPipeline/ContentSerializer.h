#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"
#include "Core.ContentPipeline/ContentPipelineNode.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Dirname.h"
#include "Core/IO/String.h"

#include "Core.RTTI/RTTI_Macros.h"

namespace Core {
namespace ContentPipeline {
FWD_INTERFACE_REFPTR(ContentSerializer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FContentSerializerContext {
public:
    explicit FContentSerializerContext(const FFilename& outputFilename) : _outputFilename(outputFilename) {}
    virtual ~FContentSerializerContext() {}

    FContentSerializerContext(const FContentSerializerContext&) = delete;
    FContentSerializerContext& operator=(const FContentSerializerContext&) = delete;

    const FFilename& OutputFilename() const { return _outputFilename; }

    virtual ILogger* Logger() const = 0;

private:
    FFilename _outputFilename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Asset>
class TContentSerializer;
//----------------------------------------------------------------------------
class IContentSerializer : public FContentPipelineNode {
public:
    virtual ~IContentSerializer() {}

    template <typename _Asset>
    bool Serialize(FContentSerializerContext& ctx, const _Asset& src) const {
        const TContentSerializer<_Asset>* const serializer = dynamic_cast<const TContentSerializer<_Asset>*>(this);
        if (nullptr == serializer)
            throw FContentSerializerException("invalid serializer type", ctx.OutputFilename(), this);
        else
            return serializer->Serialize(ctx, src);
    }

    template <typename _Asset>
    bool Deserialize(FContentSerializerContext& ctx, _Asset& dst) const {
        const TContentSerializer<_Asset>* const serializer = dynamic_cast<const TContentSerializer<_Asset>*>(this);
        if (nullptr == serializer)
            throw FContentSerializerException("invalid serializer type", ctx.OutputFilename(), this);
        else
            return serializer->Deserialize(ctx, dst);
    }

    RTTI_CLASS_HEADER(IContentSerializer, FContentPipelineNode);
};
//----------------------------------------------------------------------------
template <typename _Asset>
class TContentSerializer : public IContentSerializer {
public:
    typedef _Asset asset_type;

    virtual ~TContentSerializer() {}

    virtual bool Serialize(FContentSerializerContext& ctx, const asset_type& src) const = 0;
    virtual bool Deserialize(FContentSerializerContext& ctx, asset_type& dst) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
