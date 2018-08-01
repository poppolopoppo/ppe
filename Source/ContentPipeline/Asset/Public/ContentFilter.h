#pragma once

#include "Core.ContentPipeline/ContentPipeline.h"

#include "RTTI_Macros.h"

#include "Allocator/PoolAllocator.h"
#include "Container/Vector.h"
#include "IO/FS/Filename.h"
#include "IO/regexp.h"
#include "IO/StringView.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(ContentFilter);
class IContentFilter : public RTTI::FMetaObject {
public:
    virtual ~IContentFilter() {}

    virtual bool Matches(const FWStringView& sourceFilename) const = 0;

    RTTI_CLASS_HEADER(IContentFilter, RTTI::FMetaObject);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FContentFilterGlob : public IContentFilter {
public:
    typedef TBasicString<FileSystem::char_type> pattern_type;

    FContentFilterGlob();
    FContentFilterGlob(pattern_type&& pattern);
    virtual ~FContentFilterGlob();

    virtual bool Matches(const FWStringView& sourceFilename) const override;

    SINGLETON_POOL_ALLOCATED_DECL();
    RTTI_CLASS_HEADER(FContentFilterGlob, IContentFilter);

protected:
#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const override;
#endif
private:
    pattern_type _pattern;
};
//----------------------------------------------------------------------------
class FContentFilterGroup : public IContentFilter {
public:
    typedef VECTOR(Generation, PCContentFilter) group_type;

    FContentFilterGroup();
    FContentFilterGroup(group_type&& group);
    virtual ~FContentFilterGroup();

    virtual bool Matches(const FWStringView& sourceFilename) const override;

    SINGLETON_POOL_ALLOCATED_DECL();
    RTTI_CLASS_HEADER(FContentFilterGroup, IContentFilter);

protected:
#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const override;
#endif
private:
    group_type _group;
};
//----------------------------------------------------------------------------
class FContentFilterRegexp : public IContentFilter {
public:
    typedef TBasicString<FileSystem::char_type> string_type;
    typedef TBasicRegexp<FileSystem::char_type> regexp_type;

    FContentFilterRegexp();
    FContentFilterRegexp(string_type&& regexp);
    virtual ~FContentFilterRegexp();

    virtual bool Matches(const FWStringView& sourceFilename) const override;

    SINGLETON_POOL_ALLOCATED_DECL();
    RTTI_CLASS_HEADER(FContentFilterRegexp, IContentFilter);

    virtual void RTTI_Load(RTTI::FMetaLoadContext* context) override;

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
} //!namespace PPE
