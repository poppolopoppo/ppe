#pragma once

#include "Serialize.h"

#include "Container/HashMap.h"
#include "Container/SparseArray.h"
#include "Container/Vector.h"
#include "IO/Filename.h"
#include "Meta/PointerWFlags.h"
#include "RTTI_fwd.h"
#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"

#include "SerializeExceptions.h"

namespace PPE {
namespace Serialize {
class FTransactionSerializer;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLinkerException : public FSerializeException {
public:
    explicit FLinkerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FClassNotFound : public FLinkerException {
public:
    explicit FClassNotFound(const RTTI::FName& name);
    const RTTI::FName& name() const { return _name; }
private:
    RTTI::FName _name;
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FObjectNotFound : public FLinkerException {
public:
    explicit FObjectNotFound(const RTTI::FPathName& path);
    const RTTI::FPathName& Path() const { return _path; }
private:
    RTTI::FPathName _path;
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FUnexpectedObjectClass : public FLinkerException {
public:
    FUnexpectedObjectClass(const RTTI::FMetaClass* expected, const RTTI::FMetaClass* found);
    const RTTI::FMetaClass* Expected() const { return _expected; }
    const RTTI::FMetaClass* Found() const { return _found; }
private:
    const RTTI::FMetaClass* _expected;
    const RTTI::FMetaClass* _found;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTransactionLinker : Meta::FNonCopyableNorMovable {
public:
    FTransactionLinker(
        RTTI::FMetaTransaction* loaded,
        const FFilename& filename);
    ~FTransactionLinker();

    const RTTI::FMetaTransaction& Loaded() const { return _loaded; }
    const FFilename& Filename() const { return _filename; }

    size_t NumImports() const { return _imports.size(); }

    void AddTopObject(RTTI::FMetaObject* topObject);
    void AddExport(const RTTI::FName& name, const RTTI::PMetaObject& src);
    void AddImport(const RTTI::FPathName& path, const RTTI::PTypeTraits& traits, RTTI::PMetaObject* dst);

    void ResolveImports();

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

    RTTI::FMetaTransaction& _loaded;
    const FFilename _filename;
    SPARSEARRAY(MetaSerialize, FImport_, 64) _imports;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
