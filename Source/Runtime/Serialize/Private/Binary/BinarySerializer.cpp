#include "stdafx.h"

#include "Binary/BinarySerializer.h"

#include "RTTI_fwd.h"
#include "Any.h"
#include "Atom.h"
#include "AtomVisitor.h"
#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "Container/Deque.h"
#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/Basename.h"
#include "IO/BasenameNoExt.h"
#include "IO/Extname.h"
#include "IO/Dirname.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/BufferedStream.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"
#include "Memory/MemoryStream.h"
#include "Memory/MemoryProvider.h"
#include "Misc/FourCC.h"
#include "Meta/StronglyTyped.h"

#ifdef USE_DEBUG_LOGGER
#   define WITH_RTTI_BINARYSERIALIZER_LOG       0 //%_NOCOMMIT%
#   define WITH_RTTI_BINARYSERIALIZER_DATALOG   0 //%_NOCOMMIT%

#   if WITH_RTTI_BINARYSERIALIZER_LOG
        LOG_CATEGORY(, BinSerializer);
#       define BINARYSERIALIZER_LOG(_LEVEL, ...) LOG(BinSerializer, _LEVEL, __VA_ARGS__)
#   else
#       define BINARYSERIALIZER_LOG(...) NOOP()
#   endif

#   if WITH_RTTI_BINARYSERIALIZER_DATALOG
#       define BINARYSERIALIZER_DATALOG(...) LOG(Warning, __VA_ARGS__)
#   else
#       define BINARYSERIALIZER_DATALOG(...) NOOP()
#   endif
#else
#   define BINARYSERIALIZER_LOG(...) NOOP()
#   define BINARYSERIALIZER_DATALOG(...) NOOP()
#endif

// turn to 1 to serialize with more guards (incompatible with regular format !)
#define USE_BINARYSERIALIZER_NAZIFORMAT 0 //%_NOCOMMIT%

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Binary file format
//----------------------------------------------------------------------------
#if USE_BINARYSERIALIZER_NAZIFORMAT
static const FFourCC FILE_MAGIC_                 ("BNZI");
#else
static const FFourCC FILE_MAGIC_                 ("BINA");
#endif
static const FFourCC FILE_VERSION_               ("1.03");
//----------------------------------------------------------------------------
static const FFourCC SECTION_STRINGS_            ("#STR");
static const FFourCC SECTION_WSTRINGS_           ("#WST");
static const FFourCC SECTION_CLASSES_            ("#CLS");
static const FFourCC SECTION_PROPERTIES_         ("#PRP");
static const FFourCC SECTION_TOP_OBJECTS_        ("#TOP");
static const FFourCC SECTION_OBJECTS_HEADER_     ("#OBH");
static const FFourCC SECTION_OBJECTS_DATA_       ("#OBD");
static const FFourCC SECTION_END_                ("#END");
//----------------------------------------------------------------------------
static const FFourCC TAG_OBJECT_EXPORT_          ("OEXP");
static const FFourCC TAG_OBJECT_IMPORT_          ("OIMP");
static const FFourCC TAG_OBJECT_PRIVATE_         ("OPRI");
static const FFourCC TAG_OBJECT_NULL_            ("ONUL");
//----------------------------------------------------------------------------
#if USE_BINARYSERIALIZER_NAZIFORMAT
static const FFourCC TAG_OBJECT_START_           ("OSTA");
static const FFourCC TAG_OBJECT_METACLASS_       ("OMTC");
static const FFourCC TAG_OBJECT_END_             ("OEND");
#endif
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FFourCC) == sizeof(u32));
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FSerializedObject_ {
    FFourCC Type;
    u32 MetaClassIndex;
    u32 NameIndex;
    u32 DataOffset;
};
STATIC_ASSERT(sizeof(FSerializedObject_) == 4 * sizeof(u32));
//----------------------------------------------------------------------------
template <typename T> struct TObjectTraits_ {
    typedef Meta::THash<T> hash_type;
    typedef Meta::TEqualTo<T> equalto_type;
};
template <typename _Char> struct TObjectTraits_<TBasicStringView<_Char>> {
    typedef TStringViewHasher<_Char, ECase::Sensitive> hash_type;
    typedef TStringViewEqualTo<_Char, ECase::Sensitive> equalto_type;
};
//----------------------------------------------------------------------------
template <typename _Key>
class TObjectIndexer_ : TObjectTraits_<_Key> {
public:
    typedef TObjectTraits_<_Key> traits_type;
    typedef typename traits_type::hash_type hash_type;
    typedef typename traits_type::equalto_type equalto_type;
    typedef TNumericDefault<u32, TObjectIndexer_, UINT32_MAX> index_t;
    typedef THashMap<_Key, index_t, hash_type, equalto_type, ALLOCATOR(Binary, TPair<_Key, index_t>)> hashmap_type;

    hashmap_type Entities;

    bool empty() const { return Entities.empty(); }
    size_t size() const { return Entities.size(); }

    bool Insert_ReturnIfExists(index_t* pindex, const _Key& key) {
        index_t& incache = Entities[key];
        if (incache.IsDefaultValue()) {
            *pindex = incache = index_t(checked_cast<typename index_t::value_type>(Entities.size() - 1));
            Assert(not incache.IsDefaultValue());
            return false;
        }
        else {
            *pindex = incache;
            return true;
        }
    }

    index_t IndexOf(const _Key& key) {
        index_t index;
        Insert_ReturnIfExists(&index, key);
        return index;
    }

    index_t IndexOfIFP(const _Key& key) const {
#ifdef WITH_PPE_ASSERT
        const hashmap_type::const_iterator it = Entities.find(key);
        Assert(Entities.end() != it);
        return it->second;
#else
        return Entities[key];
#endif
    }

    template <typename _Lhs, typename _Rhs>
    struct assert_ {
        STATIC_ASSERT(std::is_same<_Lhs, _Rhs>::value);
        enum : bool { value = true };
    };

