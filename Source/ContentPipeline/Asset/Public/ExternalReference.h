#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "MetaTypeTraits.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Asset>
class TExternalReference {
public:
    typedef _Asset asset_type;

    TExternalReference() {}
    explicit TExternalReference(const FFilename& target) : _target(target) { Assert(not target.empty()); }
    TExternalReference(const FFilename& absolute, const FContentIdentity& source)
        : TExternalReference(absolute.Relative(source.SourceFilename().Dirpath())) {}

    FFilename& Target() {return _target; }
    const FFilename& Target() const {return _target; }

private:
    FFilename _target;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TExternalReference<_Asset> wrapped as a string for RTTI :
//----------------------------------------------------------------------------
template <typename _Asset>
struct TMetaTypeTraitsImpl< PPE::ContentPipeline::TExternalReference<_Asset> > {
    typedef TMetaTypeTraitsImpl< FFilename > filename_traits;

    typedef PPE::ContentPipeline::TExternalReference<_Asset> wrapped_type;
    typedef typename filename_traits::wrapper_type wrapper_type;

    typedef TMetaType< wrapper_type > meta_type;

    static const TMetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return TMetaTypeScalarTraits< wrapper_type >::Get();
    }

    static bool IsDefaultValue(const wrapped_type& value) {
        return filename_traits::IsDefaultValue(value.Target());
    }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) {
        return filename_traits::DeepEquals(lhs.Target(), rhs.Target());
    }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) {
        filename_traits::WrapCopy(dst, src.Target()); // copy intended, move not available
    }

    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) {
        filename_traits::WrapCopy(dst, src.Target());
    }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
        FFilename target;
        filename_traits::UnwrapMove(target, std::move(src));
        dst = wrapped_type(target);
    }

    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
        FFilename target;
        filename_traits::UnwrapCopy(target, src);
        dst = wrapped_type(target);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
