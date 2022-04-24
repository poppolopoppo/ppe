#pragma once

#include "Serialize_fwd.h"

#include "Binary/BinaryFormat.h"
#include "Container/HashMap.h"
#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "IO/Basename.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/StreamProvider.h"
#include "IO/String.h"

#include "RTTI_fwd.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/Any.h"
#include "MetaObject.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBinaryFormatReader : Meta::FNonCopyableNorMovable, RTTI::IAtomVisitor {
public:
    FBinaryFormatReader();

    void Read(IBufferedStreamReader& iss, FTransactionLinker& link);

private: // RTTI::IAtomVisitor
    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final;
    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final;
    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override final;

#if USE_PPE_BINA_MARKERS
#   define BINA_READ_TRAITS(_Traits) VerifyRelease(_iss->ExpectPOD((_Traits)->TypeId()))
#else
#   define BINA_READ_TRAITS(_Traits) Unused(_Traits)
#endif

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const RTTI::IScalarTraits* scalar, T& value) override final { \
        BINA_READ_TRAITS(scalar); \
        Read_(scalar, value); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    using FDataIndex = FBinaryFormat::FDataIndex;

    IBufferedStreamReader* _iss;
    FTransactionLinker* _link;

    // holds lifetime while reading the file
    VECTOR(Binary, RTTI::PMetaObject) _objects;

    struct FPropertyMemoizer_ {
        RTTI::FName Name;
        const RTTI::FMetaProperty* Prop;
    };

    struct FContents_ {
        RAWSTORAGE_INSITU(Binary, RTTI::FName, 8) Names;
        RAWSTORAGE_INSITU(Binary, const RTTI::FMetaClass*, 8) Classes;
        RAWSTORAGE_INSITU(Binary, RTTI::FMetaObject*, 8) Imports;
        RAWSTORAGE_INSITU(Binary, FPropertyMemoizer_, 8) Properties;
        RAWSTORAGE_INSITU(Binary, FDirpath, 8) Dirpaths;
        RAWSTORAGE_INSITU(Binary, FBasenameNoExt, 8) BasenameNoExts;
        RAWSTORAGE_INSITU(Binary, FExtname, 8) Extnames;
        RAWSTORAGE_INSITU(Binary, FBinaryFormat::FRawData, 8) Strings;
        RAWSTORAGE_INSITU(Binary, FBinaryFormat::FRawData, 8) WStrings;
        RAWSTORAGE_INSITU(Binary, u8, ALLOCATION_BOUNDARY) Text;
        FBinaryFormat::FRawData Bulk;

    }   _contents;

    bool CheckFingerprint(const FBinaryFormat::FHeaders& h) const;

    bool RetrieveNames_(const FBinaryFormat::FHeaders& h);
    bool RetrieveClasses_(const FBinaryFormat::FHeaders& h);
    bool RetrieveProperties_(const FBinaryFormat::FHeaders& h);
    bool RetrieveImports_(const FBinaryFormat::FHeaders& h);
    bool RetrieveDirpaths_(const FBinaryFormat::FHeaders& h);
    bool RetrieveBasenameNoExts_(const FBinaryFormat::FHeaders& h);
    bool RetrieveExtnames_(const FBinaryFormat::FHeaders& h);
    bool RetrieveStrings_(const FBinaryFormat::FHeaders& h);
    bool RetrieveWStrings_(const FBinaryFormat::FHeaders& h);
    bool RetrieveText_(const FBinaryFormat::FHeaders& h);
    bool RetrieveData_(const FBinaryFormat::FHeaders& h);

    template <typename T, class = Meta::TEnableIf< Meta::is_pod_v<T> > >
    void Read_(const RTTI::IScalarTraits* , T& pod) { _iss->ReadPOD(&pod); }
    void Read_(const RTTI::IScalarTraits* traits, FDirpath& dirpath);
    void Read_(const RTTI::IScalarTraits* traits, RTTI::FName& name);
    void Read_(const RTTI::IScalarTraits* traits, FString& str);
    void Read_(const RTTI::IScalarTraits* traits, FWString& wstr);
    void Read_(const RTTI::IScalarTraits* traits, FFilename& fname);
    void Read_(const RTTI::IScalarTraits* traits, RTTI::FAny& any);
    void Read_(const RTTI::IScalarTraits* traits, RTTI::FBinaryData& bin);
    void Read_(const RTTI::IScalarTraits* traits, RTTI::PMetaObject& obj);

    // using memoizer to cache the lookup in the meta class
    const RTTI::FMetaProperty* FetchProperty_(const RTTI::FMetaClass& klass, const FBinaryFormat::FPropertyData& p);

    // strings are directly instanced for Text section without pooling
    void FetchString_(FDataIndex::value_type id, FString* str) const;
    void FetchString_(FDataIndex::value_type id, FWString* wstr) const;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
