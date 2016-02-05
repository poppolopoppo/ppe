#include "stdafx.h"

#include "BinarySerializer.h"

#include "Core/Container/Deque.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/Vector.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/Memory/MemoryStream.h"
#include "Core/Memory/MemoryViewReader.h"
#include "Core/Misc/FourCC.h"
#include "Core/Meta/StronglyTyped.h"

#include "Core.RTTI/RTTI_fwd.h"
#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaAtomDatabase.h"
#include "Core.RTTI/MetaAtomVisitor.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaClassDatabase.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaTransaction.h"
#include "Core.RTTI/MetaType.h"

#include "Core.RTTI/MetaType.Definitions-inl.h"

#ifdef USE_DEBUG_LOGGER
#   define WITH_RTTI_BINARYSERIALIZER_LOG       0 //%_NOCOMMIT%
#   define WITH_RTTI_BINARYSERIALIZER_DATALOG   0 //%_NOCOMMIT%

#   if WITH_RTTI_BINARYSERIALIZER_LOG
#       define BINARYSERIALIZER_LOG(...) LOG(__VA_ARGS__)
#   else
#       define BINARYSERIALIZER_LOG(...) NOOP
#   endif

#   if WITH_RTTI_BINARYSERIALIZER_DATALOG
#       define BINARYSERIALIZER_DATALOG(...) LOG(Warning, __VA_ARGS__)
#   else
#       define BINARYSERIALIZER_DATALOG(...) NOOP
#   endif
#else
#   define BINARYSERIALIZER_LOG(...) NOOP
#   define BINARYSERIALIZER_DATALOG(...) NOOP
#endif

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Binary file format
//----------------------------------------------------------------------------
static const FourCC FILE_MAGIC_                 ("BINA");
static const FourCC FILE_VERSION_               ("1.00");
//----------------------------------------------------------------------------
static const FourCC SECTION_NAMES_              ("#NME");
static const FourCC SECTION_STRINGS_            ("#STR");
static const FourCC SECTION_WSTRINGS_           ("#WST");
static const FourCC SECTION_CLASSES_            ("#CLS");
static const FourCC SECTION_PROPERTIES_         ("#PRP");
static const FourCC SECTION_TOP_OBJECTS_        ("#TOP");
static const FourCC SECTION_OBJECTS_TO_EXPORT_  ("#EXP");
static const FourCC SECTION_OBJECTS_HEADER_     ("#OBH");
static const FourCC SECTION_OBJECTS_DATA_       ("#OBD");
static const FourCC SECTION_END_                ("#END");
//----------------------------------------------------------------------------
static const FourCC TAG_OBJECT_EXPORT_          ("OEXP");
static const FourCC TAG_OBJECT_IMPORT_          ("OIMP");
static const FourCC TAG_OBJECT_PRIVATE_         ("OPRI");
static const FourCC TAG_OBJECT_NULL_            ("ONUL");
static const FourCC TAG_OBJECT_START_           ("OSTA");
static const FourCC TAG_OBJECT_METACLASS_       ("OMTC");
static const FourCC TAG_OBJECT_END_             ("OEND");
//----------------------------------------------------------------------------
static const FourCC TAG_ATOM_SCALAR_            ("ASCR");
static const FourCC TAG_ATOM_PAIR_              ("APAR");
static const FourCC TAG_ATOM_VECTOR_            ("AVEC");
static const FourCC TAG_ATOM_DICTIONARY_        ("ADIC");
static const FourCC TAG_ATOM_ATOM_              ("ATOM");
static const FourCC TAG_ATOM_NULL_              ("ANUL");
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct SerializedObject_ {
    FourCC Type;
    u32 MetaClassIndex;
    u32 NameIndex;
    u32 DataOffset;
};
//----------------------------------------------------------------------------
template <typename T> struct ObjectTraits_  { T proxy_(const T& p) const { return p; } };
template <> struct ObjectTraits_< String >  { StringSlice proxy_(const String& s) const { return MakeStringSlice(s); } };
template <> struct ObjectTraits_< WString > { WStringSlice proxy_(const WString& s) const { return MakeStringSlice(s); } };
//----------------------------------------------------------------------------
template <typename _Key>
class ObjectIndexer_ : ObjectTraits_<_Key> {
public:
    typedef ObjectTraits_<_Key> traits_type;
    typedef StronglyTyped::Numeric<u32, ObjectIndexer_, UINT32_MAX> index_t;
    typedef HASHMAP_THREAD_LOCAL(Serialize, _Key, index_t) hashmap_type;

    hashmap_type Entities;

    bool empty() const { return Entities.empty(); }
    size_t size() const { return Entities.size(); }

