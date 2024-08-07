﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ContentPipelineNode.h"

#include "RTTI_Macros-impl.h"

#include "Thread/ThreadContext.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, FContentPipelineNode, Abstract)
RTTI_PROPERTY_PRIVATE_FIELD(_nodeName);
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FContentPipelineNode::FContentPipelineNode()
    : _fingerPrint{0,0} {}
//----------------------------------------------------------------------------
FContentPipelineNode::~FContentPipelineNode() {}
//----------------------------------------------------------------------------
bool FContentPipelineNode::CheckFingerPrint() const {
    const u128 computedFingerPrint = RTTI::Fingerprint128(*this);
    return (computedFingerPrint == _fingerPrint);
}
//----------------------------------------------------------------------------
void FContentPipelineNode::RefreshFingerPrint_() const {
    AssertIsMainThread();
    _fingerPrint = RTTI::Fingerprint128(*this);
}
//----------------------------------------------------------------------------
void FContentPipelineNode::RTTI_Load(RTTI::FMetaLoadContext *context) {
    RTTI_parent_type::RTTI_Load(context);
    RefreshFingerPrint_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
