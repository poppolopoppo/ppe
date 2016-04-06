#include "stdafx.h"

#include "ContentPipelineNode.h"

#include "Core.RTTI/RTTIMacros-impl.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipelineNode, Abstract)
RTTI_PROPERTY_PRIVATE_FIELD(_nodeName);
RTTI_CLASS_END()
//----------------------------------------------------------------------------
ContentPipelineNode::ContentPipelineNode()
    : _fingerPrint{0,0} {}
//----------------------------------------------------------------------------
ContentPipelineNode::~ContentPipelineNode() {}
//----------------------------------------------------------------------------
bool ContentPipelineNode::CheckFingerPrint() const {
    const u128 computedFingerPrint = RTTI::Fingerprint128(*this);
    return (computedFingerPrint == _fingerPrint);
}
//----------------------------------------------------------------------------
void ContentPipelineNode::RefreshFingerPrint_() const {
    AssertIsMainThread();
    _fingerPrint = RTTI::Fingerprint128(*this);
}
//----------------------------------------------------------------------------
void ContentPipelineNode::RTTI_Load(RTTI::MetaLoadContext *context) {
    MetaClass::parent_type::RTTI_Load(context);
    RefreshFingerPrint_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core