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
class ContentPipelineNode : public RTTI::MetaObject {
public:
    ContentPipelineNode();
    virtual ~ContentPipelineNode();

    ContentPipelineNode(ContentPipelineNode& ) = delete;
    ContentPipelineNode& operator =(ContentPipelineNode& ) = delete;

    const String& NodeName() const { return _nodeName; }
    const u128& FingerPrint() const { return _fingerPrint; }

    bool CheckFingerPrint() const;

    RTTI_CLASS_HEADER(ContentPipelineNode, RTTI::MetaObject);

protected:
    void RefreshFingerPrint_() const;

    virtual void RTTI_Load(RTTI::MetaLoadContext *context) override;

private:
    String _nodeName;
    mutable u128 _fingerPrint;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
