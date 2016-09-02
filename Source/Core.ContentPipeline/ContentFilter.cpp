#include "stdafx.h"

#include "ContentFilter.h"

#include "Core.RTTI/RTTIMacros-impl.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, IContentFilter, Abstract)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(ContentPipeline, ContentFilterGlob, );
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, ContentFilterGlob, Default)
RTTI_PROPERTY_PRIVATE_FIELD(_pattern)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
ContentFilterGlob::ContentFilterGlob() {}
//----------------------------------------------------------------------------
ContentFilterGlob::ContentFilterGlob(pattern_type&& pattern)
    : _pattern(std::move(pattern)) {}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void ContentFilterGlob::RTTI_VerifyPredicates() const {
    MetaClass::parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _pattern.empty());
}
#endif
//----------------------------------------------------------------------------
ContentFilterGlob::~ContentFilterGlob() {}
//----------------------------------------------------------------------------
bool ContentFilterGlob::Matches(const WStringView& sourceFilename) const {
    STATIC_ASSERT(Case::Insensitive == FileSystem::CaseSensitive);
    return WildMatchI(MakeStringView(_pattern), sourceFilename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(ContentPipeline, ContentFilterGroup, );
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, ContentFilterGroup, Default)
RTTI_PROPERTY_PRIVATE_FIELD(_group)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
ContentFilterGroup::ContentFilterGroup() {}
//----------------------------------------------------------------------------
ContentFilterGroup::ContentFilterGroup(group_type&& group)
    : _group(std::move(group)) {}
//----------------------------------------------------------------------------
ContentFilterGroup::~ContentFilterGroup() {}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void ContentFilterGroup::RTTI_VerifyPredicates() const {
    MetaClass::parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _group.empty());
}
#endif
//----------------------------------------------------------------------------
bool ContentFilterGroup::Matches(const WStringView& sourceFilename) const {
    for (const PCContentFilter& filter : _group) {
        if (filter->Matches(sourceFilename))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(ContentPipeline, ContentFilterRegexp, );
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, ContentFilterRegexp, Default)
RTTI_PROPERTY_PRIVATE_FIELD(_regexp)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
ContentFilterRegexp::ContentFilterRegexp() {}
//----------------------------------------------------------------------------
ContentFilterRegexp::ContentFilterRegexp(string_type&& regexp)
    : _regexp(std::move(regexp))
    , _compiled(MakeRegexp(MakeStringView(_regexp))) {}
//----------------------------------------------------------------------------
ContentFilterRegexp::~ContentFilterRegexp() {}
//----------------------------------------------------------------------------
void ContentFilterRegexp::RTTI_Load(RTTI::MetaLoadContext* context) {
    UNUSED(context);
    _compiled = MakeRegexp(MakeStringView(_regexp));
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void ContentFilterRegexp::RTTI_VerifyPredicates() const {
    MetaClass::parent_type::RTTI_VerifyPredicates();
}
#endif
//----------------------------------------------------------------------------
bool ContentFilterRegexp::Matches(const WStringView& sourceFilename) const {
    return Match(_compiled, sourceFilename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
