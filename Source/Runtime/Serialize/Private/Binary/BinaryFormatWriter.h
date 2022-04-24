#pragma once

#include "Serialize_fwd.h"

#include "Binary/BinaryFormat.h"
#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/Basename.h"
#include "IO/Dirpath.h"
#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "Memory/MemoryStream.h"

#include "RTTI_fwd.h"
#include "RTTI/Any.h"
#include "RTTI/AtomVisitor.h"
#include "MetaObject.h"

#include "TransactionSaver.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBinaryFormatWriter : Meta::FNonCopyableNorMovable, RTTI::IAtomVisitor {
public:
    FBinaryFormatWriter();

    void Append(const RTTI::FMetaObject* obj);
    void Append(const FTransactionSaver& saver);

    void Finalize(IStreamWriter& outp, bool mergeSort = true);

private: // RTTI::IAtomVisitor
    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final;
    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final;
    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override final;

#if USE_PPE_BINA_MARKERS
#   define BINA_WRITE_TRAITS(_Traits) _sections.Data.WritePOD((_Traits)->TypeId())
#else
#   define BINA_WRITE_TRAITS(_Traits) Unused(_Traits)
#endif

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
    virtual bool Visit(const RTTI::IScalarTraits* scalar, T& value) override final { \
        BINA_WRITE_TRAITS(scalar); \
        Visit_(value); \
        return true; \
    }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    using FDataIndex = FBinaryFormat::FDataIndex;
    using FRawData = FBinaryFormat::FRawData;

    struct FObjectRef_ {
        FDataIndex Index;
        RTTI::SCMetaObject Ref;
    };

    struct FContents_ {
        FBinaryFormat::EHeaderFlags Flags = FBinaryFormat::None;

        using objects_type = HASHMAP(Binary, const RTTI::FMetaObject*, FObjectRef_);
        objects_type Objects;

        template <typename T>
        using TDataMap = HASHMAP(Binary, T, FDataIndex);

        TDataMap<RTTI::FName> Names;
        TDataMap<const RTTI::FMetaClass*> Classes;
        TDataMap<const RTTI::FMetaProperty*> Properties;
        TDataMap<const RTTI::FMetaObject*> Imports;
        TDataMap<FDirpath> Dirpaths;
        TDataMap<FBasenameNoExt> BasenameNoExts;
        TDataMap<FExtname> Extnames;
        TDataMap<FString> Strings;
        TDataMap<FWString> WStrings;

    }   _contents;

    struct FSections_ {
        MEMORYSTREAM(Binary) Names;
        MEMORYSTREAM(Binary) Classes;
        MEMORYSTREAM(Binary) Properties;
        MEMORYSTREAM(Binary) Imports;
        MEMORYSTREAM(Binary) Dirpaths;
        MEMORYSTREAM(Binary) BasenameNoExts;
        MEMORYSTREAM(Binary) Extnames;
        MEMORYSTREAM(Binary) Strings;
        MEMORYSTREAM(Binary) WStrings;
        MEMORYSTREAM(Binary) Text;
        MEMORYSTREAM(Binary) Data;
        MEMORYSTREAM(Binary) Bulk;

    }   _sections;

    template <typename T, class = Meta::TEnableIf< Meta::is_pod_v<T> > >
    void Visit_(T pod) { _sections.Data.WritePOD(pod); }

    void Visit_(const FDirpath& dirpath);
    void Visit_(const RTTI::FName& name);
    void Visit_(const FString& str);
    void Visit_(const FWString& wstr);
    void Visit_(const FFilename& fname);
    void Visit_(const RTTI::FAny& any);
    void Visit_(const RTTI::FBinaryData& bin);
    void Visit_(const RTTI::PMetaObject& obj);

    template <typename T>
    FDataIndex DataIndex_(FContents_::TDataMap<T>& map, const T& elt) {
        auto r = map.try_emplace(elt);
        if (Unlikely(r.second)) {
            r.first->second = FDataIndex{ checked_cast<u32>(map.size() - 1) };
            Write_(elt);
        }
        return r.first->second;
    }

    void Write_(const RTTI::FName& name);
    void Write_(const RTTI::FMetaClass* klass);
    void Write_(const RTTI::FMetaProperty* prop);
    void Write_(const RTTI::FMetaObject* obj);
    void Write_(const FDirpath& dirpath);
    void Write_(const FBasenameNoExt& basenameNoExt);
    void Write_(const FExtname& extname);
    void Write_(const FString& string);
    void Write_(const FWString& wstring);

    void MergeSortContents_();
    void ExportContents_(FBinaryFormat::FContents& c) const;
    void ExportSections_(FBinaryFormat::FSections& s) const;
    void MakeFingerprint_(FBinaryFormat::FHeaders& h) const;
    void WriteFileData_(const FBinaryFormat::FHeaders& h, IStreamWriter& outp) const;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
