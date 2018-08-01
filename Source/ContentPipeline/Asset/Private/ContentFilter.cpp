#include "stdafx.h"

#include "ContentFilter.h"

#include "RTTI_Macros-impl.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "IO/StringView.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, IContentFilter, Abstract)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(ContentPipeline, FContentFilterGlob, );
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, FContentFilterGlob, Default)
RTTI_PROPERTY_PRIVATE_FIELD(_pattern)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FContentFilterGlob::FContentFilterGlob() {}
//----------------------------------------------------------------------------
FContentFilterGlob::FContentFilterGlob(pattern_type&& pattern)
    : _pattern(std::move(pattern)) {}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FContentFilterGlob::RTTI_VerifyPredicates() const {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _pattern.empty());
}
#endif
//----------------------------------------------------------------------------
FContentFilterGlob::~FContentFilterGlob() {}
//----------------------------------------------------------------------------
bool FContentFilterGlob::Matches(const FWStringView& sourceFilename) const {
    STATIC_ASSERT(ECase::Insensitive == FileSystem::CaseSensitive);
    return WildMatchI(MakeStringView(_pattern), sourceFilename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(ContentPipeline, FContentFilterGroup, );
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, FContentFilterGroup, Default)
RTTI_PROPERTY_PRIVATE_FIELD(_group)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FContentFilterGroup::FContentFilterGroup() {}
//----------------------------------------------------------------------------
FContentFilterGroup::FContentFilterGroup(group_type&& group)
    : _group(std::move(group)) {}
//----------------------------------------------------------------------------
FContentFilterGroup::~FContentFilterGroup() {}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FContentFilterGroup::RTTI_VerifyPredicates() const {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _group.empty());
}
#endif
//----------------------------------------------------------------------------
bool FContentFilterGroup::Matches(const FWStringView& sourceFilename) const {
    for (const PCContentFilter& filter : _group) {
        if (filter->Matches(sourceFilename))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(ContentPipeline, FContentFilterRegexp, );
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(ContentPipeline, FContentFilterRegexp, Default)
RTTI_PROPERTY_PRIVATE_FIELD(_regexp)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FContentFilterRegexp::FContentFilterRegexp() {}
//----------------------------------------------------------------------------
FContentFilterRegexp::FContentFilterRegexp(string_type&& regexp)
    : _regexp(std::move(regexp))
    , _compiled(MakeRegexp(MakeStringView(_regexp))) {}
//----------------------------------------------------------------------------
FContentFilterRegexp::~FContentFilterRegexp() {}
//----------------------------------------------------------------------------
void FContentFilterRegexp::RTTI_Load(RTTI::FMetaLoadContext* context) {
    UNUSED(context);
    _compiled = MakeRegexp(MakeStringView(_regexp));
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FContentFilterRegexp::RTTI_VerifyPredicates() const {
    RTTI_parent_type::RTTI_VerifyPredicates();
}
#endif
//----------------------------------------------------------------------------
bool FContentFilterRegexp::Matches(const FWStringView& sourceFilename) const {
    return Match(_compiled, sourceFilename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