    bool Insert_ReturnIfExists(index_t* pindex, const _Key& key) {
        index_t& incache = Entities[key];
        if (index_t::DefaultValue == incache) {
            *pindex = incache = index_t(checked_cast<typename index_t::value_type>(Entities.size() - 1));
            Assert(index_t::DefaultValue != incache);
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
#ifdef WITH_CORE_ASSERT
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
    void Serialize(IStreamWriter* writer, _WriteLambda&& lambda) const {
        const u32 count = checked_cast<u32>(Entities.size());
        writer->WritePOD(count);
        typedef decltype(traits_type::proxy_(std::declval<const _Key&>())) proxy_type;
        STACKLOCAL_POD_ARRAY(proxy_type, proxies, count);
        for (const auto& it : Entities)
            proxies[it.second] = traits_type::proxy_(it.first);
        for (const proxy_type& proxy : proxies)
            lambda(proxy);
    }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator, typename _Allocator2>
static void Linearize_(Vector< Pair<_Key, _Value>, _Allocator >& linearized, const HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator2 >& hashmap ) {
    linearized.clear();
    linearized.reserve(hashmap.size());
    for (const auto& it : hashmap)
        linearized.push_back(it);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
static void Linearize_(const MemoryView< Pair<_Key, _Value> >& linearized, const HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap ) {
    Assert(linearized.size() == hashmap.size());
    size_t i = 0;
    for (const auto& it : hashmap)
        linearized[i] = it;
}
//----------------------------------------------------------------------------
template <typename T>
static void SerializePODs_(IStreamWriter* writer, const MemoryView<T>& pods) {
    const u32 n = checked_cast<u32>(pods.size());
    writer->WritePOD(n);
    if (n)
        writer->WriteSome(pods.Pointer(), sizeof(T), n);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
static void SerializePODs_(IStreamWriter* writer, const HashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>& hashmap) {
    typedef Pair<_Key, _Value> pair_type;
    STACKLOCAL_POD_ARRAY(pair_type, linearized, hashmap.size());
    Linearize_(linearized, hashmap);
    std::stable_sort(linearized.begin(), linearized.end(), [](const pair_type& lhs, const pair_type& rhs) {
        return lhs.first < rhs.first; // sort by key
    });
    SerializePODs_(writer, linearized);
}
//----------------------------------------------------------------------------
template <typename T>
bool DeserializePODs_(MemoryViewReader& reader, VECTOR_THREAD_LOCAL(Serialize, T)& results) {
    u32 arraySize = UINT32_MAX;
    if (false == reader.ReadPOD(&arraySize))
        return false;

    Assert(UINT32_MAX > arraySize);
    results.clear();

    if (arraySize) {
        MemoryView<const u8> eaten;
        if (false == reader.EatIFP(&eaten, sizeof(T)*arraySize) )
            return false;

        Assert(eaten.SizeInBytes() == sizeof(T)*arraySize);
        const MemoryView<const T> src = eaten.Cast<const T>();
        Assert(src.size() == arraySize);

        results.assign(src.begin(), src.end());
    }

    Assert(results.size() == arraySize);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Elt, typename _Result, typename _ReadLambda>
bool DeserializePODArrays_( MemoryViewReader& reader,
                            VECTOR_THREAD_LOCAL(Serialize, _Result)& results,
                            _ReadLambda&& lambda ) {
    u32 arraySize = UINT32_MAX;
    if (false == reader.ReadPOD(&arraySize))
        return false;

    Assert(UINT32_MAX > arraySize);
    results.clear();
    results.reserve(arraySize);

    forrange(i, 0, arraySize) {
        u32 count = UINT32_MAX;
        if (false == reader.ReadPOD(&count))
            return false;

        Assert(UINT32_MAX > count);
        MemoryView<const u8> eaten;
        if (false == reader.EatIFP(&eaten, sizeof(_Elt) * count))
            return false;

        results.emplace_back(lambda(eaten.Cast<const _Elt>()) );
    }

    Assert(results.size() == arraySize);
    return true;
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
class BinaryDeserialize_ {
public:
    BinaryDeserialize_(BinarySerializer* owner) : _owner(owner) {}

    BinaryDeserialize_(const BinaryDeserialize_& ) = delete;
    BinaryDeserialize_& operator =(const BinaryDeserialize_& ) = delete;

    void Read(MemoryViewReader& reader);
    void Finalize(RTTI::MetaTransaction* transaction, bool allowOverride);

private:
    static const RTTI::MetaClass* RetrieveMetaClass_(const StringSlice& str);
    static const RTTI::MetaProperty* RetrieveMetaProperty_(const RTTI::MetaClass* metaClass, const StringSlice& str);

    RTTI::MetaObject* CreateObjectFromHeader_(const SerializedObject_& header);
    void DeserializeObjectData_(MemoryViewReader& reader, RTTI::MetaObject* object);

    class AtomReader_ : public RTTI::MetaAtomWrapMoveVisitor {
    public:
        typedef RTTI::MetaAtomWrapMoveVisitor parent_type;
        AtomReader_(BinaryDeserialize_* owner, MemoryViewReader* reader)
            : _owner(owner), _reader(reader) {}

        using parent_type::Inspect;
        using parent_type::Visit;

        virtual void Inspect(RTTI::IMetaAtomPair* ppair, RTTI::Pair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair) override {
            BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize RTTI::Pair<{0}, {1}>",
                ppair->FirstTypeInfo().Name, ppair->SecondTypeInfo().Name );

            if (false == _reader->ExpectPOD(TAG_ATOM_PAIR_) ||
                false == _reader->ExpectPOD(ppair->Atom()->TypeInfo().Id) )
                throw BinarySerializerException("failed to deserialize a PAIR vector");

            pair.first = ppair->FirstTraits()->CreateDefaultValue();
            pair.second = ppair->SecondTraits()->CreateDefaultValue();

            parent_type::Inspect(ppair, pair);
        }

        virtual void Inspect(RTTI::IMetaAtomVector* pvector, RTTI::Vector<RTTI::PMetaAtom>& vector) override {
            BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize RTTI::Vector<{0}>",
                pvector->ValueTypeInfo().Name );

            u32 count;
            if (false == _reader->ExpectPOD(TAG_ATOM_VECTOR_) ||
                false == _reader->ExpectPOD(pvector->Atom()->TypeInfo().Id) ||
                false == _reader->ReadPOD(&count) )
                throw BinarySerializerException("failed to deserialize a RTTI vector");

            const RTTI::IMetaTypeVirtualTraits* traits = pvector->ValueTraits();

            vector.reserve(count);
            forrange(i, 0, count)
                vector.emplace_back(traits->CreateDefaultValue());

            parent_type::Inspect(pvector, vector);
        }

        virtual void Inspect(RTTI::IMetaAtomDictionary* pdictionary, RTTI::Dictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
            BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize RTTI::Dictionary<{0}, {1}>",
                pdictionary->KeyTypeInfo().Name, pdictionary->ValueTypeInfo().Name );

            u32 count;
            if (false == _reader->ExpectPOD(TAG_ATOM_DICTIONARY_) ||
                false == _reader->ExpectPOD(pdictionary->Atom()->TypeInfo().Id) ||
                false == _reader->ReadPOD(&count) )
                throw BinarySerializerException("failed to deserialize a RTTI dictionary");

            const RTTI::IMetaTypeVirtualTraits* keyTraits = pdictionary->KeyTraits();
            const RTTI::IMetaTypeVirtualTraits* valueTraits = pdictionary->ValueTraits();

            dictionary.reserve(count);
            forrange(i, 0, count)
                dictionary.Vector().emplace_back(
                    keyTraits->CreateDefaultValue(),
                    valueTraits->CreateDefaultValue() );

            parent_type::Inspect(pdictionary, dictionary);
        }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
        virtual void Visit(RTTI::MetaTypedAtom<T>* scalar) override { \
            Assert(scalar); \
            Assert(_TypeId == scalar->TypeInfo().Id); \
            if (false == Visit_(scalar)) \
                throw BinarySerializerException("failed to deserialize a RTTI " #T ); \
            /*parent_type::Visit(scalar);*/ \
        }
        FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

    private:
        template <typename T>
        bool Visit_(RTTI::MetaTypedAtom<T>* scalar) {
            const std::streamoff dataoff = _reader->TellI();

            if (false == _reader->ExpectPOD(TAG_ATOM_SCALAR_))
                return false;
            if (false == _reader->ExpectPOD(scalar->TypeInfo().Id))
                return false;
            if (false == ReadValue_(scalar->Wrapper()))
                return false;

            BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize scalar {0} = [{1}]",
                scalar->TypeInfo().Name, scalar->Wrapper() );
            BINARYSERIALIZER_DATALOG(L"@{2:#8X} : <{0}> = [{1}]",
                scalar->TypeInfo().Name, scalar->Wrapper(), dataoff );

            return true;
        }


        template <typename T>
        bool ReadValue_(T& value) {
            STATIC_ASSERT(std::is_pod<T>::value);
            return _reader->ReadPOD(&value);
        }

        template <typename T, size_t _Dim>
        bool ReadValue_(ScalarVector<T, _Dim>& value) {
            STATIC_ASSERT(std::is_pod<T>::value);
            for (size_t i = 0; i < _Dim; ++i)
                if (false == _reader->ReadPOD(&value[i]))
                    return false;
            return true;
        }

        template <typename T, size_t _Width, size_t _Height>
        bool ReadValue_(ScalarMatrix<T, _Width, _Height>& value) {
            STATIC_ASSERT(std::is_pod<T>::value);
            ScalarMatrixData<T, _Width, _Height>& data = value.data();
            const size_t dim = _Width * _Height;
            for (size_t i = 0; i < dim; ++i)
                if (false == _reader->ReadPOD(&data.raw[i]))
                    return false;
            return true;
        }

        bool ReadValue_(String& str) {
            index_t string_i;
            if (false == _reader->ReadPOD(&string_i))
                return false;

            if (string_i >= _owner->_strings.size())
                throw BinarySerializerException("invalid RTTI string index");

            const StringSlice& data = _owner->_strings[string_i];
            str.assign(data.begin(), data.end());
            return true;
        }

        bool ReadValue_(WString& wstr) {
            index_t wstring_i;
            if (false == _reader->ReadPOD(&wstring_i))
                return false;

            if (wstring_i >= _owner->_wstrings.size())
                throw BinarySerializerException("invalid RTTI wstring index");

            const WStringSlice& data = _owner->_wstrings[wstring_i];
            wstr.assign(data.begin(), data.end());
            return true;
        }

        bool ReadValue_(RTTI::PMetaAtom& atom) {
            if (_reader->ExpectPOD(TAG_ATOM_ATOM_)) {
                RTTI::MetaTypeId typeId;
                if (false == _reader->ReadPOD(&typeId))
                    return false;
                switch (typeId) {
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
                case _TypeId: \
                    atom = RTTI::MakeAtom( std::move(RTTI::MetaType< T >::DefaultValue()) ); \
                    Assert(atom->IsDefaultValue()); \
                    atom->Accept(this); \
                    break;
                FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR
                default:
                    throw BinarySerializerException("no support for abstract RTTI atoms deserialization");
                }
                return true;
            }
            else {
                return _reader->ExpectPOD(TAG_ATOM_NULL_);
            }
        }

        bool ReadValue_(RTTI::PMetaObject& object) {
            if (_reader->ExpectPOD(TAG_OBJECT_NULL_)) {
                Assert(nullptr == object);
                return true;
            }
            else {
                index_t object_i;
                if (false == _reader->ReadPOD(&object_i))
                    return false;

                if (object_i >= _owner->_objects.size())
                    throw BinarySerializerException("invalid RTTI object index");

                object = _owner->_objects[object_i];
                return true;
            }
        }

        BinaryDeserialize_* _owner;
        MemoryViewReader* _reader;
        std::streamoff _dataoff;
    };

    BinarySerializer* _owner;

    typedef StronglyTyped::Numeric<u32, BinaryDeserialize_, UINT32_MAX> index_t;
    typedef index_t name_index_t;
    typedef index_t object_index_t;

    typedef VECTOR_THREAD_LOCAL(Serialize, const RTTI::MetaProperty*) properties_type;

    VECTOR_THREAD_LOCAL(Serialize, RTTI::MetaObjectName) _names;
    VECTOR_THREAD_LOCAL(Serialize, StringSlice) _strings;
    VECTOR_THREAD_LOCAL(Serialize, WStringSlice) _wstrings;
    VECTOR_THREAD_LOCAL(Serialize, const RTTI::MetaClass*) _metaClasses;
    VECTOR_THREAD_LOCAL(Serialize, properties_type) _properties;
    VECTOR_THREAD_LOCAL(Serialize, object_index_t) _topObjects;
    VECTOR_THREAD_LOCAL(Serialize, Pair<name_index_t COMMA object_index_t>) _objectsToExport;
    VECTOR_THREAD_LOCAL(Serialize, SerializedObject_) _headers;
    VECTOR_THREAD_LOCAL(Serialize, RTTI::PMetaObject) _objects;
};
//----------------------------------------------------------------------------
void BinaryDeserialize_::Read(MemoryViewReader& reader) {

    if (false == reader.ExpectPOD(FILE_MAGIC_) )
        throw BinarySerializerException("invalid file magic");
    if (false == reader.ExpectPOD(FILE_VERSION_) )
        throw BinarySerializerException("unsupported file version");

    if (false == reader.ExpectPOD(SECTION_NAMES_) ||
        false == DeserializePODArrays_<char>(reader, _names, [](const StringSlice& str) { Assert(str.size()); return RTTI::MetaObjectName(str); }) )
        throw BinarySerializerException("invalid names section");

    if (false == reader.ExpectPOD(SECTION_STRINGS_) ||
        false == DeserializePODArrays_<char>(reader, _strings, [](const StringSlice& str) { return str; }) )
        throw BinarySerializerException("invalid strings section");

    if (false == reader.ExpectPOD(SECTION_WSTRINGS_) ||
        false == DeserializePODArrays_<wchar_t>(reader, _wstrings, [](const WStringSlice& wstr) { return wstr; }) )
        throw BinarySerializerException("invalid wstrings section");

    if (false == reader.ExpectPOD(SECTION_CLASSES_) ||
        false == DeserializePODArrays_<char>(reader, _metaClasses, RetrieveMetaClass_) )
        throw BinarySerializerException("expected classes section");

    if (false == reader.ExpectPOD(SECTION_PROPERTIES_) )
        throw BinarySerializerException("expected properties section");

    _properties.resize(_metaClasses.size());
    forrange(i, 0, _properties.size()) {
        const RTTI::MetaClass* metaClass = _metaClasses[i];
        properties_type& properties = _properties[i];

        const bool succeed = DeserializePODArrays_<char>(reader, properties, [metaClass](const StringSlice& str) {
            return RetrieveMetaProperty_(metaClass, str);
        });

        if (false == succeed)
            throw BinarySerializerException("invalid RTTI property");
    }

    if (false == reader.ExpectPOD(SECTION_TOP_OBJECTS_) ||
        false == DeserializePODs_(reader, _topObjects) )
        throw BinarySerializerException("expected top objects section");

    if (false == reader.ExpectPOD(SECTION_OBJECTS_TO_EXPORT_) ||
        false == DeserializePODs_(reader, _objectsToExport) )
        throw BinarySerializerException("expected objects to export section");

    if (false == reader.ExpectPOD(SECTION_OBJECTS_HEADER_) ||
        false == DeserializePODs_(reader, _headers))
        throw BinarySerializerException("expected objects header section");

    // first pass : create or import every object in the file
    _objects.reserve(_headers.size());
    for (const SerializedObject_& header : _headers)
        _objects.emplace_back(CreateObjectFromHeader_(header));

    u64 dataSizeInBytes = 0;
    if (false == reader.ExpectPOD(SECTION_OBJECTS_DATA_) ||
        false == reader.ReadPOD(&dataSizeInBytes) )
        throw BinarySerializerException("expected objects data section");

    const std::streamoff dataBegin = reader.TellI();
    const std::streamoff dataEnd = checked_cast<std::streamoff>(dataBegin + dataSizeInBytes);

    // second pass : read every object properties
    MemoryViewReader dataReader = reader.SubRange(
        checked_cast<size_t>(dataBegin),
        checked_cast<size_t>(dataSizeInBytes) );

    forrange(i, 0, _headers.size()) {
        const SerializedObject_& header = _headers[i];
        if (header.Type == TAG_OBJECT_NULL_ ||
            header.Type == TAG_OBJECT_IMPORT_ )
            continue;

        const RTTI::PMetaObject& object = _objects[i];
        Assert(object);
        Assert(header.DataOffset == dataReader.TellI());

        DeserializeObjectData_(dataReader, object.get());
    }

    if (false == dataReader.Eof())
        throw BinarySerializerException("object section has unread data");

    reader.SeekI(dataEnd);
    if (false == reader.ExpectPOD(SECTION_END_) )
        throw BinarySerializerException("expected end section");
}
//----------------------------------------------------------------------------
void BinaryDeserialize_::Finalize(RTTI::MetaTransaction* transaction, bool allowOverride) {
    Assert(transaction);

    for (const Pair<name_index_t, object_index_t>& it : _objectsToExport) {
        if (it.first > _names.size() ||
            it.second > _objects.size() )
            throw BinarySerializerException("invalid RTTI object export");

        const RTTI::MetaObjectName& name = _names[it.first];
        const RTTI::PMetaObject& object = _objects[it.second];

        object->RTTI_Export(name);
    }

    Assert(_topObjects.size() || 0 == _objects.size());

    transaction->reserve(transaction->size() + _topObjects.size());
    for (const object_index_t& object_i : _topObjects)
        transaction->Add(_objects[object_i].get());
}
//----------------------------------------------------------------------------
const RTTI::MetaClass* BinaryDeserialize_::RetrieveMetaClass_(const StringSlice& str) {
    const RTTI::MetaClassName name(str);
    Assert(!name.empty());

    const RTTI::MetaClass* metaClass = RTTI::MetaClassDatabase::Instance().GetIFP(name);
    if (nullptr == metaClass)
        throw BinarySerializerException("unknown RTTI metaclass");

    return metaClass;
}
//----------------------------------------------------------------------------
const RTTI::MetaProperty* BinaryDeserialize_::RetrieveMetaProperty_(const RTTI::MetaClass* metaClass, const StringSlice& str) {
    Assert(metaClass);

    const RTTI::MetaPropertyName name(str);
    Assert(!name.empty());

    const RTTI::MetaProperty* metaProperty = metaClass->PropertyIFP(name);
    if (nullptr == metaProperty)
        throw BinarySerializerException("unknown RTTI property");

    return metaProperty;
}
//----------------------------------------------------------------------------
RTTI::MetaObject* BinaryDeserialize_::CreateObjectFromHeader_(const SerializedObject_& header) {
    if (TAG_OBJECT_NULL_ == header.Type) {
        Assert(UINT32_MAX == header.MetaClassIndex);
        Assert(UINT32_MAX == header.NameIndex);
        Assert(UINT32_MAX == header.DataOffset);

        BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize null object");

        return nullptr;
    }
    else if (TAG_OBJECT_PRIVATE_ == header.Type) {
        Assert(UINT32_MAX != header.MetaClassIndex);
        Assert(UINT32_MAX == header.NameIndex);
        Assert(UINT32_MAX != header.DataOffset);

        const RTTI::MetaClass* metaClass = _metaClasses[header.MetaClassIndex];

        BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize private object <{0}>", metaClass->Name());

        return metaClass->CreateInstance();
    }
    else {
        Assert(UINT32_MAX != header.MetaClassIndex);
        Assert(UINT32_MAX != header.NameIndex);
        Assert(UINT32_MAX != header.DataOffset);

        const RTTI::MetaObjectName name = _names[header.NameIndex];
        const RTTI::MetaClass* metaClass = _metaClasses[header.MetaClassIndex];

        if (TAG_OBJECT_EXPORT_ == header.Type) {

            BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize exported object <{0}> '{1}'", metaClass->Name(), name);

            return metaClass->CreateInstance();
        }
        else {
            Assert(TAG_OBJECT_IMPORT_ == header.Type);

            BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize imported object <{0}> '{1}'", metaClass->Name(), name);

            const RTTI::MetaAtom* patom = RTTI::MetaAtomDatabase::Instance().GetIFP(name);
            if (nullptr == patom ||
                nullptr == patom->As<RTTI::PMetaObject>() )
                throw BinarySerializerException("failed to import RTTI object from database");

            return patom->Cast<RTTI::PMetaObject>()->Wrapper().get();
        }
    }
}
//----------------------------------------------------------------------------
void BinaryDeserialize_::DeserializeObjectData_(MemoryViewReader& reader, RTTI::MetaObject* object) {
    u32 metaClassCount = 0;
    if (false == reader.ExpectPOD(TAG_OBJECT_START_) ||
        false == reader.ReadPOD(&metaClassCount) )
        throw BinarySerializerException("failed to read RTTI object header");

    BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize object data for @{0}", reader.TellI());

    AtomReader_ atomReader(this, &reader);

    forrange(i, 0, metaClassCount) {
        index_t class_i;
        u32 propertyCount = 0;
        if (false == reader.ExpectPOD(TAG_OBJECT_METACLASS_) ||
            false == reader.ReadPOD(&class_i) ||
            false == reader.ReadPOD(&propertyCount) )
            throw BinarySerializerException("failed to read RTTI obect metaclass");

        Assert(0 < propertyCount);

        if (class_i >= _metaClasses.size())
            throw BinarySerializerException("invalid RTTI metaclass index");
        Assert(_metaClasses.size() == _properties.size());

        const RTTI::MetaClass* metaClass = _metaClasses[class_i];
        const properties_type& properties = _properties[class_i];

        BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize metaclass data <{0}>", metaClass->Name());

        forrange(j, 0, propertyCount) {
            index_t prop_i;
            if (false == reader.ReadPOD(&prop_i) ||
                prop_i > properties.size() )
                throw BinarySerializerException("invalid RTTI property index");

            const RTTI::MetaProperty* metaProperty = properties[prop_i];

            BINARYSERIALIZER_LOG(Info, L"[Serialize] Deserialize property data <{0}> '{1}'",
                metaProperty->TypeInfo().Name, metaProperty->Name());

            const RTTI::PMetaAtom atom = metaProperty->Traits()->CreateDefaultValue();
            Assert(atom);
            Assert(atom->IsDefaultValue());
            Assert(atom->TypeInfo().Id == metaProperty->TypeInfo().Id);

            atomReader.Append(atom.get());
            Assert(false == atom->IsDefaultValue());

            metaProperty->UnwrapMove(object, atom.get());
        }
    }

    if (false == reader.ExpectPOD(TAG_OBJECT_END_))
        throw BinarySerializerException("failed to read RTTI object foorter");
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
class BinarySerialize_ {
public:
    typedef ObjectIndexer_<const RTTI::MetaProperty* > property_indices_type;
    typedef ObjectIndexer_<const RTTI::MetaClass* > class_indices_type;
    typedef ObjectIndexer_<const RTTI::MetaObject* > object_indices_type;
    typedef ObjectIndexer_<String > string_indices_type;
    typedef ObjectIndexer_<WString > wstring_indices_type;
    typedef ObjectIndexer_<RTTI::MetaObjectName > name_indices_type;

    typedef property_indices_type::index_t property_index_t;
    typedef class_indices_type::index_t class_index_t;
    typedef object_indices_type::index_t object_index_t;
    typedef string_indices_type::index_t string_index_t;
    typedef wstring_indices_type::index_t wstring_index_t;
    typedef name_indices_type::index_t name_index_t;

    BinarySerialize_(const BinarySerializer* owner, const RTTI::MetaTransaction* transaction)
        : _owner(owner), _transaction(transaction) {
        Assert(owner);
        Assert(transaction);
    }

    BinarySerialize_(const BinarySerialize_& ) = delete;
    BinarySerialize_& operator =(const BinarySerialize_& ) = delete;

    void Append(const RTTI::MetaObject* object, bool topObject = true);
    void Append(const MemoryView<const RTTI::PMetaObject>& objects, bool topObject = true);
    void Finalize(IStreamWriter* writer);

private:
    class AtomWriter_ : public RTTI::MetaAtomWrapCopyVisitor {
    public:
        typedef RTTI::MetaAtomWrapCopyVisitor parent_type;
        AtomWriter_(BinarySerialize_* owner) : _owner(owner) {}

        using parent_type::Append;
        using parent_type::Inspect;
        using parent_type::Visit;

        template <typename T>
        void WritePOD(const T& pod) { _owner->_objectStream.WritePOD(pod); }

        virtual void Inspect(const RTTI::IMetaAtomPair* ppair, const RTTI::Pair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair) override {
            BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize RTTI::Pair<{0}, {1}>",
                ppair->FirstTypeInfo().Name, ppair->SecondTypeInfo().Name );

            WritePOD(TAG_ATOM_PAIR_);
            WritePOD(ppair->Atom()->TypeInfo().Id);
            parent_type::Inspect(ppair, pair);
        }

        virtual void Inspect(const RTTI::IMetaAtomVector* pvector, const RTTI::Vector<RTTI::PMetaAtom>& vector) override {
            BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize RTTI::Vector<{0}>",
                pvector->ValueTypeInfo().Name );

            WritePOD(TAG_ATOM_VECTOR_);
            WritePOD(pvector->Atom()->TypeInfo().Id);
            WritePOD(checked_cast<u32>(vector.size()));
            parent_type::Inspect(pvector, vector);
        }

        virtual void Inspect(const RTTI::IMetaAtomDictionary* pdictionary, const RTTI::Dictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
            BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize RTTI::Dictionary<{0}, {1}>",
                pdictionary->KeyTypeInfo().Name, pdictionary->ValueTypeInfo().Name );

            WritePOD(TAG_ATOM_DICTIONARY_);
            WritePOD(pdictionary->Atom()->TypeInfo().Id);
            WritePOD(checked_cast<u32>(dictionary.size()));
            parent_type::Inspect(pdictionary, dictionary);
        }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
        virtual void Visit(const RTTI::MetaTypedAtom<T>* scalar) override { \
            Assert(scalar); \
            Assert(_TypeId == scalar->TypeInfo().Id); \
            Visit_(scalar); \
            /*parent_type::Visit(scalar);*/ \
        }
        FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

    private:
        template <typename T>
        void Visit_(const RTTI::MetaTypedAtom<T>* scalar) {
            BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize scalar {0} = [{1}]",
                scalar->TypeInfo().Name, scalar->Wrapper() );
            BINARYSERIALIZER_DATALOG(L"@{2:#8X} : <{0}> = [{1}]",
                scalar->TypeInfo().Name, scalar->Wrapper(), _owner->_objectStream.TellO() );

            WritePOD(TAG_ATOM_SCALAR_);
            WritePOD(scalar->TypeInfo().Id);
            WriteValue_(scalar->Wrapper());
        }

        template <typename T>
        void WriteValue_(const T& value) {
            STATIC_ASSERT(std::is_pod<T>::value);
            WritePOD(value);
        }

        template <typename T, size_t _Dim>
        void WriteValue_(const ScalarVector<T, _Dim>& value) {
            STATIC_ASSERT(std::is_pod<T>::value);
            for (size_t i = 0; i < _Dim; ++i)
                WritePOD(value[i]);
        }

        template <typename T, size_t _Width, size_t _Height>
        void WriteValue_(const ScalarMatrix<T, _Width, _Height>& value) {
            STATIC_ASSERT(std::is_pod<T>::value);
            const MemoryView<const T> data = value.MakeView();
            for (const T& pod : data)
                WritePOD(pod);
        }

        void WriteValue_(const String& str) {
            const string_index_t string_i = _owner->_stringIndices.IndexOf(str);
            WritePOD(string_i);
        }

        void WriteValue_(const WString& wstr) {
            const wstring_index_t wstring_i = _owner->_wstringIndices.IndexOf(wstr);
            WritePOD(wstring_i);
        }

        void WriteValue_(const RTTI::PMetaAtom& atom) {
            if (atom) {
                const RTTI::MetaTypeId typeId = atom->TypeInfo().Id;
                switch(typeId)
                {
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) case _TypeId:
                FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR
                    WritePOD(TAG_ATOM_ATOM_);
                    WritePOD(typeId);
                    atom->Accept(this);
                    break;

                default:
                    throw BinarySerializerException("no support for abstract RTTI atoms serialization");
                }
            }
            else
                WritePOD(TAG_ATOM_NULL_);
        }

        void WriteValue_(const RTTI::PMetaObject& object) {
            if (object) {
                const object_index_t object_i = _owner->AddObject_(object.get());
                WritePOD(object_i);
            }
            else
                WritePOD(TAG_OBJECT_NULL_);
        }

        BinarySerialize_* _owner;
    };