    template <typename _WriteLambda>
    void Serialize(IBufferedStreamWriter* writer, _WriteLambda&& lambda) const {
        const u32 count = checked_cast<u32>(Entities.size());
        writer->WritePOD(count);
        // THashMap isn't preserving insertion order, need to sort by index before serialization  :
        STACKLOCAL_POD_ARRAY(const _Key*, keysByIndex, count);
        for (const auto& it : Entities)
            keysByIndex[it.second] = &it.first;
        for(const _Key* pkey : keysByIndex)
            lambda(*pkey);
    }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator, typename _Allocator2>
static void Linearize_(TVector< TPair<_Key, _Value>, _Allocator >& linearized, const THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator2 >& hashmap ) {
    linearized.clear();
    linearized.reserve(hashmap.size());
    for (const auto& it : hashmap)
        linearized.push_back(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
static void Linearize_(const TMemoryView< TPair<_Key, _Value> >& linearized, const THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap ) {
    Assert(linearized.size() == hashmap.size());
    size_t i = 0;
    for (const auto& it : hashmap)
        linearized[i] = it;
}
//----------------------------------------------------------------------------
template <typename T>
static void SerializePODs_(IBufferedStreamWriter* writer, const TMemoryView<T>& pods) {
    const u32 n = checked_cast<u32>(pods.size());
    writer->WritePOD(n);

    if (n)
        writer->Write(pods.data(), pods.SizeInBytes());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
static void SerializePODs_(IBufferedStreamWriter* writer, const THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap) {
    typedef TPair<_Key, _Value> pair_type;

    STACKLOCAL_POD_ARRAY(pair_type, linearized, hashmap.size());
    Linearize_(linearized, hashmap);

    std::stable_sort(linearized.begin(), linearized.end(), [](const pair_type& lhs, const pair_type& rhs) {
        return lhs.first < rhs.first; // sort by key
    });

    SerializePODs_(writer, linearized);
}
//----------------------------------------------------------------------------
template <typename T>
bool DeserializePODs_(IBufferedStreamReader& reader, VECTOR(Binary, T)& results) {
    u32 arraySize = UINT32_MAX;
    if (false == reader.ReadPOD(&arraySize))
        return false;

    Assert(UINT32_MAX > arraySize);
    results.clear();

    if (arraySize) {
        results.resize(arraySize);
        if (false == reader.ReadView(results.MakeView()) )
            return false;
    }
    Assert(results.size() == arraySize);

    return true;
}
//----------------------------------------------------------------------------
template <typename _Elt, typename _Result, typename _ReadLambda>
bool DeserializePODArrays_( IBufferedStreamReader& reader,
                            VECTOR(Binary, _Result)& results,
                            _ReadLambda&& lambda ) {
    u32 arraySize = UINT32_MAX;
    if (false == reader.ReadPOD(&arraySize))
        return false;

    Assert(UINT32_MAX > arraySize);
    results.clear();
    results.reserve(arraySize);

    TAllocaBlock<u8> temp;

    forrange(i, 0, arraySize) {
        u32 count = UINT32_MAX;
        if (false == reader.ReadPOD(&count))
            return false;

        TMemoryView<const _Elt> eaten;

        if (count) {
            Assert(UINT32_MAX > count);
            const size_t sizeInBytes = sizeof(_Elt) * count;
            temp.RelocateIFP(sizeInBytes, false);

            if (false == reader.Read(temp.RawData, sizeInBytes))
                return false;

            eaten = temp.MakeView().CutBefore(sizeInBytes).Cast<const _Elt>();
            Assert(eaten.SizeInBytes() == sizeInBytes);
        }

        results.emplace_back(lambda(eaten));
    }

    Assert(results.size() == arraySize);
    return true;
}
//----------------------------------------------------------------------------
static void WriteAlignment_(IBufferedStreamWriter* writer) {
    switch (writer->TellO() & 3) {
    case 1: writer->WritePOD('\xCC');
    case 2: writer->WritePOD('\xCC');
    case 3: writer->WritePOD('\xCC');
    case 0: break;
    }
    Assert(!(writer->TellO() & 3));
}
//----------------------------------------------------------------------------
static void ReadAlignment_(IBufferedStreamReader* reader) {
    switch (reader->TellI() & 3) {
    case 1: Verify(reader->ExpectPOD('\xCC'));
    case 2: Verify(reader->ExpectPOD('\xCC'));
    case 3: Verify(reader->ExpectPOD('\xCC'));
    case 0: break;
    }
    Assert(!(reader->TellI() & 3));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FBinaryDeserialize_ {
public:
    FBinaryDeserialize_(FBinarySerializer* owner) : _owner(owner) {}

    FBinaryDeserialize_(const FBinaryDeserialize_& ) = delete;
    FBinaryDeserialize_& operator =(const FBinaryDeserialize_& ) = delete;

    const FBinarySerializer* Owner() const { return _owner; }

    void Read(IBufferedStreamReader& reader);
    void Finalize(RTTI::FMetaTransaction* transaction);

private:
    static const RTTI::FMetaClass* RetrieveMetaClass_(const FStringView& str);
    static const RTTI::FMetaProperty* RetrieveMetaProperty_(const RTTI::FMetaClass* metaClass, const FStringView& str);

    RTTI::PMetaObject CreateObjectFromHeader_(const FSerializedObject_& header);
    void DeserializeObjectData_(IBufferedStreamReader& reader, RTTI::FMetaObject* object);

    class FAtomReader_ : public RTTI::IAtomVisitor {
    public:
        typedef RTTI::IAtomVisitor parent_type;
        FAtomReader_(FBinaryDeserialize_* owner, IBufferedStreamReader* reader)
            : _owner(owner), _reader(reader) {}


        virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
            BINARYSERIALIZER_LOG(Info, L"Deserialize tuple {0}", tuple->TypeInfos().Name());

#if USE_BINARYSERIALIZER_NAZIFORMAT
            if (false == _reader->ExpectPOD(tuple->TypeId()))
                PPE_THROW_IT(FBinarySerializerException("failed to deserialize a RTTI tuple"));
#endif

            forrange(i, 0, tuple->Arity()) {
                RTTI::FAtom elt = tuple->At(data, i);
                if (not elt.Accept(this))
                    return false;
            }

            return true;
        }

        virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
            BINARYSERIALIZER_LOG(Info, L"Deserialize list {0}", list->TypeInfos().Name());

#if USE_BINARYSERIALIZER_NAZIFORMAT
            if (false == _reader->ExpectPOD(list->TypeId()))
                PPE_THROW_IT(FBinarySerializerException("failed to deserialize a RTTI list"));
#endif
            u32 count = 0;
            if (false == _reader->ReadPOD(&count))
                PPE_THROW_IT(FBinarySerializerException("failed to deserialize a RTTI list"));

            Assert(list->IsEmpty(data));

            list->Reserve(data, count);
            forrange(i, 0, count) {
                RTTI::FAtom item = list->AddDefault(data);
                if (not item.Accept(this))
                    return false;
            }

            return true;
        }

        virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override final {
            BINARYSERIALIZER_LOG(Info, L"Deserialize dico {0}", dico->TypeInfos().Name());

#if USE_BINARYSERIALIZER_NAZIFORMAT
            if (false == _reader->ExpectPOD(dico->TypeId()))
                PPE_THROW_IT(FBinarySerializerException("failed to deserialize a RTTI dico"));
#endif
            u32 count = 0;
            if (false == _reader->ReadPOD(&count))
                PPE_THROW_IT(FBinarySerializerException("failed to deserialize a RTTI dico"));

            Assert(dico->IsEmpty(data));

            dico->Reserve(data, count);

            STACKLOCAL_ATOM(keyData, dico->KeyTraits());
            const RTTI::FAtom keyAtom = keyData.MakeAtom();

            forrange(i, 0, count) {
                if (not keyAtom.Accept(this))
                    return false;

                BINARYSERIALIZER_LOG(Info, L"Add key to dico : {0}", keyAtom);

                const RTTI::FAtom value = dico->AddDefaultMove(data, std::move(keyAtom));
                if (not value.Accept(this))
                    return false;
            }

            return true;
        }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
        virtual bool Visit(const RTTI::IScalarTraits* traits, T& value) override final { \
            return Visit_(traits, value); \
        }
        FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

    private:
        template <typename T>
        FORCE_INLINE bool Visit_(const RTTI::IScalarTraits* traits, T& value) {
#if WITH_RTTI_BINARYSERIALIZER_DATALOG
            const std::streamoff dataoff = _reader->TellI();
#endif

#if USE_BINARYSERIALIZER_NAZIFORMAT
            if (false == _reader->ExpectPOD(traits->TypeId()))
                return false;
#else
            NOOP(traits);
#endif
            if (false == ReadValue_(value))
                return false;

            BINARYSERIALIZER_LOG(Info, L"Deserialize scalar {0} = [{1}]",
                traits->TypeInfos().Name(), value );
            BINARYSERIALIZER_DATALOG(L"@{2:#8X} : <{0}> = [{1}]",
                traits->TypeInfos().Name(), value, dataoff );

            return true;
        }

        template <typename T>
        bool ReadValue_(T& value) {
            STATIC_ASSERT(Meta::TIsPod<T>::value);
            return _reader->ReadPOD(&value);
        }

        template <typename T, size_t _Dim>
        bool ReadValue_(TScalarVector<T, _Dim>& value) {
            STATIC_ASSERT(Meta::TIsPod<T>::value);
            return _reader->Read(&value, sizeof(value));
        }

        template <typename T, size_t _Width, size_t _Height>
        bool ReadValue_(TScalarMatrix<T, _Width, _Height>& value) {
            STATIC_ASSERT(Meta::TIsPod<T>::value);
            return _reader->Read(&value, sizeof(value));
        }

        bool ReadValue_(FString& str) {
            index_t string_i;
            if (false == _reader->ReadPOD(&string_i))
                return false;

            if (string_i.IsDefaultValue()) {
                Assert(str.empty());
                return true;
            }

            if (string_i >= _owner->_strings.size())
                PPE_THROW_IT(FBinarySerializerException("invalid RTTI string index"));

            str = _owner->_strings[string_i];
            return true;
        }

        bool ReadValue_(FWString& wstr) {
            index_t wstring_i;
            if (false == _reader->ReadPOD(&wstring_i))
                return false;

            if (wstring_i.IsDefaultValue()) {
                Assert(wstr.empty());
                return true;
            }

            if (wstring_i >= _owner->_wstrings.size())
                PPE_THROW_IT(FBinarySerializerException("invalid RTTI wstring index"));

            wstr = _owner->_wstrings[wstring_i];
            return true;
        }

        bool ReadValue_(RTTI::FAny& any) {
            RTTI::ENativeType nativeTypeId;
            if (false == _reader->ReadPOD(&nativeTypeId))
                return false;

            if (RTTI::ENativeType::Invalid == nativeTypeId) {
                Assert(not any.Valid());
                return true;
            }

            any = RTTI::FAny(nativeTypeId);
            return any.InnerAtom().Accept(this);
        }

        bool ReadValue_(RTTI::PMetaObject& object) {
            index_t object_i;
            if (false == _reader->ReadPOD(&object_i))
                return false;

            if (object_i.IsDefaultValue()) {
                Assert(not object);
                return true;
            }

            if (object_i >= _owner->_objects.size())
                PPE_THROW_IT(FBinarySerializerException("invalid RTTI object index"));

            object = _owner->_objects[object_i];
            return true;
        }

        bool ReadValue_(RTTI::FName& name) {
            index_t string_i;
            if (false == _reader->ReadPOD(&string_i))
                return false;

            if (string_i.IsDefaultValue()) {
                Assert(name.empty());
                return true;
            }

            if (string_i >= _owner->_strings.size())
                PPE_THROW_IT(FBinarySerializerException("invalid RTTI name index"));

            const FString& data = _owner->_strings[string_i];
            name = RTTI::FName(data.MakeView());
            return true;
        }

        bool ReadValue_(RTTI::FBinaryData& rawdata) {
            u32 rawsize;
            if (_reader->ReadPOD(&rawsize)) {
                if (0 == rawsize) {
                    Assert(rawdata.empty());
                    return true;
                }
                else {
                    rawdata.Resize_DiscardData(checked_cast<size_t>(rawsize));
                    return _reader->Read(rawdata.data(), rawsize);
                }
            }
            else {
                return false;
            }
        }

        bool ReadValue_(FDirpath& dirpath) {
            index_t k;
            if (false == _reader->ReadPOD(&k))
                return false;

            if (0 == k) {
                Assert(dirpath.empty());
                return true;
            }

            STACKLOCAL_ASSUMEPOD_ARRAY(FFileSystemToken, tokenized, k.Value);
            forrange(i, 0, k.Value) {
                index_t wstr_i;
                if (false == _reader->ReadPOD(&wstr_i))
                    return false;

                if (wstr_i >= _owner->_wstrings.size())
                    PPE_THROW_IT(FBinarySerializerException("invalid RTTI dirpath token index"));

                tokenized[i] = FFileSystemToken(_owner->_wstrings[wstr_i].MakeView());
            }

            dirpath.AssignTokens(tokenized);
            return true;
        }

        bool ReadValue_(FFilename& filename) {
            FDirpath dirpath;
            if (not ReadValue_(dirpath))
                return false;

            index_t wstr_i;

            if (false == _reader->ReadPOD(&wstr_i))
                return false;
            if (wstr_i >= _owner->_wstrings.size())
                PPE_THROW_IT(FBinarySerializerException("invalid RTTI basename index"));

            FBasenameNoExt basenameNoExt(_owner->_wstrings[wstr_i].MakeView());

            if (false == _reader->ReadPOD(&wstr_i))
                return false;
            if (wstr_i >= _owner->_wstrings.size())
                PPE_THROW_IT(FBinarySerializerException("invalid RTTI extname index"));

            FExtname extname(_owner->_wstrings[wstr_i].MakeView());

            filename = FFilename(dirpath, basenameNoExt, extname);
            return true;
        }

        FBinaryDeserialize_* _owner;
        IBufferedStreamReader* _reader;
    };

    FBinarySerializer* _owner;

    typedef TNumericDefault<u32, FBinaryDeserialize_, UINT32_MAX> index_t;
    typedef index_t object_index_t;

    typedef VECTOR(Binary, const RTTI::FMetaProperty*) properties_type;

    VECTOR(Binary, FString) _strings;
    VECTOR(Binary, FWString) _wstrings;
    VECTOR(Binary, const RTTI::FMetaClass*) _metaClasses;
    VECTOR(Binary, properties_type) _properties;
    VECTOR(Binary, object_index_t) _topObjects;
    VECTOR(Binary, FSerializedObject_) _headers;
    VECTOR(Binary, RTTI::PMetaObject) _objects;
};
//----------------------------------------------------------------------------
void FBinaryDeserialize_::Read(IBufferedStreamReader& reader) {

    if (false == reader.ExpectPOD(FILE_MAGIC_) )
        PPE_THROW_IT(FBinarySerializerException("invalid file magic"));
    if (false == reader.ExpectPOD(FILE_VERSION_) )
        PPE_THROW_IT(FBinarySerializerException("unsupported file version"));

    if (false == reader.ExpectPOD(SECTION_STRINGS_) ||
        false == DeserializePODArrays_<char>(reader, _strings, [](const FStringView& str) { return ToString(str); }) )
        PPE_THROW_IT(FBinarySerializerException("invalid strings section"));

    ReadAlignment_(&reader); // keep sections aligned on u32

    if (false == reader.ExpectPOD(SECTION_WSTRINGS_) ||
        false == DeserializePODArrays_<wchar_t>(reader, _wstrings, [](const FWStringView& wstr) { return ToWString(wstr); }) )
        PPE_THROW_IT(FBinarySerializerException("invalid wstrings section"));

    ReadAlignment_(&reader); // keep sections aligned on u32

    if (false == reader.ExpectPOD(SECTION_CLASSES_) ||
        false == DeserializePODArrays_<char>(reader, _metaClasses, RetrieveMetaClass_) )
        PPE_THROW_IT(FBinarySerializerException("expected classes section"));

    ReadAlignment_(&reader); // keep sections aligned on u32

    if (false == reader.ExpectPOD(SECTION_PROPERTIES_) )
        PPE_THROW_IT(FBinarySerializerException("expected properties section"));

    _properties.resize(_metaClasses.size());
    forrange(i, 0, _properties.size()) {
        const RTTI::FMetaClass* metaClass = _metaClasses[i];
        properties_type& properties = _properties[i];

        const bool succeed = DeserializePODArrays_<char>(reader, properties, [metaClass](const FStringView& str) {
            return RetrieveMetaProperty_(metaClass, str);
        });

        if (false == succeed)
            PPE_THROW_IT(FBinarySerializerException("invalid RTTI property"));
    }

    ReadAlignment_(&reader); // keep sections aligned on u32

    if (false == reader.ExpectPOD(SECTION_TOP_OBJECTS_) ||
        false == DeserializePODs_(reader, _topObjects) )
        PPE_THROW_IT(FBinarySerializerException("expected top objects section"));

    ReadAlignment_(&reader); // keep sections aligned on u32

    if (false == reader.ExpectPOD(SECTION_OBJECTS_HEADER_) ||
        false == DeserializePODs_(reader, _headers))
        PPE_THROW_IT(FBinarySerializerException("expected objects header section"));

    // first pass : create or import every object in the file

    ReadAlignment_(&reader); // keep sections aligned on u32

    _objects.reserve(_headers.size());
    for (const FSerializedObject_& header : _headers)
        _objects.emplace_back(CreateObjectFromHeader_(header));

    u64 dataSizeInBytes = 0;
    if (false == reader.ExpectPOD(SECTION_OBJECTS_DATA_) ||
        false == reader.ReadPOD(&dataSizeInBytes) )
        PPE_THROW_IT(FBinarySerializerException("expected objects data section"));

    const std::streamoff dataBegin = reader.TellI();
    const std::streamoff dataEnd = checked_cast<std::streamoff>(dataBegin + dataSizeInBytes);

    // second pass : read every object properties

    ReadAlignment_(&reader); // keep sections aligned on u32

    forrange(i, 0, _headers.size()) {
        const FSerializedObject_& header = _headers[i];
        if (header.Type == TAG_OBJECT_NULL_ ||
            header.Type == TAG_OBJECT_IMPORT_ )
            continue;

        const RTTI::PMetaObject& object = _objects[i];
        Assert(object);
        Assert(dataBegin + header.DataOffset == reader.TellI());

        DeserializeObjectData_(reader, object.get());
    }

    if (reader.TellI() != dataEnd)
        PPE_THROW_IT(FBinarySerializerException("object section has unread data"));

    if (false == reader.ExpectPOD(SECTION_END_) )
        PPE_THROW_IT(FBinarySerializerException("expected end section"));
}
//----------------------------------------------------------------------------
void FBinaryDeserialize_::Finalize(RTTI::FMetaTransaction* transaction) {
    Assert(transaction);
    Assert(_topObjects.size() || 0 == _objects.size());

    transaction->reserve(transaction->TopObjects().size() + _topObjects.size());
    for (const object_index_t& object_i : _topObjects)
        transaction->RegisterObject(_objects[object_i].get());
}
//----------------------------------------------------------------------------
const RTTI::FMetaClass* FBinaryDeserialize_::RetrieveMetaClass_(const FStringView& str) {
    const RTTI::FName name(str);
    Assert(!name.empty());

    const RTTI::FMetaClass* metaClass = RTTI::MetaDB().ClassIFP(name);
    if (nullptr == metaClass)
        PPE_THROW_IT(FBinarySerializerException("unknown RTTI metaclass"));

    return metaClass;
}
//----------------------------------------------------------------------------
const RTTI::FMetaProperty* FBinaryDeserialize_::RetrieveMetaProperty_(const RTTI::FMetaClass* metaClass, const FStringView& str) {
    Assert(metaClass);

    const RTTI::FName name(str);
    Assert(!name.empty());

    const RTTI::FMetaProperty* metaProperty = metaClass->PropertyIFP(name);
    if (nullptr == metaProperty)
        PPE_THROW_IT(FBinarySerializerException("unknown RTTI property"));

    return metaProperty;
}
//----------------------------------------------------------------------------
RTTI::PMetaObject FBinaryDeserialize_::CreateObjectFromHeader_(const FSerializedObject_& header) {
    RTTI::PMetaObject result;

    if (TAG_OBJECT_NULL_ == header.Type) {
        Assert(UINT32_MAX == header.MetaClassIndex);
        Assert(UINT32_MAX == header.NameIndex);
        Assert(UINT32_MAX == header.DataOffset);

        BINARYSERIALIZER_LOG(Info, L"Deserialize null object");
    }
    else if (TAG_OBJECT_PRIVATE_ == header.Type) {
        Assert(UINT32_MAX != header.MetaClassIndex);
        Assert(UINT32_MAX == header.NameIndex);
        Assert(UINT32_MAX != header.DataOffset);

        const RTTI::FMetaClass* metaClass = _metaClasses[header.MetaClassIndex];

        BINARYSERIALIZER_LOG(Info, L"Deserialize private object <{0}>", metaClass->Name());

        if (not metaClass->CreateInstance(result))
            PPE_THROW_IT(FBinarySerializerException("can't create instance of RTTI metaclass"));
    }
    else {
        Assert(UINT32_MAX != header.MetaClassIndex);
        Assert(UINT32_MAX != header.NameIndex);

        const RTTI::FMetaClass* metaClass = _metaClasses[header.MetaClassIndex];

        const RTTI::FName name = RTTI::FName(_strings[header.NameIndex].MakeView());

        if (TAG_OBJECT_EXPORT_ == header.Type) {
            Assert(UINT32_MAX != header.DataOffset);

            BINARYSERIALIZER_LOG(Info, L"Deserialize exported object <{0}> '{1}'", metaClass->Name(), name);

            if (not metaClass->CreateInstance(result))
                PPE_THROW_IT(FBinarySerializerException("can't create instance of RTTI metaclass"));

            result->RTTI_Export(name);
        }
        else {
            Assert(UINT32_MAX != header.DataOffset);
            Assert(TAG_OBJECT_IMPORT_ == header.Type);

            const RTTI::FName transaction = RTTI::FName(_strings[header.DataOffset].MakeView());
            const RTTI::FPathName pathName{ transaction, name };

            BINARYSERIALIZER_LOG(Info, L"Deserialize imported object <{0}> '{1}'", metaClass->Name(), pathName);

            result = RTTI::MetaDB().ObjectIFP(pathName);
            if (nullptr == result)
                PPE_THROW_IT(FBinarySerializerException("failed to import RTTI object from database"));
        }

        Assert(result->RTTI_Name() == name);
    }

    return result;
}
//----------------------------------------------------------------------------
void FBinaryDeserialize_::DeserializeObjectData_(IBufferedStreamReader& reader, RTTI::FMetaObject* object) {
    u32 metaClassCount = 0;

#if USE_BINARYSERIALIZER_NAZIFORMAT
    if (false == reader.ExpectPOD(TAG_OBJECT_START_))
        PPE_THROW_IT(FBinarySerializerException("failed to read RTTI object header"));
#endif
    if (false == reader.ReadPOD(&metaClassCount))
        PPE_THROW_IT(FBinarySerializerException("failed to read RTTI object header"));

    BINARYSERIALIZER_LOG(Info, L"Deserialize object data for @{0}", reader.TellI());

    FAtomReader_ atomReader(this, &reader);

    forrange(i, 0, metaClassCount) {
        index_t class_i;
        u32 propertyCount = 0;

#if USE_BINARYSERIALIZER_NAZIFORMAT
        if (false == reader.ExpectPOD(TAG_OBJECT_METACLASS_))
            PPE_THROW_IT(FBinarySerializerException("failed to read RTTI object metaclass"));
#endif
        if (false == reader.ReadPOD(&class_i) ||
            false == reader.ReadPOD(&propertyCount) )
            PPE_THROW_IT(FBinarySerializerException("failed to read RTTI object metaclass"));

        Assert(0 < propertyCount);

        if (class_i >= _metaClasses.size())
            PPE_THROW_IT(FBinarySerializerException("invalid RTTI metaclass index"));
        Assert(_metaClasses.size() == _properties.size());

#if WITH_RTTI_BINARYSERIALIZER_LOG
        const RTTI::FMetaClass* metaClass = _metaClasses[class_i];
#endif

        const properties_type& properties = _properties[class_i];

        BINARYSERIALIZER_LOG(Info, L"Deserialize metaclass data <{0}>", metaClass->Name());

        forrange(j, 0, propertyCount) {
            index_t prop_i;
            if (false == reader.ReadPOD(&prop_i) ||
                prop_i > properties.size() )
                PPE_THROW_IT(FBinarySerializerException("invalid RTTI property index"));

            const RTTI::FMetaProperty* metaProperty = properties[prop_i];

            BINARYSERIALIZER_LOG(Info, L"Deserialize property data <{0}> '{1}'",
                metaProperty->Traits()->TypeInfos().Name(), metaProperty->Name());

            const RTTI::FAtom atom = metaProperty->Get(*object);
            Assert(atom);
            Assert(atom.IsDefaultValue());

            atom.Accept(&atomReader);
            Assert(not atom.IsDefaultValue());
        }
    }

#if USE_BINARYSERIALIZER_NAZIFORMAT
    if (false == reader.ExpectPOD(TAG_OBJECT_END_))
        PPE_THROW_IT(FBinarySerializerException("failed to read RTTI object foorter"));
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FBinarySerialize_ {
public:
    typedef TObjectIndexer_<const RTTI::FMetaProperty* > property_indices_type;
    typedef TObjectIndexer_<const RTTI::FMetaClass* > class_indices_type;
    typedef TObjectIndexer_<const RTTI::FMetaObject* > object_indices_type;
    typedef TObjectIndexer_<FStringView> string_indices_type;
    typedef TObjectIndexer_<FWStringView> wstring_indices_type;

    typedef property_indices_type::index_t property_index_t;
    typedef class_indices_type::index_t class_index_t;
    typedef object_indices_type::index_t object_index_t;
    typedef string_indices_type::index_t string_index_t;
    typedef wstring_indices_type::index_t wstring_index_t;

    FBinarySerialize_(const FBinarySerializer* owner, const RTTI::FMetaTransaction* transaction)
        : _owner(owner), _transaction(transaction) {
        Assert(owner);
        Assert(transaction);
    }

    FBinarySerialize_(const FBinarySerialize_& ) = delete;
    FBinarySerialize_& operator =(const FBinarySerialize_& ) = delete;

    const FBinarySerializer* Owner() const { return _owner; }

    void Append(const RTTI::FMetaObject* object, bool topObject = true);
    void Append(const TMemoryView<const RTTI::PMetaObject>& objects, bool topObject = true);
    void Finalize(IBufferedStreamWriter* writer);

private:
    class FAtomWriter_ : public RTTI::IAtomVisitor {
    public:
        typedef RTTI::IAtomVisitor parent_type;
        FAtomWriter_(FBinarySerialize_* owner) : _owner(owner) {}

        template <typename T>
        void WritePOD(const T& pod) { _owner->_objectStream.WritePOD(pod); }
        void WriteRaw(const void* ptr, size_t sizeInBytes) { _owner->_objectStream.Write(ptr, sizeInBytes); }

        virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
            BINARYSERIALIZER_LOG(Info, L"Serialize tuple {0}", tuple->TypeInfos().Name());

#if USE_BINARYSERIALIZER_NAZIFORMAT
            WritePOD(tuple->TypeId());
#endif

            return tuple->ForEach(data, [this](const RTTI::FAtom& elt) {
                return elt.Accept(this);
            });
        }

        virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
            BINARYSERIALIZER_LOG(Info, L"Serialize list {0}", list->TypeInfos().Name());

#if USE_BINARYSERIALIZER_NAZIFORMAT
            WritePOD(list->TypeId());
#endif
            WritePOD(checked_cast<u32>(list->Count(data)));

            return list->ForEach(data, [this](const RTTI::FAtom& item) {
                return item.Accept(this);
            });
        }

        virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override final {
            BINARYSERIALIZER_LOG(Info, L"Serialize dico {0}", dico->TypeInfos().Name());

#if USE_BINARYSERIALIZER_NAZIFORMAT
            WritePOD(dico->TypeId());
#endif
            WritePOD(checked_cast<u32>(dico->Count(data)));

            return dico->ForEach(data, [this](const RTTI::FAtom& key, const RTTI::FAtom& value) {
                return (key.Accept(this) && value.Accept(this));
            });
        }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
        virtual bool Visit(const RTTI::IScalarTraits* traits, T& value) override final { \
            return Write_(traits, value); \
        }
        FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

    private:
        template <typename T>
        bool Write_(const RTTI::IScalarTraits* scalar, const T& value) {
            BINARYSERIALIZER_LOG(Info, L"Serialize scalar {0} = [{1}]",
                scalar->TypeInfos().Name(), value );
            BINARYSERIALIZER_DATALOG(L"@{2:#8X} : <{0}> = [{1}]",
                scalar->TypeInfos().Name(), value, _owner->_objectStream.TellO() );

#if USE_BINARYSERIALIZER_NAZIFORMAT
            WritePOD(scalar->TypeId());
#else
            NOOP(scalar);
#endif
            WriteValue_(value);

            return true;
        }

        template <typename T>
        void WriteValue_(const T& value) {
            STATIC_ASSERT(Meta::TIsPod<T>::value);
            WritePOD(value);
        }

        template <typename T, size_t _Dim>
        void WriteValue_(const TScalarVector<T, _Dim>& value) {
            STATIC_ASSERT(Meta::TIsPod<T>::value);
            WriteRaw(&value, sizeof(value));
        }

        template <typename T, size_t _Width, size_t _Height>
        void WriteValue_(const TScalarMatrix<T, _Width, _Height>& value) {
            STATIC_ASSERT(Meta::TIsPod<T>::value);
            WriteRaw(&value, sizeof(value));
        }

        void WriteValue_(const FString& str) {
            const string_index_t string_i = _owner->_stringIndices.IndexOf(MakeStringView(str));
            WritePOD(string_i);
        }

        void WriteValue_(const FWString& wstr) {
            const wstring_index_t wstring_i = _owner->_wstringIndices.IndexOf(MakeStringView(wstr));
            WritePOD(wstring_i);
        }

        void WriteValue_(const RTTI::FAny& any) {
            if (any) {
                WritePOD(any.Traits()->TypeId());
                if (not any.InnerAtom().Accept(this))
                    AssertNotReached();
            }
            else {
                WritePOD(RTTI::ENativeType::Invalid);
            }
        }

        void WriteValue_(const RTTI::PMetaObject& object) {
            if (object) {
                const object_index_t object_i = _owner->AddObject_(object.get());
                WritePOD(object_i);
            }
            else
                WritePOD(UINT32_MAX);
        }

        void WriteValue_(const RTTI::FName& name) {
            const string_index_t string_i = _owner->_stringIndices.IndexOf(name.MakeView());
            WritePOD(string_i);
        }

        void WriteValue_(const RTTI::FBinaryData& rawdata) {
            WritePOD(checked_cast<u32>(rawdata.size()));
            if (rawdata.size())
                WriteRaw(rawdata.data(), rawdata.SizeInBytes());
        }

        void WriteValue_(const FDirpath& dirpath) {
            const size_t k = dirpath.Depth();
            if (0 == k) {
                WriteValue_(0);
                return;
            }

            STACKLOCAL_ASSUMEPOD_ARRAY(FFileSystemToken, tokenized, k);
            dirpath.ExpandTokens(tokenized);

            WritePOD(checked_cast<u32>(k));

            for (const FFileSystemToken& token : tokenized) {
                const wstring_index_t wstr_i = _owner->_wstringIndices.IndexOf(token.MakeView());
                WritePOD(wstr_i);
            }
        }

        void WriteValue_(const FFilename& filename) {
            WriteValue_(filename.Dirpath());

            wstring_index_t wstr_i;
            FWStringView wstr;

            wstr_i = _owner->_wstringIndices.IndexOf(filename.BasenameNoExt().MakeView());
            WritePOD(wstr_i);

            wstr_i = _owner->_wstringIndices.IndexOf(filename.Extname().MakeView());
            WritePOD(wstr_i);
        }

        FBinarySerialize_* _owner;
    };

    class_index_t AddClass_(const RTTI::FMetaClass* metaClass);
    object_index_t AddObject_(const RTTI::FMetaObject* object);
    void ProcessQueue_();
    void WriteObject_(FAtomWriter_& atomWriter, const RTTI::FMetaObject* object, const RTTI::FMetaClass* metaClass);

    const FBinarySerializer* _owner;
    const RTTI::FMetaTransaction* _transaction;

    MEMORYSTREAM(Binary) _objectStream;
    DEQUE(Binary, RTTI::SCMetaObject) _objectQueue;

    STATIC_ASSERT(sizeof(FFourCC) == sizeof(property_index_t));
    STATIC_ASSERT(sizeof(FFourCC) == sizeof(class_index_t));
    STATIC_ASSERT(sizeof(FFourCC) == sizeof(object_index_t));
    STATIC_ASSERT(sizeof(FFourCC) == sizeof(string_index_t));
    STATIC_ASSERT(sizeof(FFourCC) == sizeof(wstring_index_t));

    string_indices_type _stringIndices;
    wstring_indices_type _wstringIndices;

    class_indices_type _classIndices;
    VECTOR(Binary, property_indices_type) _propertiesByClassIndex;

    object_indices_type _objectIndices;
    VECTOR(Binary, object_index_t) _topObjects;
    VECTOR(Binary, FSerializedObject_) _objectsStreamed;
};
//----------------------------------------------------------------------------
void FBinarySerialize_::Append(const RTTI::FMetaObject* object, bool topObject /* = true */) {
    Assert(false == topObject || nullptr != object);
    const object_index_t object_i = AddObject_(object);

    if (topObject) {
        AssertRelease(object);
        _topObjects.push_back(object_i);
    }

    ProcessQueue_();
}
//----------------------------------------------------------------------------
void FBinarySerialize_::Append(const TMemoryView<const RTTI::PMetaObject>& objects, bool topObject /* = true */) {
    for (const RTTI::PMetaObject& object : objects) {
        const object_index_t object_i = AddObject_(object.get());

        if (topObject) {
            AssertRelease(object);
            _topObjects.push_back(object_i);
        }
    }

    ProcessQueue_();
}
//----------------------------------------------------------------------------
void FBinarySerialize_::Finalize(IBufferedStreamWriter* writer) {
    Assert(writer);

    // TODO: merge objects when metaclass allows it (separated preprocess ?)

    writer->WritePOD(FILE_MAGIC_);
    writer->WritePOD(FILE_VERSION_);

    writer->WritePOD(SECTION_STRINGS_);
    _stringIndices.Serialize(writer, [writer](const FStringView& str) {
        SerializePODs_(writer, str);
    });

    WriteAlignment_(writer); // keep sections aligned on u32

    writer->WritePOD(SECTION_WSTRINGS_);
    _wstringIndices.Serialize(writer, [writer](const FWStringView& wstr) {
        SerializePODs_(writer, wstr);
    });

    WriteAlignment_(writer); // keep sections aligned on u32

    writer->WritePOD(SECTION_CLASSES_);
    _classIndices.Serialize(writer, [writer](const RTTI::FMetaClass* metaClass) {
        SerializePODs_(writer, metaClass->Name().MakeView());
    });

    WriteAlignment_(writer); // keep sections aligned on u32

    writer->WritePOD(SECTION_PROPERTIES_);
    AssertRelease(_propertiesByClassIndex.size() == _classIndices.size());
    for (const property_indices_type& properties : _propertiesByClassIndex) {
        properties.Serialize(writer, [writer](const RTTI::FMetaProperty* property) {
            SerializePODs_(writer, property->Name().MakeView());
        });
    }

    WriteAlignment_(writer); // keep sections aligned on u32

    writer->WritePOD(SECTION_TOP_OBJECTS_);
    SerializePODs_(writer, MakeConstView(_topObjects));

    WriteAlignment_(writer); // keep sections aligned on u32

    writer->WritePOD(SECTION_OBJECTS_HEADER_);
    AssertRelease(_objectsStreamed.size() == _objectIndices.size());
    SerializePODs_(writer, MakeConstView(_objectsStreamed));

    WriteAlignment_(writer); // keep sections aligned on u32

    writer->WritePOD(SECTION_OBJECTS_DATA_);
    writer->WritePOD(checked_cast<u64>(_objectStream.SizeInBytes()));

    ONLY_IF_ASSERT(const std::streamoff dataEnd = writer->TellO() + checked_cast<std::streamoff>(_objectStream.SizeInBytes()));
    writer->Write(_objectStream.Pointer(), _objectStream.SizeInBytes());

    Assert_NoAssume(writer->TellO() == dataEnd);
    writer->WritePOD(SECTION_END_);
}
//----------------------------------------------------------------------------
auto FBinarySerialize_::AddClass_(const RTTI::FMetaClass* metaClass) -> class_index_t {
    Assert(metaClass);
    class_index_t class_i;
    if (false == _classIndices.Insert_ReturnIfExists(&class_i, metaClass)) {
        Assert(_propertiesByClassIndex.size() + 1 == _classIndices.size());
        _propertiesByClassIndex.emplace_back();
    }
    Assert(not class_i.IsDefaultValue());
    return class_i;
}
//----------------------------------------------------------------------------
auto FBinarySerialize_::AddObject_(const RTTI::FMetaObject* object) -> object_index_t {
    object_index_t object_i;
    const bool alreadyStreamed = _objectIndices.Insert_ReturnIfExists(&object_i, object);
    if (false == alreadyStreamed)
        _objectQueue.push_back(object);

    Assert(not object_i.IsDefaultValue());
    return object_i;
}
//----------------------------------------------------------------------------
void FBinarySerialize_::ProcessQueue_() {
    FAtomWriter_ atomWriter(this);

    _objectsStreamed.reserve(_objectIndices.size());

    while (!_objectQueue.empty()) {
        const RTTI::SCMetaObject object = _objectQueue.front();
        _objectQueue.pop_front();

        const object_index_t object_i = _objectIndices.IndexOf(object);

        _objectsStreamed.emplace_back();
        Assert(_objectsStreamed.size() == object_i + 1);

        FSerializedObject_& header = _objectsStreamed.back();
        header.Type = FFourCC();
        header.MetaClassIndex = UINT32_MAX;
        header.NameIndex = UINT32_MAX;
        header.DataOffset = UINT32_MAX;

        if (nullptr == object) {
            BINARYSERIALIZER_LOG(Info, L"Serialize null object");
            header.Type = TAG_OBJECT_NULL_;
        }
        else {
            Assert(object->RTTI_Outer());

            const RTTI::FMetaClass* metaClass = object->RTTI_Class();
            Assert(metaClass);

            const class_index_t class_i = AddClass_(metaClass);
            header.MetaClassIndex = class_i;

            if (object->RTTI_IsExported()) {

                const RTTI::FName metaName = object->RTTI_Name();
                Assert(!metaName.empty());

                const string_index_t name_i = _stringIndices.IndexOf(metaName.MakeView());

                const bool exported = (object->RTTI_Outer() == _transaction);
                if (exported) {
                    BINARYSERIALIZER_LOG(Info, L"Serialize exported object <{0}> '{1}'", metaClass->Name(), metaName);
                    header.Type = TAG_OBJECT_EXPORT_;
                    header.NameIndex = name_i;
                }
                else {
                    BINARYSERIALIZER_LOG(Info, L"Serialize imported object <{0}> '{1}'", metaClass->Name(), metaName);
                    header.Type = TAG_OBJECT_IMPORT_;
                    header.NameIndex = name_i;
                    header.DataOffset = _stringIndices.IndexOf(object->RTTI_Outer()->Name().MakeView()); // pack transaction name index in unused DataOffset
                    continue; // skip serialization because it does not belong to the current transaction
                }
            }
            else {
                BINARYSERIALIZER_LOG(Info, L"Serialize private object <{0}>", object->RTTI_Class()->Name());
                header.Type = TAG_OBJECT_PRIVATE_;

                Assert(object->RTTI_Outer() == _transaction);
            }

            header.DataOffset = checked_cast<u32>(_objectStream.TellO());

            WriteObject_(atomWriter, object, metaClass);
        }

        Assert(FFourCC() != header.Type);
    }
}
//----------------------------------------------------------------------------
void FBinarySerialize_::WriteObject_(FAtomWriter_& atomWriter, const RTTI::FMetaObject* object, const RTTI::FMetaClass* metaClass) {
    Assert(object);
    Assert(object->RTTI_Class() == metaClass);

    BINARYSERIALIZER_LOG(Info, L"Serialize object data for @{0}", _objectStream.TellI());

#if USE_BINARYSERIALIZER_NAZIFORMAT
    _objectStream.WritePOD(TAG_OBJECT_START_);
#endif

    u32 metaClassCount = 0;
    const std::streamoff metaClassCountOffset = _objectStream.TellO();
    _objectStream.WritePOD(metaClassCount); // will be overwritten at the end of function

    while (metaClass) {
        BINARYSERIALIZER_LOG(Info, L"Serialize metaclass data <{0}>", metaClass->Name());

        const std::streamoff metaClassTagOffset = _objectStream.TellO();

#if USE_BINARYSERIALIZER_NAZIFORMAT
        _objectStream.WritePOD(TAG_OBJECT_METACLASS_);
#endif

        const class_index_t class_i = AddClass_(metaClass);
        _objectStream.WritePOD(class_i);

        u32 propertyCount = 0;
        const std::streamoff propertyCountOffset = _objectStream.TellO();
        _objectStream.WritePOD(propertyCount); // will be overwritten at the end of the loop

        property_indices_type& propertyIndices = _propertiesByClassIndex[class_i];

        for (const RTTI::FMetaProperty& prop : metaClass->SelfProperties()) {
            RTTI::FAtom atom = prop.Get(*object);

            if (atom.IsDefaultValue())
                continue;

            BINARYSERIALIZER_LOG(Info, L"Serialize property data <{0}> '{1}'",
                prop.Traits()->TypeInfos().Name(), prop.Name());

            ++propertyCount;

            const property_index_t prop_i = propertyIndices.IndexOf(&prop);
            _objectStream.WritePOD(prop_i);

            if (not atom.Accept(&atomWriter))
                AssertNotReached();
        }

        if (0 < propertyCount) {
            ++metaClassCount;

            const std::streamoff propertyEndOffset = _objectStream.TellO();
            _objectStream.SeekO(propertyCountOffset, ESeekOrigin::Begin);
            _objectStream.WritePOD(propertyCount); // overwrite property count
            _objectStream.SeekO(propertyEndOffset, ESeekOrigin::Begin);
        }
        else {
            _objectStream.resize(checked_cast<size_t>(metaClassTagOffset)); // erase empty metaclass
            Assert(_objectStream.TellO() == metaClassTagOffset);
        }

        metaClass = metaClass->Parent();
    }

    const std::streamoff objectEndOffset = _objectStream.TellO();
    _objectStream.SeekO(metaClassCountOffset, ESeekOrigin::Begin);
    _objectStream.WritePOD(metaClassCount); // overwrite metaclass count
    _objectStream.SeekO(objectEndOffset, ESeekOrigin::Begin);

#if USE_BINARYSERIALIZER_NAZIFORMAT
    _objectStream.WritePOD(TAG_OBJECT_END_);
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBinarySerializer::FBinarySerializer() {}
//----------------------------------------------------------------------------
FBinarySerializer::~FBinarySerializer() {}
//----------------------------------------------------------------------------
void FBinarySerializer::DeserializeImpl(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName) {
    Assert(input);
    UNUSED(sourceName);

    FBinaryDeserialize_ deserialize(this);

    UsingBufferedStream(input, [&deserialize](IBufferedStreamReader* buffered) {
        deserialize.Read(*buffered);
    });

    deserialize.Finalize(transaction);
}
//----------------------------------------------------------------------------
void FBinarySerializer::SerializeImpl(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);

    FBinarySerialize_ serialize(this, transaction);
    serialize.Append(transaction->TopObjects());

    UsingBufferedStream(output, [&serialize](IBufferedStreamWriter* buffered) {
        serialize.Finalize(buffered);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE