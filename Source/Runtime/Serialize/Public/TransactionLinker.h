#pragma once

#include "Serialize.h"

#include "SerializeExceptions.h"

#include "Container/Vector.h"
#include "IO/Filename.h"
#include "Meta/PointerWFlags.h"

#include "RTTI_fwd.h"
#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FLinkerException : public FSerializeException {
public:
    explicit FLinkerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FClassNotFound : public FLinkerException {
public:
    explicit FClassNotFound(const RTTI::FName& name);
    const RTTI::FName& name() const { return _name; }
#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif
private:
    RTTI::FName _name;
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FObjectNotFound : public FLinkerException {
public:
    explicit FObjectNotFound(const RTTI::FPathName& path);
    const RTTI::FPathName& Path() const { return _path; }
#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif
private:
    RTTI::FPathName _path;
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FUnexpectedObjectClass : public FLinkerException {
public:
    FUnexpectedObjectClass(const RTTI::FMetaClass* expected, const RTTI::FMetaClass* found);
    const RTTI::FMetaClass* Expected() const { return _expected; }
    const RTTI::FMetaClass* Found() const { return _found; }
#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif
private:
    const RTTI::FMetaClass* _expected;
    const RTTI::FMetaClass* _found;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTransactionLinker {
public:
    explicit FTransactionLinker(Meta::FForceInit/* in memory */) NOEXCEPT;
    explicit FTransactionLinker(const FFilename& filename);
    ~FTransactionLinker();

    FTransactionLinker();

    FTransactionLinker(const FTransactionLinker& ) = delete;
    FTransactionLinker& operator =(const FTransactionLinker& ) = delete;

    FTransactionLinker(FTransactionLinker&& rvalue) NOEXCEPT;
    FTransactionLinker& operator =(FTransactionLinker&& rvalue) NOEXCEPT;

    const FFilename& Filename() const { return _filename; }

    size_t NumImports() const { return _imports.size(); }

    void AddTopObject(RTTI::FMetaObject* topObject);
    void AddExport(const RTTI::FName& name, const RTTI::PMetaObject& src);
    void AddImport(const RTTI::FPathName& path, const RTTI::PTypeTraits& traits, RTTI::PMetaObject* dst);

    void AppendTo(FTransactionLinker& other) const;
    void MoveTo(FTransactionLinker& other);

    void Resolve(RTTI::FMetaTransaction& loaded);

    void CheckAssignment(const RTTI::PTypeTraits& traits, const RTTI::FMetaObject& obj) const;

    const RTTI::FMetaClass* ResolveClass(
        const RTTI::FMetaDatabaseReadable& metaDb,
        const RTTI::FName& name ) const;

    RTTI::FMetaObject* ResolveImport(
        const RTTI::FMetaDatabaseReadable& metaDb,
        const RTTI::FPathName& path,
        const RTTI::PTypeTraits& traits ) const;

private:
    struct FImport_ {
        RTTI::FPathName Path;
        RTTI::PTypeTraits Traits;
        RTTI::PMetaObject* Dst;
    };

    FFilename _filename;
    VECTORINSITU(MetaSerialize, RTTI::PMetaObject, 8) _topObjects;
    VECTORINSITU(MetaSerialize, FImport_, 8) _imports;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
