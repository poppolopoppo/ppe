#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core/IO/String.h"
#include "Core.RTTI/RTTIMacros.h"

namespace Core {
namespace ContentPipeline {
FWD_REFPTR(ContentPipelineNode);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FContentPipelineNode : public RTTI::FMetaObject {
public:
    FContentPipelineNode();
    virtual ~FContentPipelineNode();

    FContentPipelineNode(FContentPipelineNode& ) = delete;
    FContentPipelineNode& operator =(FContentPipelineNode& ) = delete;

    const FString& NodeName() const { return _nodeName; }
    const u128& FingerPrint() const { return _fingerPrint; }

    bool CheckFingerPrint() const;

    RTTI_CLASS_HEADER(FContentPipelineNode, RTTI::FMetaObject);

protected:
    void RefreshFingerPrint_() const;

    virtual void RTTI_Load(RTTI::FMetaLoadContext *context) override;

private:
    FString _nodeName;
    mutable u128 _fingerPrint;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
