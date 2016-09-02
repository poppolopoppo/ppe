#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "Core.RTTI/RTTIMacros.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/regexp.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(ContentFilter);
class IContentFilter : public RTTI::MetaObject {
public:
    virtual ~IContentFilter() {}

    virtual bool Matches(const WStringView& sourceFilename) const = 0;

    RTTI_CLASS_HEADER(IContentFilter, RTTI::MetaObject);
};
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ContentFilterGlob : public IContentFilter {
public:
    typedef BasicString<FileSystem::char_type> pattern_type;

    ContentFilterGlob();
    ContentFilterGlob(pattern_type&& pattern);
    virtual ~ContentFilterGlob();

    virtual bool Matches(const WStringView& sourceFilename) const override;

    SINGLETON_POOL_ALLOCATED_DECL();
    RTTI_CLASS_HEADER(ContentFilterGlob, IContentFilter);

protected:
#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const override;
#endif
private:
    pattern_type _pattern;
};
//----------------------------------------------------------------------------
class ContentFilterGroup : public IContentFilter {
public:
    typedef VECTOR(Generation, PCContentFilter) group_type;

    ContentFilterGroup();
    ContentFilterGroup(group_type&& group);
    virtual ~ContentFilterGroup();

    virtual bool Matches(const WStringView& sourceFilename) const override;

    SINGLETON_POOL_ALLOCATED_DECL();
    RTTI_CLASS_HEADER(ContentFilterGroup, IContentFilter);

protected:
#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const override;
#endif
private:
    group_type _group;
};
//----------------------------------------------------------------------------
class ContentFilterRegexp : public IContentFilter {
public:
    typedef BasicString<FileSystem::char_type> string_type;
    typedef BasicRegexp<FileSystem::char_type> regexp_type;

    ContentFilterRegexp();
    ContentFilterRegexp(string_type&& regexp);
    virtual ~ContentFilterRegexp();

    virtual bool Matches(const WStringView& sourceFilename) const override;

    SINGLETON_POOL_ALLOCATED_DECL();
    RTTI_CLASS_HEADER(ContentFilterRegexp, IContentFilter);

    virtual void RTTI_Load(RTTI::MetaLoadContext* context) override;

protected:
#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const override;
#endif
private:
    string_type _regexp;
    regexp_type _compiled;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core
