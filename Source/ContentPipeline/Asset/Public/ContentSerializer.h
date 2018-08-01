#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.ContentPipeline/ContentIdentity.h"
#include "Core.ContentPipeline/ContentPipelineNode.h"

#include "Diagnostic/Logger.h"
#include "IO/FS/Dirname.h"
#include "IO/String.h"

#include "RTTI_Macros.h"

namespace PPE {
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
class IContentSerializer : public FContentPipelineNode, public Meta::TDynamicCastable<TContentSerializer> {
public:
    virtual ~IContentSerializer() {}

    template <typename _Asset>
    bool Serialize(FContentSerializerContext& ctx, const _Asset& src) const {
        const TContentSerializer<_Asset>* const serializer = As<_Asset>();
        if (nullptr == serializer)
            PPE_THROW_IT(FContentSerializerException("invalid serializer type", ctx.OutputFilename(), this));
        else
            return serializer->Serialize(ctx, src);
    }

    template <typename _Asset>
    bool Deserialize(FContentSerializerContext& ctx, _Asset& dst) const {
        const TContentSerializer<_Asset>* const serializer = As<_Asset>();
        if (nullptr == serializer)
            PPE_THROW_IT(FContentSerializerException("invalid serializer type", ctx.OutputFilename(), this));
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

    META_DYNAMIC_CASTABLE_IMPL(TContentSerializer);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
