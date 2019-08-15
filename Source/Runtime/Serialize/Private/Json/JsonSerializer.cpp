#include "stdafx.h"

#include "Json/JsonSerializer.h"

#include "Json/Json.h"

#include "RTTI/Any.h"
#include "RTTI/Atom.h"
#include "RTTI/AtomHelpers.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/Typedefs.h"
#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "Container/HashMap.h"
#include "Container/HashSet.h"
#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/StreamProvider.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"

#include "TransactionLinker.h"
#include "TransactionSaver.h"

namespace PPE {
namespace Serialize {
EXTERN_LOG_CATEGORY(PPE_SERIALIZE_API, Serialize)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FJsonSerializer::FJsonSerializer(bool minify/* = true */) {
    SetMinify(minify);
}
//----------------------------------------------------------------------------
FJsonSerializer::~FJsonSerializer() = default;
//----------------------------------------------------------------------------
void FJsonSerializer::Deserialize(IStreamReader& input, FTransactionLinker* linker) const {
    Assert(linker);

    FJson json;

    UsingBufferedStream(&input, [&json, linker](IBufferedStreamReader* buffered) {
        if (not FJson::Load(&json, linker->Filename(), buffered))
            PPE_THROW_IT(FJsonSerializerException("failed to parse Json document"));
    });

    if (not Json_to_RTTI(json, linker))
        PPE_THROW_IT(FJsonSerializerException("failed to convert Json to RTTI"));
}
//----------------------------------------------------------------------------
void FJsonSerializer::Serialize(const FTransactionSaver& saver, IStreamWriter* output) const {
    Assert(output);

    FJson json;
    RTTI_to_Json(saver, &json);

    UsingBufferedStream(output, [this, &json](IBufferedStreamWriter* buffered) {
        FTextWriter oss(buffered);
        json.ToStream(oss, Minify());
    });
}
//----------------------------------------------------------------------------
FExtname FJsonSerializer::Extname() {
    return FFSConstNames::Json();
}
//----------------------------------------------------------------------------
PSerializer FJsonSerializer::Get() {
    return PSerializer::Make<FJsonSerializer>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
enum class EJsonRTTI_ : u32 {
    None = 0,
    TopObject = 1<<0,
};
ENUM_FLAGS(EJsonRTTI_);
//----------------------------------------------------------------------------
static FJson::FTextHeap::FText Json_Class()         { return FJson::FTextHeap::MakeStaticText("Class"); };
static FJson::FTextHeap::FText Json_Name()          { return FJson::FTextHeap::MakeStaticText("Name"); };
static FJson::FTextHeap::FText Json_Flags()         { return FJson::FTextHeap::MakeStaticText("Flags"); };
static FJson::FTextHeap::FText Json_Properties()    { return FJson::FTextHeap::MakeStaticText("Properties"); };
//----------------------------------------------------------------------------
template <typename T, class = void>
struct TJson_RTTI_traits;
template <typename T>
void RTTI_to_Json_(FJson& doc, FJson::FValue& dst, const T& src) {
    TJson_RTTI_traits<T>::Json(doc, dst, src);
}
template <typename T>
bool Json_to_RTTI_(T& dst, const FJson::FValue& src) {
    return TJson_RTTI_traits<T>::RTTI(dst, src);
}
//----------------------------------------------------------------------------
template <>
struct TJson_RTTI_traits<bool> {
    static void Json(FJson&, FJson::FValue& dst, bool src) {
        dst.SetValue(src);
    }
    static bool RTTI(bool& dst, const FJson::FValue& src) {
        const FJson::FBool* boolIFP = src.AsBool();
        if (nullptr == boolIFP) return false;
        dst = *boolIFP;
        return true;
    }
};
template <typename T>
struct TJson_RTTI_traits<T, Meta::TEnableIf< std::is_integral<T>::value > > {
    static void Json(FJson& , FJson::FValue& dst, T src) {
        dst.SetValue(checked_cast<FJson::FInteger>(src));
    }
    static bool RTTI(T& dst, const FJson::FValue& src) {
        const FJson::FInteger* intIFP = src.AsInteger();
        if (nullptr == intIFP) return false;
        dst = T(*intIFP);
        return true;
    }
};
template <typename T>
struct TJson_RTTI_traits<T, Meta::TEnableIf< std::is_floating_point<T>::value > > {
    static void Json(FJson& , FJson::FValue& dst, T src) {
        dst.SetValue(checked_cast<FJson::FFloat>(src));
    }
    static bool RTTI(T& dst, const FJson::FValue& src) {
        const FJson::FFloat* floatIFP = src.AsFloat();
        if (nullptr == floatIFP) return false;
        dst = T(*floatIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FString> {
    static void Json(FJson& doc, FJson::FValue& dst, const FString& src) {
        dst.SetValue(doc.MakeString(src));
    }
    static bool RTTI(FString& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.AsString();
        if (nullptr == strIFP) return false;
        dst = *strIFP;
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FWString> {
    static void Json(FJson& doc, FJson::FValue& dst, const FWString& src) {
        dst.SetValue(doc.MakeString(ToString(src)));
    }
    static bool RTTI(FWString& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.AsString();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<RTTI::FName> {
    static void Json(FJson& doc, FJson::FValue& dst, const RTTI::FName& src) {
        dst.SetValue(doc.MakeString(src.MakeView()));
    }
    static bool RTTI(RTTI::FName& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.AsString();
        if (nullptr == strIFP) return false;
        dst = RTTI::FName(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FDirpath> {
    static void Json(FJson& doc, FJson::FValue& dst, const FDirpath& src) {
        dst.SetValue(doc.MakeString(src.ToString()));
    }
    static bool RTTI(FDirpath& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.AsString();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FFilename> {
    static void Json(FJson& doc, FJson::FValue& dst, const FFilename& src) {
        dst.SetValue(doc.MakeString(src.ToString()));
    }
    static bool RTTI(FFilename& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.AsString();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<RTTI::FBinaryData> {
    static void Json(FJson& doc, FJson::FValue& dst, const RTTI::FBinaryData& src) {
        dst.SetValue(doc.MakeString(src.MakeConstView().Cast<const char>(), false/* don't merge */));
    }
    static bool RTTI(RTTI::FBinaryData& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.AsString();
        if (nullptr == strIFP) return false;
        dst.CopyFrom(strIFP->MakeView().Cast<const u8>());
        return true;
    }
};
template <typename T, size_t _Dim>
struct TJson_RTTI_traits<TScalarVector<T, _Dim>> {
    static void Json(FJson& doc, FJson::FValue& dst, const TScalarVector<T, _Dim>& src) {
        FJson::FArray& arr = dst.SetType_AssumeNull(doc, FJson::TypeArray{});
        arr.resize(_Dim);
        forrange(i, 0, _Dim)
            RTTI_to_Json_(doc, arr[i], src._data[i]);
    }
    static bool RTTI(TScalarVector<T, _Dim>& dst, const FJson::FValue& src) {
        const FJson::FArray* arrIFP = src.AsArray();
        if (nullptr == arrIFP || _Dim != arrIFP->size()) return false;
        forrange(i, 0, _Dim)
            if (not Json_to_RTTI_(dst._data[i], arrIFP->at(i)))
                return false;
        return true;
    }
};
template <typename T, size_t _W, size_t _H>
struct TJson_RTTI_traits<TScalarMatrix<T, _W, _H>> {
    static void Json(FJson& doc, FJson::FValue& dst, const TScalarMatrix<T, _W, _H>& src) {
        FJson::FArray& arr = dst.SetType_AssumeNull(doc, FJson::TypeArray{});
        arr.resize(_W);
        forrange(i, 0, _W)
            RTTI_to_Json_(doc, arr[i], src.Column(i));
    }
    static bool RTTI(TScalarMatrix<T, _W, _H>& dst, const FJson::FValue& src) {
        const FJson::FArray* arrIFP = src.AsArray();
        if (nullptr == arrIFP || _W != arrIFP->size()) return false;
        forrange(i, 0, _W)
            if (not Json_to_RTTI_(dst.Column(i), arrIFP->at(i)))
                return false;
        return true;
    }
};
//----------------------------------------------------------------------------
class FRTTI_to_Json_ : public RTTI::IAtomVisitor {
public:
    explicit FRTTI_to_Json_(FJson& doc)
        : _doc(doc)
    {}

    void reserve(size_t n) {
        _visiteds.reserve(n);
        _values.reserve(n);
    }

    void Append(const RTTI::FMetaObject* ref, FJson::FValue& value) {
        Assert(ref);
        Assert_NoAssume(_values.empty());

        FString name;
        if (ref->RTTI_IsExported()) {
            Assert_NoAssume(not ref->RTTI_Name().empty());

            name = StringFormat("~/{0}", ref->RTTI_Name());
        }
        else { // generate temporary name (for internal references)
            Assert_NoAssume(ref->RTTI_Name().empty());

            name = StringFormat("{0}_{1}", ref->RTTI_Class()->Name(), _visiteds.size());
        }

        Insert_AssertUnique(_visiteds, ref, name);

        const RTTI::FMetaClass* metaClass = ref->RTTI_Class();

        FJson::FObject& jsonObject = value.SetType_AssumeNull(_doc, FJson::TypeObject{});
        jsonObject.reserve(1/* class */ + 1/* name */ + 1/* props */);

        jsonObject.Add(Json_Class()).SetValue(
            _doc.MakeString(metaClass->Name().MakeView()));

        jsonObject.Add(Json_Name()).SetValue(
            _doc.MakeString(name));

        if (ref->RTTI_IsTopObject())
            jsonObject.Add(Json_Flags()).SetValue(i64(EJsonRTTI_::TopObject));

        FJson::FObject* jsonProperties = nullptr;

        for (const RTTI::FMetaProperty* prop : metaClass->AllProperties()) {
            RTTI::FAtom atom = prop->Get(*ref);

            if (atom.IsDefaultValue())
                continue;

            if (nullptr == jsonProperties)
                jsonProperties = &jsonObject.Add(Json_Properties()).SetType_AssumeNull(_doc, FJson::TypeObject{});

            _values.push_back(&jsonProperties->Add(
                _doc.MakeString(prop->Name().MakeView()) ));

            atom.Accept(this);

            _values.pop_back();
        }
    }

    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        const size_t n = tuple->Arity();

        FJson::FArray& arr = Head_().SetType_AssumeNull(_doc, FJson::TypeArray{});
        arr.resize(n);

        if (arr.empty())
            return true;

        forrange(i, 0, n) {
            _values.push_back(&arr[i]);
            const bool result = tuple->At(data, i).Accept(this);
            _values.pop_back();

            if (not result)
                return false;
        }

        return true;
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        FJson::FArray& arr = Head_().SetType_AssumeNull(_doc, FJson::TypeArray{});
        arr.resize(list->Count(data));

        if (arr.empty())
            return true;

        FJson::FValue* pvalue = (&arr[0]);
        return list->ForEach(data, [this, &pvalue](const RTTI::FAtom& item) {
            _values.push_back(pvalue);
            bool r = item.Accept(this);
            _values.pop_back();

            ++pvalue;
            return r;
        });
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override {
        FJson::FArray& arr = Head_().SetType_AssumeNull(_doc, FJson::TypeArray{});
        arr.resize(dico->Count(data));

        if (arr.empty())
            return true;

        FJson::FValue* pvalue = (&arr[0]);
        return dico->ForEach(data, [this, &pvalue](const RTTI::FAtom& key, const RTTI::FAtom& value) {
            FJson::FArray& pair = pvalue->SetType_AssumeNull(_doc, FJson::TypeArray{});
            pair.resize(2);
            bool r = true;

            _values.push_back(&pair.front());
            r &= key.Accept(this);
            _values.pop_back();

            if (not r)
                return false;

            _values.push_back(&pair.back());
            r &= value.Accept(this);
            _values.pop_back();

            ++pvalue;
            return r;
        });
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
        virtual bool Visit(const RTTI::IScalarTraits* , T& value) override final { \
            ToJson_(value); \
            return true; \
        }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    FJson& _doc;
    HASHMAP(Json, const RTTI::FMetaObject*, FString) _visiteds;
    VECTORINSITU(Json, FJson::FValue*, 8) _values;

    FJson::FValue& Head_() const { return (*_values.back()); }

    template <typename T>
    void ToJson_(const T& value) {
        RTTI_to_Json_(_doc, Head_(), value);
    }

    void ToJson_(const RTTI::FAny& any) {
        if (any) {
            FJson::FArray& jsonArr = Head_().SetType_AssumeNull(_doc, FJson::TypeArray{});
            jsonArr.reserve(2);

            jsonArr.emplace_back(FJson::FInteger(any.Traits()->TypeId()));
            auto& wrapped = jsonArr.push_back_Default();

            _values.push_back(&wrapped);
            any.InnerAtom().Accept(this);
            _values.pop_back();
        }
        else {
            Assert(Head_().AsNull());
        }
    }

    void ToJson_(const RTTI::PMetaObject& obj) {
        if (obj) {
            FJson::FText& jsonTxt = Head_().SetType_AssumeNull(_doc, FJson::TypeString{});

            const auto it = _visiteds.find(obj.get());
            if (_visiteds.end() == it) {
                // import
                const RTTI::FPathName path{ RTTI::FPathName::FromObject(*obj) };
                jsonTxt = _doc.MakeString(ToString(path));
            }
            else {
                // internal
                jsonTxt = _doc.MakeString(it->second);
            }
        }
        else {
            Assert(Head_().AsNull());
        }
    }
};
//----------------------------------------------------------------------------
class FJson_to_RTTI_ : public RTTI::IAtomVisitor {
public:
    explicit FJson_to_RTTI_(FTransactionLinker& link)
        : _link(link)
    {}

    bool Parse(const FJson::FObject& jsonObj) {
        Assert_NoAssume(_values.empty());

        const FJson::FValue* const klassName = jsonObj.GetIFP(Json_Class());
        if (nullptr == klassName || nullptr == klassName->AsString()) {
            LOG(Serialize, Error, L"missing meta class name");
            return false;
        }

        const FJson::FValue* const objName = jsonObj.GetIFP(Json_Name());
        if (nullptr == objName || nullptr == objName->AsString()) {
            LOG(Serialize, Error, L"missing meta object name");
            return false;
        }

        bool topObject = false;
        const FJson::FValue* const objFlags = jsonObj.GetIFP(Json_Flags());
        if (objFlags) {
            const FJson::FInteger* const intFlags = objFlags->AsInteger();
            if (nullptr == intFlags)
                LOG(Serialize, Error, L"invalid meta object flags");
            else if (EJsonRTTI_(*intFlags) ^ EJsonRTTI_::TopObject)
                topObject = true;
        }

        const RTTI::FMetaClass* const klass = _link.ResolveClass(
            RTTI::FMetaDatabaseReadable{},
            RTTI::FName{ klassName->ToString() });

        if (nullptr == klass) {
            LOG(Serialize, Error, L"unknown meta class <{0}>", klassName->ToString());
            return false;
        }

        RTTI::PMetaObject rttiObj;
        Verify(klass->CreateInstance(rttiObj, true));

        if (topObject)
            _link.AddTopObject(rttiObj.get());

        Insert_AssertUnique(_visiteds, objName->ToString(), rttiObj);

        const FJson::FValue* const objProperties = jsonObj.GetIFP(Json_Properties());
        if (objProperties) {
            const FJson::FObject* const jsonProperties = objProperties->AsObject();
            if (nullptr == jsonProperties) {
                LOG(Serialize, Error, L"invalid meta object properties");
                return false;
            }

            for (const auto& it : *jsonProperties) {
                const RTTI::FMetaProperty* const prop = klass->PropertyIFP(it.first.MakeView());
                if (nullptr == prop) {
                    LOG(Serialize, Error, L"unknown meta property <{0}::{1}>", klassName->ToString(), it.first.MakeView());
                    return false;
                }

                Assert_NoAssume(_values.empty());
                _values.push_back(&it.second);

                const bool r = prop->Get(*rttiObj).Accept(this);

                Assert_NoAssume(1 == _values.size());
                Assert_NoAssume(&it.second == _values.front());
                _values.pop_back();

                if (not r)
                    return false;
            }
        }

        return true;
    }

    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        const FJson::FArray* arrIFP = Head_().AsArray();
        if (nullptr == arrIFP)
            return false;

        const size_t n = tuple->Arity();
        if (n != arrIFP->size())
            return false;

        forrange(i, 0, n) {
            _values.push_back(&(*arrIFP)[i]);
            const bool result = tuple->At(data, i).Accept(this);
            _values.pop_back();

            if (not result)
                return false;
        }

        return true;
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        const FJson::FArray* arrIFP = Head_().AsArray();
        if (nullptr == arrIFP)
            return false;

        const size_t n = arrIFP->size();
        list->Reserve(data, n);

        forrange(i, 0, n) {
            _values.push_back(&(*arrIFP)[i]);
            const bool result = list->AddDefault(data).Accept(this);
            _values.pop_back();

            if (not result)
                return false;
        }

        return true;
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override {
        const FJson::FArray* arrIFP = Head_().AsArray();
        if (nullptr == arrIFP)
            return false;

        const size_t n = arrIFP->size();
        dico->Reserve(data, n);

        STACKLOCAL_ATOM(keyData, dico->KeyTraits());
        RTTI::FAtom keyAtom = keyData.MakeAtom();

        forrange(i, 0, n) {
            const FJson::FValue& jsonIt = arrIFP->at(i);
            const FJson::FArray* pairIFP = jsonIt.AsArray();
            if (nullptr == pairIFP || 2 != pairIFP->size())
                return false;

            bool r = true;

            _values.push_back(&pairIFP->front());
            r &= keyAtom.Accept(this);
            _values.pop_back();

            if (not r)
                return false;

            RTTI::FAtom valueAtom = dico->AddDefaultMove(data, keyAtom);

            _values.push_back(&pairIFP->back());
            r &= valueAtom.Accept(this);
            _values.pop_back();

            if (not r)
                return false;

            keyAtom.ResetToDefaultValue();
        }

        return true;
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
        virtual bool Visit(const RTTI::IScalarTraits* scalar, T& value) override final { \
            return ToRTTI_(scalar, value); \
        }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    FTransactionLinker& _link;
    HASHMAP(Json, FJson::FText, RTTI::PMetaObject) _visiteds;
    VECTORINSITU(Json, const FJson::FValue*, 8) _values;

    const FJson::FValue& Head_() const { return (*_values.back()); }

    template <typename T>
    bool ToRTTI_(const RTTI::IScalarTraits* , T& value) {
        return Json_to_RTTI_(value, Head_());
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , RTTI::FAny& any) {
        if (Head_().AsNull())
            return true;

        const FJson::FArray* arrIFP = Head_().AsArray();
        if (nullptr == arrIFP || 2 != arrIFP->size())
            return false;

        const FJson::FInteger* numIFP = arrIFP->front().AsInteger();
        if (nullptr == numIFP)
            return false;

        const RTTI::ENativeType typeId = RTTI::ENativeType(*numIFP);
        const RTTI::FAtom anyAtom = any.Reset(RTTI::MakeTraits(typeId));

        _values.push_back(&arrIFP->back());
        bool r = anyAtom.Accept(this);
        _values.pop_back();

        return r;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* scalar, RTTI::PMetaObject& obj) {
        Assert_NoAssume(not obj);
        if (Head_().AsNull())
            return true;

        RTTI::FPathName path;

        const FJson::FText* const name = Head_().AsString();
        if (nullptr == name)
            PPE_THROW_IT(FJsonSerializerException("expected a Json string"));
        else if (name->empty())
            PPE_THROW_IT(FJsonSerializerException("expected a non empty Json string"));
        else if (not RTTI::FPathName::Parse(&path, name->MakeView()))
            PPE_THROW_IT(FJsonSerializerException("malformed RTTI path name"));

        const auto it = _visiteds.find(*name);
        if (_visiteds.end() == it) {
            // assuming this is an import
            obj.reset(_link.ResolveImport(
                RTTI::FMetaDatabaseReadable{},
                path, RTTI::PTypeTraits{ scalar }));
        }
        else {
            // retrieve from already parsed object
            obj = it->second;

            if (name->MakeView().StartsWith('~'))
                _link.AddExport(path.Identifier, obj);

#if !(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING) // unchecked assignment for optimized builds
            _link.CheckAssignment(RTTI::PTypeTraits{ scalar }, *obj);
#endif
        }

        return true;
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RTTI_to_Json(const FTransactionSaver& saved, class FJson* dst) {
    Assert(dst);

    FRTTI_to_Json_ toJson(*dst);
    toJson.reserve(saved.LoadedRefs().size());

    FJson::FArray& arr = dst->Root().SetType_AssumeNull(*dst, FJson::TypeArray{});
    arr.reserve_AssumeEmpty(saved.LoadedRefs().size());

    for (const RTTI::SMetaObject& ref : saved.LoadedRefs())
        toJson.Append(ref.get(), arr.push_back_Default());
}
//----------------------------------------------------------------------------
bool Json_to_RTTI(const class FJson& src, FTransactionLinker* link) {
    Assert(link);

    const FJson::FArray* arrIFP = src.Root().AsArray();
    if (nullptr == arrIFP) {
        LOG(Serialize, Error, L"top json entry should be an array");
        return false;
    }

    FJson_to_RTTI_ toRTTI(*link);

    forrange(i, 0, arrIFP->size()) {
        const FJson::FValue& item = arrIFP->at(i);
        if (not item.AsObject()) {
            LOG(Serialize, Error, L"all top entries should be json objects");
            return false;
        }

        if (not toRTTI.Parse(item.ToObject())) {
            LOG(Serialize, Error, L"failed to parse json object");
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