    class_index_t AddClass_(const RTTI::MetaClass* metaClass);
    object_index_t AddObject_(const RTTI::MetaObject* object);
    void ProcessQueue_();
    void WriteObject_(AtomWriter_& atomWriter, const RTTI::MetaObject* object, const RTTI::MetaClass* metaClass);

    const BinarySerializer* _owner;
    const RTTI::MetaTransaction* _transaction;

    MEMORYSTREAM_THREAD_LOCAL(Serialize) _objectStream;
    DEQUE_THREAD_LOCAL(Serialize, RTTI::SCMetaObject) _objectQueue;

    STATIC_ASSERT(sizeof(FourCC) == sizeof(property_index_t));
    STATIC_ASSERT(sizeof(FourCC) == sizeof(class_index_t));
    STATIC_ASSERT(sizeof(FourCC) == sizeof(object_index_t));
    STATIC_ASSERT(sizeof(FourCC) == sizeof(string_index_t));
    STATIC_ASSERT(sizeof(FourCC) == sizeof(wstring_index_t));
    STATIC_ASSERT(sizeof(FourCC) == sizeof(name_index_t));

    string_indices_type _stringIndices;
    wstring_indices_type _wstringIndices;

    class_indices_type _classIndices;
    VECTOR_THREAD_LOCAL(Serialize, property_indices_type) _propertiesByClassIndex;

