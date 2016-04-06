#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.RTTI/MetaTypeTraits.h"

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

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// ExternalReference<_Asset> wrapped as a string for RTTI :
//----------------------------------------------------------------------------
template <typename _Asset>
struct MetaTypeTraitsImpl< Core::ContentPipeline::ExternalReference<_Asset> > {
    typedef MetaTypeTraitsImpl< Filename > filename_traits;

    typedef Core::ContentPipeline::ExternalReference<_Asset> wrapped_type;
    typedef typename filename_traits::wrapper_type wrapper_type;

    typedef MetaType< wrapper_type > meta_type;

    static const MetaTypeScalarTraits< wrapper_type > *VirtualTraits() {
        return MetaTypeScalarTraits< wrapper_type >::Instance();
    }

    static bool IsDefaultValue(const wrapped_type& value) {
        return filename_traits::IsDefaultValue(value.Target());
    }

    static bool DeepEquals(const wrapped_type& lhs, const wrapped_type& rhs) {
        return Filename::DeepEquals(lhs.Target(), rhs.Target());
    }

    static void WrapMove(wrapper_type& dst, wrapped_type&& src) {
        filename_traits::WrapCopy(dst, src.Target()); // copy intended, move not available
    }

    static void WrapCopy(wrapper_type& dst, const wrapped_type& src) {
        filename_traits::WrapCopy(dst, src.Target());
    }

    static void UnwrapMove(wrapped_type& dst, wrapper_type&& src) {
        Filename target;
        filename_traits::UnwrapMove(target, std::move(src));
        dst = wrapped_type(target);
    }

    static void UnwrapCopy(wrapped_type& dst, const wrapper_type& src) {
        Filename target;
        filename_traits::UnwrapCopy(target, src);
        dst = wrapped_type(target);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
