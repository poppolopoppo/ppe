#include "stdafx.h"

#include "Binary/BinaryFormatReader.h"

#include "Binary/BinarySerializer.h"

#include "RTTI/Atom.h"
#include "RTTI/Any.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"

#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaProperty.h"

#include "TransactionLinker.h"

#include "Allocator/Alloca.h"
#include "Diagnostic/Logger.h"

namespace PPE {
namespace Serialize {
EXTERN_LOG_CATEGORY(PPE_SERIALIZE_API, Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char, typename T, typename _Allocator>
static bool ReadTextSection_(
    IBufferedStreamReader& iss,
    const size_t numEntries,
    const FBinaryFormat::FRawData& section,
    TRawStorage<T, _Allocator>& contents ) {
    Assert_NoAssume(0 == section.Size || Meta::IsAligned(sizeof(_Char), section.Size));
    Assert_NoAssume(0 != numEntries || 0 == section.Size);

    contents.Resize_DiscardData(numEntries);

    T* pentry = contents.data();

#if 1 // with local copy (larger memory spike) :
    STACKLOCAL_POD_ARRAY(_Char, text, section.Size / sizeof(_Char));
    if (not iss.ReadAt(text, section.Offset))
        return false;

    for (const _Char *pch = text.data(), *end = pch + text.size(); pch < end;) {
        const u16 n = (*(const u16*)pch);
        pch += (sizeof(u16) / sizeof(_Char));
        Assert_NoAssume(pch + n <= end);
        Assert_NoAssume(pentry < contents.data() + contents.size());
        INPLACE_NEW(pentry++, T){ TBasicStringView<_Char>(pch, n) };
        pch += n;
    }

#else // using the buffered stream (slower) :

    TAllocaBlock<_Char> tmp;
    iss.SeekI(section.Offset, ESeekOrigin::Begin);

    u32 n;
    for (;;) {
        if (not iss.ReadPOD(&n))
            return false;

        tmp.RelocateIFP(n, false);

        const auto view = TMemoryView<_Char>(tmp.RawData, n);
        if (not iss.ReadView(view))
            return false;

        if (iss.TellI() == section.Offset + section.Size)
            break;

        *(pentry++) = T{ view };
    }

#endif

    return (contents.data() + contents.size() == pentry);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBinaryFormatReader::FBinaryFormatReader()
:   _iss(nullptr)
,   _link(nullptr)
{}
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read(IBufferedStreamReader& iss, FTransactionLinker& link) {
    Assert_NoAssume(nullptr == _iss);
    Assert_NoAssume(nullptr == _link);

    _iss = &iss;
    _link = &link;

    FBinaryFormat::FHeaders h;

    _iss->SeekI(0, ESeekOrigin::Begin);
    if (not _iss->ReadPOD(&h) || h.Magic != FBinaryFormat::FILE_MAGIC)
        PPE_THROW_IT(FBinarySerializerException("invalid file"));
    else if (h.Version != FBinaryFormat::FILE_VERSION)
        PPE_THROW_IT(FBinarySerializerException("version mismatch"));
    else if (not CheckFingerprint(h))
        PPE_THROW_IT(FBinarySerializerException("fingerpint mismatch"));
    else if (not RetrieveNames_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Names section"));
    else if (not RetrieveClasses_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Classes section"));
    else if (not RetrieveProperties_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Properties section"));
    else if (not RetrieveImports_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Imports section"));
    else if (not RetrieveDirpaths_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Dirpaths section"));
    else if (not RetrieveBasenameNoExts_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .BasenameNoExts section"));
    else if (not RetrieveExtnames_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Extnames section"));
    else if (not RetrieveStrings_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Strings section"));
    else if (not RetrieveWStrings_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .WStrings section"));
    else if (not RetrieveText_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Text section"));
    else if (not RetrieveData_(h))
        PPE_THROW_IT(FBinarySerializerException("invalid .Data section"));

    _iss = nullptr;
    _link = nullptr;

    ONLY_IF_ASSERT(_iss = nullptr);
    ONLY_IF_ASSERT(_link = nullptr);
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::CheckFingerprint(const FBinaryFormat::FHeaders& h) const {
#if !(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING) || USE_PPE_BINA_MARKERS
    TAllocaBlock<char> tmp;

    auto sectionFingerprint = [iss=_iss, &tmp](const FBinaryFormat::FRawData& section) -> u128 {
        tmp.RelocateIFP(section.Size, false);
        const auto view = TMemoryView<char>(tmp.RawData, section.Size);
        VerifyRelease(iss->ReadAt(view, section.Offset));
        return Fingerprint128(view);
    };

    FBinaryFormat::FHeaders hZero = h;
    hZero.Fingerprint = 0;

    FBinaryFormat::FSignature s;
    s.Headers = Fingerprint128(&hZero, sizeof(hZero)); // with Fingerpint == 0
    s.Names = sectionFingerprint(h.Sections.Names);
    s.Classes = sectionFingerprint(h.Sections.Classes);
    s.Properties = sectionFingerprint(h.Sections.Properties);
    s.Imports = sectionFingerprint(h.Sections.Imports);
    s.Dirpaths = sectionFingerprint(h.Sections.Dirpaths);
    s.BasenameNoExts = sectionFingerprint(h.Sections.BasenameNoExts);
    s.Extnames = sectionFingerprint(h.Sections.Extnames);
    s.Strings = sectionFingerprint(h.Sections.Strings);
    s.WStrings = sectionFingerprint(h.Sections.WStrings);
    s.Text = sectionFingerprint(h.Sections.Text);
    s.Data = sectionFingerprint(h.Sections.Data);
    s.Bulk = sectionFingerprint(h.Sections.Bulk);

#if USE_PPE_BINA_MARKERS && USE_PPE_LOGGER
    FLogger::Log(
        GLogCategory_Serialize,
        FLogger::EVerbosity::Debug,
        FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
        FBinaryFormat::DumpInfos(h, s) );
    FLogger::Flush(true);
#endif

    const u32 fp = Fingerprint32(&s, sizeof(s));
    return (fp == h.Fingerprint);

#else
    UNUSED(h);
    return true;

#endif
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveNames_(const FBinaryFormat::FHeaders& h) {
    return ReadTextSection_<char>(*_iss, h.Contents.NumNames, h.Sections.Names, _contents.Names);
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveClasses_(const FBinaryFormat::FHeaders& h) {
    STACKLOCAL_ASSUMEPOD_ARRAY(FDataIndex, indices, h.Contents.NumClasses);
    if (indices.SizeInBytes() != h.Sections.Classes.Size ||
        not _iss->ReadAt(indices, h.Sections.Classes.Offset))
        return false;

    _contents.Classes.Resize_DiscardData(h.Contents.NumClasses);
    const RTTI::FMetaClass** pclass = _contents.Classes.data();

    const RTTI::FMetaDatabaseReadable metaDB; // will lock for read the db only once

    for (FDataIndex i : indices)
        *(pclass++) = _link->ResolveClass(metaDB, _contents.Names[i]);

    return true;
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveProperties_(const FBinaryFormat::FHeaders& h) {
    STACKLOCAL_ASSUMEPOD_ARRAY(FDataIndex, indices, h.Contents.NumProperties);
    if (indices.SizeInBytes() != h.Sections.Properties.Size ||
        not _iss->ReadAt(indices, h.Sections.Properties.Offset))
        return false;

    _contents.Properties.Resize_DiscardData(h.Contents.NumProperties);
    FPropertyMemoizer_* pprop = _contents.Properties.data();

    for (FDataIndex i : indices)
        *(pprop++) = FPropertyMemoizer_{ _contents.Names[i], nullptr/* need the class to resolve this */ };

    return true;
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveImports_(const FBinaryFormat::FHeaders& h) {
    STACKLOCAL_ASSUMEPOD_ARRAY(FBinaryFormat::FImportData, imports, h.Contents.NumImports);
    if (imports.SizeInBytes() != h.Sections.Imports.Size ||
        not _iss->ReadAt(imports, h.Sections.Imports.Offset))
        return false;

    _contents.Imports.Resize_DiscardData(h.Contents.NumImports);
    RTTI::FMetaObject** pobj = _contents.Imports.data();

    const RTTI::FMetaDatabaseReadable metaDb; // lock only once

    RTTI::FPathName path;
    for (const FBinaryFormat::FImportData& importData : imports) {
        path.Namespace = _contents.Names[importData.TransactionIndex];
        path.Identifier = _contents.Names[importData.NameIndex];

        *(pobj++) = _link->ResolveImport(metaDb, path, RTTI::PTypeTraits{});
    }

    return true;
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveDirpaths_(const FBinaryFormat::FHeaders& h) {
    return ReadTextSection_<wchar_t>(*_iss, h.Contents.NumDirpaths, h.Sections.Dirpaths, _contents.Dirpaths);
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveBasenameNoExts_(const FBinaryFormat::FHeaders& h) {
    return ReadTextSection_<wchar_t>(*_iss, h.Contents.NumBasenameNoExts, h.Sections.BasenameNoExts, _contents.BasenameNoExts);
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveExtnames_(const FBinaryFormat::FHeaders& h) {
    return ReadTextSection_<wchar_t>(*_iss, h.Contents.NumExtnames, h.Sections.Extnames, _contents.Extnames);
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveStrings_(const FBinaryFormat::FHeaders& h) {
    return _iss->ReadAt(_contents.Strings, h.Sections.Strings.Offset, h.Sections.Strings.Size);
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveWStrings_(const FBinaryFormat::FHeaders& h) {
    return _iss->ReadAt(_contents.WStrings, h.Sections.WStrings.Offset, h.Sections.WStrings.Size);
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveText_(const FBinaryFormat::FHeaders& h) {
    return _iss->ReadAt(_contents.Text, h.Sections.Text.Offset, h.Sections.Text.Size);
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::RetrieveData_(const FBinaryFormat::FHeaders& h) {
    _contents.Bulk = h.Sections.Bulk;
    _objects.resize(h.Contents.NumObjects);

    _iss->SeekI(h.Sections.Data.Offset, ESeekOrigin::Begin);

    FBinaryFormat::FObjectData objData;
    FBinaryFormat::FPropertyData propData;
    for (RTTI::PMetaObject& obj : _objects) {
        Verify(_iss->ReadPOD(&objData));

        const RTTI::FMetaClass* const klass = _contents.Classes[objData.ClassIndex];
        Assert(klass);

        if (not klass->CreateInstance(obj, true))
            return false;

        if (objData.Flags ^ FBinaryFormat::TopObject) {
            _link->AddTopObject(obj.get());
        }

        if (objData.Flags ^ FBinaryFormat::Export) {
            Assert_NoAssume(FDataIndex::DefaultValue() != objData.NameIndex);
            _link->AddExport(_contents.Names[objData.NameIndex], obj);
        }

        forrange(i, 0, u32(objData.NumProperties)) {
            Verify(_iss->ReadPOD(&propData));

            const RTTI::FMetaProperty* const prop = FetchProperty_(*klass, propData);
            if (nullptr == prop)
                return false;

            const RTTI::FAtom atom = prop->Get(*obj);
            Assert_NoAssume(atom);
            Assert_NoAssume(atom.IsDefaultValue());

            Verify(atom.Accept(this));

            Assert_NoAssume(not atom.IsDefaultValue());
        }
    }

    return true;
}
//----------------------------------------------------------------------------
const RTTI::FMetaProperty* FBinaryFormatReader::FetchProperty_(const RTTI::FMetaClass& klass, const FBinaryFormat::FPropertyData& p) {
    FPropertyMemoizer_& m = _contents.Properties[p.PropertyIndex];

    if (nullptr == m.Prop)
        m.Prop = klass.PropertyIFP(m.Name);

    if (nullptr == m.Prop) {
        LOG(Serialize, Error, L"unknown meta property <{0}::{1}>", klass.Name(), m.Name);
        return nullptr;
    }

    return m.Prop;
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::FetchString_(FDataIndex::value_type id, FString* str) const {
    const FBinaryFormat::FRawData& s = _contents.Strings[id];
    str->assign(_contents.Text.SubRange(s.Offset, s.Size).Cast<const FString::char_type>());
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::FetchString_(FDataIndex::value_type id, FWString* str) const {
    const FBinaryFormat::FRawData& s = _contents.WStrings[id];
    str->assign(_contents.Text.SubRange(s.Offset, s.Size).Cast<const FWString::char_type>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FBinaryFormatReader::Visit(const RTTI::ITupleTraits* tuple, void* data) {
    BINA_READ_TRAITS(tuple);

    forrange(i, 0, tuple->Arity()) {
        RTTI::FAtom elt = tuple->At(data, i);
        if (not elt.Accept(this))
            return false;
    }
    return true;
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::Visit(const RTTI::IListTraits* list, void* data) {
    Assert_NoAssume(list->IsEmpty(data));

    BINA_READ_TRAITS(list);

    FBinaryFormat::FArrayData arrData;
    Verify(_iss->ReadPOD(&arrData));

    list->Reserve(data, arrData.NumElements);
    forrange(i, 0, arrData.NumElements) {
        RTTI::FAtom item = list->AddDefault(data);
        if (not item.Accept(this))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FBinaryFormatReader::Visit(const RTTI::IDicoTraits* dico, void* data) {
    Assert_NoAssume(dico->IsEmpty(data));

    BINA_READ_TRAITS(dico);

    FBinaryFormat::FArrayData arrData;
    Verify(_iss->ReadPOD(&arrData));

    dico->Reserve(data, arrData.NumElements);

    STACKLOCAL_ATOM(keyData, dico->KeyTraits());
    const RTTI::FAtom keyAtom = keyData.MakeAtom();

    forrange(i, 0, arrData.NumElements) {
        if (not keyAtom.Accept(this))
            return false;

        const RTTI::FAtom value = dico->AddDefaultMove(data, keyAtom);
        if (not value.Accept(this))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read_(const RTTI::IScalarTraits*, FDirpath& dirpath) {
    FDataIndex::value_type id;
    Verify(_iss->ReadPOD(&id));

    dirpath = _contents.Dirpaths[id];
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read_(const RTTI::IScalarTraits*, RTTI::FName& name) {
    FDataIndex::value_type id;
    Verify(_iss->ReadPOD(&id));

    name = _contents.Names[id];
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read_(const RTTI::IScalarTraits*, FString& str) {
    FDataIndex::value_type id;
    Verify(_iss->ReadPOD(&id));

    FetchString_(id, &str);
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read_(const RTTI::IScalarTraits*, FWString& wstr) {
    FDataIndex::value_type id;
    Verify(_iss->ReadPOD(&id));

    FetchString_(id, &wstr);
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read_(const RTTI::IScalarTraits*, FFilename& fname) {
    FBinaryFormat::FPathData pathData;
    Verify(_iss->ReadPOD(&pathData));

    fname = FFilename(
        _contents.Dirpaths[pathData.Dirpath],
        _contents.BasenameNoExts[pathData.BasenameNoExt],
        _contents.BasenameNoExts[pathData.Extname] );
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read_(const RTTI::IScalarTraits*, RTTI::FAny& any) {
    FBinaryFormat::FAnyData anyData;
    Verify(_iss->ReadPOD(&anyData));

    const auto anyType = RTTI::ENativeType(anyData.NativeType);
    if (RTTI::ENativeType::Unknown != anyType) {
        any = RTTI::FAny(anyType);
        Verify(any.InnerAtom().Accept(this));
    }
    else {
        Assert_NoAssume(not any.Valid());
    }
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read_(const RTTI::IScalarTraits*, RTTI::FBinaryData& bin) {
    FBinaryFormat::FBulkData bulkData;
    Verify(_iss->ReadPOD(&bulkData));

    bin.Resize_DiscardData(bulkData.Stream.Size);

    if (bulkData.Stream.End() > _contents.Bulk.Size ||
        not _iss->ReadAt_SkipBuffer(bin.MakeView(), _contents.Bulk.Offset + bulkData.Stream.Offset))
        PPE_THROW_IT(FBinarySerializerException("invalid .Bulk section"));
}
//----------------------------------------------------------------------------
void FBinaryFormatReader::Read_(const RTTI::IScalarTraits* traits, RTTI::PMetaObject& obj) {
    Assert(traits);

    FBinaryFormat::FReferenceData refData;
    Verify(_iss->ReadPOD(&refData));

    if (refData.IsImport) {
        Assert_NoAssume(not refData.IsNull());

        obj = _contents.Imports[refData.ObjectIndex];
    }
    else if (not refData.IsNull()) {

        obj = _objects[refData.ObjectIndex];
    }
    else {
        Assert_NoAssume(not obj);
    }

#if !(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING) // unchecked assignment for optimized builds
    if (obj)
        _link->CheckAssignment(RTTI::PTypeTraits{ traits }, *obj);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
