#include "stdafx.h"

#include "Binary/BinaryFormatWriter.h"

#include "RTTI/Any.h"
#include "RTTI/Atom.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/Typedefs.h"

#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "Container/RawStorage.h"
#include "Container/Stack.h"
#include "IO/StringView.h"
#include "Memory/HashFunctions.h"
#include "Time/Timestamp.h"

#include "Diagnostic/Logger.h"

#include <algorithm>

namespace PPE {
namespace Serialize {
EXTERN_LOG_CATEGORY(, Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static void WriteAlign_(IStreamWriter& outp, const TMemoryView<T>& view) {
    static constexpr char GPaddingStr_[] = "PADDING0PADDING1PADDING2PADDING3";

    outp.WriteView(view);

    const size_t paddedSize = ROUND_TO_NEXT_16(view.SizeInBytes());
    const size_t padding = paddedSize - view.SizeInBytes();
    if (padding)
        outp.WriteView(TMemoryView<const char>(GPaddingStr_, padding));
}
//----------------------------------------------------------------------------
template <typename T>
static void WriteAlign_(IStreamWriter& outp, const FBinaryFormat::FRawData& section, const TMemoryView<T>& view) {
    Assert_NoAssume(section.Offset == outp.TellO());
    Assert_NoAssume(section.Size == view.SizeInBytes());
    Unused(section);
    WriteAlign_(outp, view);
}
//----------------------------------------------------------------------------
template <typename _Char>
static void WriteCharArray_(MEMORYSTREAM(Binary)& s, const TBasicStringView<_Char>& view) {
    s.WritePOD(checked_cast<u16>(view.size()));
    s.WriteView(view);
}
//----------------------------------------------------------------------------
static bool SortMetaClass_(const RTTI::FMetaClass* a, const RTTI::FMetaClass* b) {
    return (a->Name() < b->Name());
}
//----------------------------------------------------------------------------
static bool SortMetaProperty_(const RTTI::FMetaProperty* a, const RTTI::FMetaProperty* b) {
    return (a->Name() < b->Name());
}
//----------------------------------------------------------------------------
static bool SortMetaObject_(const RTTI::FMetaObject* a, const RTTI::FMetaObject* b) {
    const RTTI::FMetaTransaction* const outer_a = a->RTTI_Outer();
    const RTTI::FMetaTransaction* const outer_b = b->RTTI_Outer();
    return (outer_a == outer_b
        ? a->RTTI_Name() < b->RTTI_Name()
        : outer_a->Namespace() < outer_b->Namespace());
}
//----------------------------------------------------------------------------
template <typename T, typename _Write, typename _Pred = Meta::TLess<T> >
static void Sort_(
    HASHMAP(Binary, T, FBinaryFormat::FDataIndex)& map,
    _Write write, _Pred pred = _Pred{}) {
    // trixxing to avoid map rehashing
    using map_t = HASHMAP(Binary, T, FBinaryFormat::FDataIndex);
    using item_t = typename map_t::public_type;

    // linearize all pairs
    STACKLOCAL_POD_STACK(item_t*, sorted, map.size());
    for (item_t& it : map)
        sorted.Push(&it);

    // sort linearized vector using pred
    std::sort(sorted.begin(), sorted.end(), [pred](item_t* a, item_t* b) {
        return pred(a->first, b->first);
    });

    // update data index according to index in sorted vector
    forrange(i, 0, sorted.size()) {
        sorted[i]->second = FBinaryFormat::FDataIndex{ u32(i) };
        write(sorted[i]->first);
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBinaryFormatWriter::FBinaryFormatWriter()
{}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Append(const RTTI::FMetaObject* obj) {
    Assert_NoAssume(obj);

    Insert_AssertUnique(_contents.Objects, obj,
        FObjectRef_{ FDataIndex{ checked_cast<u32>(_contents.Objects.size()) }, obj });

    FBinaryFormat::FObjectData objData;
    objData.NumProperties = 0;
    objData.ClassIndex = DataIndex_(_contents.Classes, obj->RTTI_Class());

    if (obj->RTTI_IsExported()) {
        objData.Flags = FBinaryFormat::Export;
        objData.NameIndex = DataIndex_(_contents.Names, obj->RTTI_Name());

        _contents.Flags = _contents.Flags | FBinaryFormat::HasExternalExports;
    }
    else {
        objData.Flags = FBinaryFormat::Default;
        objData.NameIndex = FDataIndex::DefaultValue();
    }

    if (obj->RTTI_IsTopObject())
        objData.Flags = objData.Flags + FBinaryFormat::TopObject;

    const std::streamoff objDataOff = _sections.Data.TellO();
    _sections.Data.WritePOD(objData);

    const RTTI::FMetaClass* const klass = obj->RTTI_Class();

    for (const RTTI::FMetaProperty* prop : klass->AllProperties()) {
        const RTTI::FAtom atom = prop->Get(*obj);

        if (atom.IsDefaultValue())
            continue;

        FBinaryFormat::FPropertyData propData;
        propData.PropertyIndex = DataIndex_(_contents.Properties, prop);

        _sections.Data.WritePOD(propData);

        Verify(atom.Accept(this));

        objData.NumProperties = checked_cast<u16>(u32(objData.NumProperties) + 1);
    }

    const std::streamoff objPropOff = _sections.Data.TellO();

    _sections.Data.SeekO(objDataOff, ESeekOrigin::Begin);
    _sections.Data.WritePOD(objData); // updated NumProperties
    _sections.Data.SeekO(objPropOff, ESeekOrigin::Begin);
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Append(const FTransactionSaver& saver) {
    _contents.Objects.reserve_Additional(saver.LoadedRefs().size());
    _contents.Imports.reserve_Additional(saver.ImportedRefs().size());
    _contents.Names.reserve_Additional(saver.ExportedRefs().size());

    for (const RTTI::SMetaObject& ref : saver.LoadedRefs())
        Append(ref.get());
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Finalize(IStreamWriter& outp, bool mergeSort/* = true */) {
   if (mergeSort)
        MergeSortContents_();

    FBinaryFormat::FHeaders h;
    h.Magic = FBinaryFormat::FILE_MAGIC;
    h.Version = FBinaryFormat::FILE_VERSION;
    h.Flags = _contents.Flags;
    h.Fingerprint = 0;

    ExportContents_(h.Contents);
    ExportSections_(h.Sections);
    MakeFingerprint_(h);
    WriteFileData_(h, outp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FBinaryFormatWriter::Visit(const RTTI::ITupleTraits* tuple, void* data) {
    BINA_WRITE_TRAITS(tuple);

    // tuples have an implicit, static size, no need to serialize it

    return tuple->ForEach(data, [this](const RTTI::FAtom& elt) {
        return elt.Accept(this);
    });
}
//----------------------------------------------------------------------------
bool FBinaryFormatWriter::Visit(const RTTI::IListTraits* list, void* data) {
    BINA_WRITE_TRAITS(list);

    FBinaryFormat::FArrayData arrData;
    arrData.NumElements = checked_cast<u32>(list->Count(data));

    _sections.Data.WritePOD(arrData);

    return list->ForEach(data, [this](const RTTI::FAtom& item) {
        return item.Accept(this);
    });
}
//----------------------------------------------------------------------------
bool FBinaryFormatWriter::Visit(const RTTI::IDicoTraits* dico, void* data) {
    BINA_WRITE_TRAITS(dico);

    FBinaryFormat::FArrayData arrData;
    arrData.NumElements = checked_cast<u32>(dico->Count(data));

    _sections.Data.WritePOD(arrData);

    return dico->ForEach(data, [this](const RTTI::FAtom& key, const RTTI::FAtom& value) {
        return (key.Accept(this) && value.Accept(this));
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Visit_(const FDirpath& dirpath) {
    _sections.Data.WritePOD(DataIndex_(_contents.Dirpaths, dirpath));
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Visit_(const RTTI::FName& name) {
    _sections.Data.WritePOD(DataIndex_(_contents.Names, name));
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Visit_(const FString& str) {
    _sections.Data.WritePOD(DataIndex_(_contents.Strings, str));
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Visit_(const FWString& wstr) {
    _sections.Data.WritePOD(DataIndex_(_contents.WStrings, wstr));
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Visit_(const FFilename& fname) {
    FBinaryFormat::FPathData pathData;
    pathData.Dirpath = DataIndex_(_contents.Dirpaths, fname.Dirpath());
    pathData.BasenameNoExt = DataIndex_(_contents.BasenameNoExts, fname.BasenameNoExt());
    pathData.Extname = DataIndex_(_contents.Extnames, fname.Extname());

    _sections.Data.WritePOD(pathData);
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Visit_(const RTTI::FAny& any) {
    FBinaryFormat::FAnyData anyData;

    if (any) {
        const RTTI::PTypeTraits traits = any.Traits();
        AssertRelease_NoAssume(traits->TypeFlags() ^ RTTI::ETypeFlags::Native);

        anyData.NativeType = traits->TypeId();
    }
    else {
        anyData.NativeType = u32(RTTI::ENativeType::Unknown);
    }

    _sections.Data.WritePOD(anyData);

    if (RTTI::ENativeType(anyData.NativeType) != RTTI::ENativeType::Unknown)
        Verify(any.InnerAtom().Accept(this));
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Visit_(const RTTI::FBinaryData& bin) {
    FBinaryFormat::FBulkData bulkData;
    bulkData.Stream.Offset = checked_cast<u32>(_sections.Bulk.TellO());
    bulkData.Stream.Size =checked_cast<u32>(bin.SizeInBytes());

    _sections.Data.WritePOD(bulkData);
    _sections.Bulk.WriteView(bin.MakeView());

    _contents.Flags = _contents.Flags | FBinaryFormat::HasBulkData;
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Visit_(const RTTI::PMetaObject& obj) {
    FBinaryFormat::FReferenceData refData;
    if (RTTI::FMetaObject* o = obj.get()) {
        auto it = _contents.Objects.find(o);
        if (it == _contents.Objects.end()) {
            // assuming this is an import, since all objects are serialized by inverse order of discovery
            Assert_NoAssume(o->RTTI_IsExported()); // can't import a non exported object

            refData.IsImport = 1;
            refData.ObjectIndex = DataIndex_<const RTTI::FMetaObject*>(_contents.Imports, obj.get()).Value;
        }
        else {
            refData.IsImport = 0;
            refData.ObjectIndex = it->second.Index.Value;
        }
    }
    else {
        refData.IsImport = 0;
#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wbitfield-constant-conversion"
#endif
        refData.ObjectIndex = FDataIndex::DefaultValue();
#ifdef __clang__
#   pragma clang diagnostic pop
#endif
        Assert_NoAssume(refData.IsNull());
    }

    _sections.Data.WritePOD(refData);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const RTTI::FName& name) {
    WriteCharArray_(_sections.Names, name.MakeView());
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const RTTI::FMetaClass* klass) {
    Assert(klass);
    _sections.Classes.WritePOD(DataIndex_(_contents.Names, klass->Name()));
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const RTTI::FMetaProperty* prop) {
    Assert(prop);
    _sections.Properties.WritePOD(DataIndex_(_contents.Names, prop->Name()));
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const RTTI::FMetaObject* obj) {
    Assert(obj);

    FBinaryFormat::FImportData importData;
    importData.TransactionIndex = DataIndex_(_contents.Names, obj->RTTI_Outer()->Namespace());
    importData.NameIndex = DataIndex_(_contents.Names, obj->RTTI_Name());

    _sections.Imports.WritePOD(importData);
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const FDirpath& dirpath) {
    STACKLOCAL_POD_ARRAY(wchar_t, tmp, FileSystem::MaxPathLength);
    WriteCharArray_(_sections.Dirpaths, dirpath.ToWCStr(tmp));
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const FBasenameNoExt& basenameNoExt) {
    WriteCharArray_(_sections.BasenameNoExts, basenameNoExt.MakeView());
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const FExtname& extname) {
    WriteCharArray_(_sections.Extnames, extname.MakeView());
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const FString& string) {
    FBinaryFormat::FRawData rawData;
    rawData.Offset = checked_cast<u32>(_sections.Text.TellO());
    rawData.Size = checked_cast<u32>(string.SizeInBytes());

    _sections.Strings.WritePOD(rawData);
    _sections.Text.WriteView(string.MakeView());
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::Write_(const FWString& wstring) {
    FBinaryFormat::FRawData rawData;
    rawData.Offset = checked_cast<u32>(_sections.Text.TellO());
    rawData.Size = checked_cast<u32>(wstring.SizeInBytes());

    _sections.WStrings.WritePOD(rawData);
    _sections.Text.WriteView(wstring.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FBinaryFormatWriter::MergeSortContents_() {
    // clear all streams (keep them warm)

    _sections.Names.clear();
    _sections.Classes.clear();
    _sections.Properties.clear();
    _sections.Imports.clear();
    _sections.Dirpaths.clear();
    _sections.BasenameNoExts.clear();
    _sections.Extnames.clear();
    _sections.Strings.clear();
    _sections.WStrings.clear();
    _sections.Text.clear();
    _sections.Data.clear();
    _sections.Bulk.clear();

    // sort and merge identical names, producing a smaller file with less entropy

    auto write = [this](const auto& x) { Write_(x); };

    Sort_(_contents.Names, write);
    Sort_(_contents.Classes, write, &SortMetaClass_);
    Sort_(_contents.Properties, write, &SortMetaProperty_);
    Sort_(_contents.Imports, write, &SortMetaObject_);
    Sort_(_contents.Dirpaths, write);
    Sort_(_contents.BasenameNoExts, write);
    Sort_(_contents.Extnames, write);
    Sort_(_contents.Strings, write, TStringLess<char, ECase::Sensitive>{});
    Sort_(_contents.WStrings, write, TStringLess<wchar_t, ECase::Sensitive>{});

    // copy visited objects for a 2nd pass

    STACKLOCAL_POD_ARRAY(const RTTI::FMetaObject*, visiteds, _contents.Objects.size());
    for (const FObjectRef_& obj : _contents.Objects.Values())
        visiteds[obj.Index] = obj.Ref;

    _contents.Objects.clear();

    // finally serialize all visited objects with sorted data

    for (const RTTI::FMetaObject* ref : visiteds)
        Append(ref);
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::ExportContents_(FBinaryFormat::FContents& c) const {
    c.NumNames = checked_cast<u32>(_contents.Names.size());
    c.NumClasses = checked_cast<u32>(_contents.Classes.size());
    c.NumProperties = checked_cast<u32>(_contents.Properties.size());
    c.NumImports = checked_cast<u32>(_contents.Imports.size());
    c.NumObjects = checked_cast<u32>(_contents.Objects.size());
    c.NumDirpaths = checked_cast<u32>(_contents.Dirpaths.size());
    c.NumBasenameNoExts = checked_cast<u32>(_contents.BasenameNoExts.size());
    c.NumExtnames = checked_cast<u32>(_contents.Extnames.size());
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::ExportSections_(FBinaryFormat::FSections& s) const {
    s.Names.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(sizeof(FBinaryFormat::FHeaders)));
    s.Names.Size = checked_cast<u32>(_sections.Names.SizeInBytes());

    s.Classes.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Names.End()));
    s.Classes.Size = checked_cast<u32>(_sections.Classes.SizeInBytes());

    s.Properties.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Classes.End()));
    s.Properties.Size = checked_cast<u32>(_sections.Properties.SizeInBytes());

    s.Imports.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Properties.End()));
    s.Imports.Size = checked_cast<u32>(_sections.Imports.SizeInBytes());

    s.Dirpaths.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Imports.End()));
    s.Dirpaths.Size = checked_cast<u32>(_sections.Dirpaths.SizeInBytes());

    s.BasenameNoExts.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Dirpaths.End()));
    s.BasenameNoExts.Size = checked_cast<u32>(_sections.BasenameNoExts.SizeInBytes());

    s.Extnames.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.BasenameNoExts.End()));
    s.Extnames.Size = checked_cast<u32>(_sections.Extnames.SizeInBytes());

    s.Strings.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Extnames.End()));
    s.Strings.Size = checked_cast<u32>(_sections.Strings.SizeInBytes());

    s.WStrings.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Strings.End()));
    s.WStrings.Size = checked_cast<u32>(_sections.WStrings.SizeInBytes());

    s.Text.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.WStrings.End()));
    s.Text.Size = checked_cast<u32>(_sections.Text.SizeInBytes());

    s.Data.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Text.End()));
    s.Data.Size = checked_cast<u32>(_sections.Data.SizeInBytes());

    s.Bulk.Offset = checked_cast<u32>(ROUND_TO_NEXT_16(s.Data.End()));
    s.Bulk.Size = checked_cast<u32>(_sections.Bulk.SizeInBytes());
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::MakeFingerprint_(FBinaryFormat::FHeaders& h) const {
    h.Fingerprint = 0; // always reset before hashing

    FBinaryFormat::FSignature s;
    s.Headers = Fingerprint128(&h, sizeof(h));
    s.Names = Fingerprint128(_sections.Names.MakeView());
    s.Classes = Fingerprint128(_sections.Classes.MakeView());
    s.Properties = Fingerprint128(_sections.Properties.MakeView());
    s.Imports = Fingerprint128(_sections.Imports.MakeView());
    s.Dirpaths = Fingerprint128(_sections.Dirpaths.MakeView());
    s.BasenameNoExts = Fingerprint128(_sections.BasenameNoExts.MakeView());
    s.Extnames = Fingerprint128(_sections.Extnames.MakeView());
    s.Strings = Fingerprint128(_sections.Strings.MakeView());
    s.WStrings = Fingerprint128(_sections.WStrings.MakeView());
    s.Text = Fingerprint128(_sections.Text.MakeView());
    s.Data = Fingerprint128(_sections.Data.MakeView());
    s.Bulk = Fingerprint128(_sections.Bulk.MakeView());

    h.Fingerprint = Fingerprint32(&s, sizeof(s));

#if USE_PPE_BINA_MARKERS && USE_PPE_LOGGER
    FLogger::Log(LOG_MAKESITE(Serialize, Debug), FBinaryFormat::DumpInfos(h, s));
#endif
}
//----------------------------------------------------------------------------
void FBinaryFormatWriter::WriteFileData_(const FBinaryFormat::FHeaders& h, IStreamWriter& outp) const {
    WriteAlign_(outp, TMemoryView<const FBinaryFormat::FHeaders>(&h, 1));
    WriteAlign_(outp, h.Sections.Names, _sections.Names.MakeView());
    WriteAlign_(outp, h.Sections.Classes, _sections.Classes.MakeView());
    WriteAlign_(outp, h.Sections.Properties, _sections.Properties.MakeView());
    WriteAlign_(outp, h.Sections.Imports, _sections.Imports.MakeView());
    WriteAlign_(outp, h.Sections.Dirpaths, _sections.Dirpaths.MakeView());
    WriteAlign_(outp, h.Sections.BasenameNoExts, _sections.BasenameNoExts.MakeView());
    WriteAlign_(outp, h.Sections.Extnames, _sections.Extnames.MakeView());
    WriteAlign_(outp, h.Sections.Strings, _sections.Strings.MakeView());
    WriteAlign_(outp, h.Sections.WStrings, _sections.WStrings.MakeView());
    WriteAlign_(outp, h.Sections.Text, _sections.Text.MakeView());
    WriteAlign_(outp, h.Sections.Data, _sections.Data.MakeView());
    WriteAlign_(outp, h.Sections.Bulk, _sections.Bulk.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