    object_indices_type _objectIndices;
    VECTOR_THREAD_LOCAL(Serialize, object_index_t) _topObjects;
    VECTOR_THREAD_LOCAL(Serialize, SerializedObject_) _objectsStreamed;

    name_indices_type _nameIndices;
    HASHMAP_THREAD_LOCAL(Serialize, name_index_t, object_index_t) _namesToExport;
    HASHMAP_THREAD_LOCAL(Serialize, name_index_t, object_index_t) _namesToImport;
};
//----------------------------------------------------------------------------
void BinarySerialize_::Append(const RTTI::MetaObject* object, bool topObject /* = true */) {
    Assert(false == topObject || nullptr != object);
    const object_index_t object_i = AddObject_(object);

    if (topObject) {
        AssertRelease(object);
        _topObjects.push_back(object_i);
    }

    ProcessQueue_();
}
//----------------------------------------------------------------------------
void BinarySerialize_::Append(const MemoryView<const RTTI::PMetaObject>& objects, bool topObject /* = true */) {
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
void BinarySerialize_::Finalize(IStreamWriter* writer) {
    Assert(writer);

    // TODO: merge objects when metaclass allows it

    writer->WritePOD(FILE_MAGIC_);
    writer->WritePOD(FILE_VERSION_);

    writer->WritePOD(SECTION_NAMES_);
    _nameIndices.Serialize(writer, [writer](const RTTI::MetaObjectName& name) {
        SerializePODs_(writer, name.MakeView());
    });

    writer->WritePOD(SECTION_STRINGS_);
    _stringIndices.Serialize(writer, [writer](const StringSlice& str) {
        SerializePODs_(writer, str);
    });

    writer->WritePOD(SECTION_WSTRINGS_);
    _wstringIndices.Serialize(writer, [writer](const WStringSlice& wstr) {
        SerializePODs_(writer, wstr);
    });

    writer->WritePOD(SECTION_CLASSES_);
    _classIndices.Serialize(writer, [writer](const RTTI::MetaClass* metaClass) {
        SerializePODs_(writer, metaClass->Name().MakeView());
    });

    writer->WritePOD(SECTION_PROPERTIES_);
    AssertRelease(_propertiesByClassIndex.size() == _classIndices.size());
    for (const property_indices_type& properties : _propertiesByClassIndex) {
        properties.Serialize(writer, [writer](const RTTI::MetaProperty* property) {
            SerializePODs_(writer, property->Name().MakeView());
        });
    }

    writer->WritePOD(SECTION_TOP_OBJECTS_);
    SerializePODs_(writer, MakeConstView(_topObjects));

    writer->WritePOD(SECTION_OBJECTS_TO_EXPORT_);
    SerializePODs_(writer, _namesToExport);

    writer->WritePOD(SECTION_OBJECTS_HEADER_);
    AssertRelease(_objectsStreamed.size() == _objectIndices.size());
    SerializePODs_(writer, MakeConstView(_objectsStreamed));

    writer->WritePOD(SECTION_OBJECTS_DATA_);
    writer->WritePOD(checked_cast<u64>(_objectStream.SizeInBytes()));

    const std::streamoff dataBegin = writer->TellO();
    const std::streamoff dataEnd = dataBegin + checked_cast<std::streamoff>(_objectStream.SizeInBytes());
    writer->Write(_objectStream.Pointer(), _objectStream.SizeInBytes());

    const std::streamoff fileEnd = writer->TellO();
    Assert(fileEnd == dataEnd);
    writer->WritePOD(SECTION_END_);
}
//----------------------------------------------------------------------------
auto BinarySerialize_::AddClass_(const RTTI::MetaClass* metaClass) -> class_index_t {
    Assert(metaClass);
    class_index_t class_i;
    if (false == _classIndices.Insert_ReturnIfExists(&class_i, metaClass)) {
        Assert(_propertiesByClassIndex.size() + 1 == _classIndices.size());
        _propertiesByClassIndex.emplace_back();
    }
    Assert(class_index_t::DefaultValue != class_i);
    return class_i;
}
//----------------------------------------------------------------------------
auto BinarySerialize_::AddObject_(const RTTI::MetaObject* object) -> object_index_t {
    object_index_t object_i;
    const bool alreadyStreamed = _objectIndices.Insert_ReturnIfExists(&object_i, object);
    if (false == alreadyStreamed)
        _objectQueue.push_back(object);

    Assert(object_index_t::DefaultValue != object_i);
    return object_i;
}
//----------------------------------------------------------------------------
void BinarySerialize_::ProcessQueue_() {
    AtomWriter_ atomWriter(this);

    _objectsStreamed.reserve(_objectIndices.size());

    while (!_objectQueue.empty()) {
        const RTTI::SCMetaObject object = _objectQueue.front();
        _objectQueue.pop_front();

        const object_index_t object_i = _objectIndices.IndexOf(object);

        _objectsStreamed.emplace_back();
        Assert(_objectsStreamed.size() == object_i + 1);

        SerializedObject_& header = _objectsStreamed.back();
        header.Type = FourCC();
        header.MetaClassIndex = UINT32_MAX;
        header.NameIndex = UINT32_MAX;
        header.DataOffset = UINT32_MAX;

        if (nullptr == object) {
            BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize null object");
            header.Type = TAG_OBJECT_NULL_;
        }
        else {
            const RTTI::MetaClass* metaClass = object->RTTI_MetaClass();
            Assert(metaClass);

            const class_index_t class_i = AddClass_(metaClass);
            header.MetaClassIndex = class_i;

            if (object->RTTI_IsExported()) {
                const RTTI::MetaObjectName metaName = object->RTTI_Name();
                Assert(!metaName.empty());

                const name_index_t name_i = _nameIndices.IndexOf(metaName);

                const bool exported = _transaction->Contains(object);
                if (exported) {
                    BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize exported object <{0}> '{1}'", metaClass->Name(), metaName);
                    header.Type = TAG_OBJECT_EXPORT_;
                    header.NameIndex = name_i;
                    Insert_AssertUnique(_namesToExport, name_i, object_i);
                    Assert(_namesToImport.end() == _namesToImport.find(name_i));
                }
                else {
                    BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize imported object <{0}> '{1}'", metaClass->Name(), metaName);
                    header.Type = TAG_OBJECT_IMPORT_;
                    header.NameIndex = name_i;
                    Insert_AssertUnique(_namesToImport, name_i, object_i);
                    Assert(_namesToExport.end() == _namesToExport.find(name_i));
                    continue; // skip serialization because it does not belong to the current transaction
                }
            }
            else {
                BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize private object <{0}>", object->RTTI_MetaClass()->Name());
                header.Type = TAG_OBJECT_PRIVATE_;
            }

            header.DataOffset = checked_cast<u32>(_objectStream.TellO());

            WriteObject_(atomWriter, object, metaClass);
        }

        Assert(FourCC() != header.Type);
    }
}
//----------------------------------------------------------------------------
void BinarySerialize_::WriteObject_(AtomWriter_& atomWriter, const RTTI::MetaObject* object, const RTTI::MetaClass* metaClass) {
    Assert(object);
    Assert(object->RTTI_MetaClass() == metaClass);

    BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize object data for @{0}", _objectStream.TellI());

    _objectStream.WritePOD(TAG_OBJECT_START_);

    u32 metaClassCount = 0;
    const std::streamoff metaClassCountOffset = _objectStream.TellO();
    _objectStream.WritePOD(metaClassCount); // will be overwritten at the end of function

    while (metaClass) {
        BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize metaclass data <{0}>", metaClass->Name());

        const std::streamoff metaClassTagOffset = _objectStream.TellO();
        _objectStream.WritePOD(TAG_OBJECT_METACLASS_);

        const class_index_t class_i = AddClass_(metaClass);
        _objectStream.WritePOD(class_i);

        u32 propertyCount = 0;
        const std::streamoff propertyCountOffset = _objectStream.TellO();
        _objectStream.WritePOD(propertyCount); // will be overwritten at the end of the loop

        property_indices_type& propertyIndices = _propertiesByClassIndex[class_i];

        for (const RTTI::UCMetaProperty& prop : metaClass->Properties()) {
            if (prop->IsDefaultValue(object))
                continue;

            BINARYSERIALIZER_LOG(Info, L"[Serialize] Serialize property data <{0}> '{1}'",
                it.second->TypeInfo().Name, it.second->Name());

            ++propertyCount;

            const property_index_t prop_i = propertyIndices.IndexOf(prop.get());
            _objectStream.WritePOD(prop_i);

            const RTTI::PMetaAtom atom = prop->WrapCopy(object);
            atomWriter.Append(atom.get());
        }

        if (0 < propertyCount) {
            ++metaClassCount;

            const std::streamoff propertyEndOffset = _objectStream.TellO();
            _objectStream.SeekO(propertyCountOffset, SeekOrigin::Begin);
            _objectStream.WritePOD(propertyCount); // overwrite property count
            _objectStream.SeekO(propertyEndOffset, SeekOrigin::Begin);
        }
        else {
            _objectStream.resize(checked_cast<size_t>(metaClassTagOffset)); // erase empty metaclass
            Assert(_objectStream.TellO() == metaClassTagOffset);
        }

        metaClass = metaClass->Parent();
    }

    const std::streamoff objectEndOffset = _objectStream.TellO();
    _objectStream.SeekO(metaClassCountOffset, SeekOrigin::Begin);
    _objectStream.WritePOD(metaClassCount); // overwrite metaclass count
    _objectStream.SeekO(objectEndOffset, SeekOrigin::Begin);

    _objectStream.WritePOD(TAG_OBJECT_END_);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BinarySerializer::BinarySerializer() {}
//----------------------------------------------------------------------------
BinarySerializer::~BinarySerializer() {}
//----------------------------------------------------------------------------
void BinarySerializer::Deserialize(RTTI::MetaTransaction* transaction, const MemoryView<const u8>& input, const wchar_t *sourceName/* = nullptr */) {
    if (input.empty())
        return;

    MemoryViewReader reader(input);
    BinaryDeserialize_ deserialize(this);
    deserialize.Read(reader);
    deserialize.Finalize(transaction, true);
}
//----------------------------------------------------------------------------
void BinarySerializer::Serialize(IStreamWriter* output, const RTTI::MetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);

    BinarySerialize_ serialize(this, transaction);
    serialize.Append(transaction->MakeView());
    serialize.Finalize(output);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
