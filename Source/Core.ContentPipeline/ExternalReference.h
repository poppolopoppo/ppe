#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Asset>
class ExternalReference {
public:
    typedef _Asset asset_type;

    ExternalReference() {}
    explicit ExternalReference(const Filename& target) : _target(target) { Assert(not target.empty()); }
    ExternalReference(const Filename& absolute, const ContentIdentity& source)
        : ExternalReference(absolute.Relative(source.SourceFilename().Dirpath())) {}

    Filename& Target() {return _target; }
    const Filename& Target() const {return _target; }

private:
    Filename _target;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
