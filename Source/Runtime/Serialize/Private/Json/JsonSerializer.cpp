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
#include "MetaEnum.h"
#include "MetaModule.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "Container/HashMap.h"
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
static RTTI::FName Json_Class()         { return RTTI::FName("Class"); }
static RTTI::FName Json_Name()          { return RTTI::FName("Name"); }
static RTTI::FName Json_Parent()        { return RTTI::FName("Parent"); }
static RTTI::FName Json_Flags()         { return RTTI::FName("Flags"); }
static RTTI::FName Json_Functions()     { return RTTI::FName("Functions"); }
static RTTI::FName Json_Module()        { return RTTI::FName("Module"); }
static RTTI::FName Json_Properties()    { return RTTI::FName("Properties"); }
static RTTI::FName Json_SizeInBytes()   { return RTTI::FName("SizeInBytes"); }
static RTTI::FName Json_Traits()        { return RTTI::FName("Traits"); }
static RTTI::FName Json_TypeId()        { return RTTI::FName("TypeId"); }
static RTTI::FName Json_Values()        { return RTTI::FName("Values"); }
//----------------------------------------------------------------------------
template <typename T, class = void>
struct TJson_RTTI_traits;
template <typename T>
void RTTI_to_Json_(FJson::FValue& dst, const T& src) {
    TJson_RTTI_traits<T>::Json(dst, src);
}
template <typename T>
bool Json_to_RTTI_(T& dst, const FJson::FValue& src) {
    return TJson_RTTI_traits<T>::RTTI(dst, src);
}
//----------------------------------------------------------------------------
template <>
struct TJson_RTTI_traits<FJson::FBool> {
    static void Json(FJson::FValue& dst, FJson::FBool src) {
        dst.Assign(src);
    }
    static bool RTTI(FJson::FBool& dst, const FJson::FValue& src) {
        const FJson::FBool* boolIFP = src.TypedConstDataIFP<FJson::FBool>();
        if (nullptr == boolIFP) return false;
        dst = *boolIFP;
        return true;
    }
};
template <typename T>
struct TJson_RTTI_traits<T, Meta::TEnableIf< std::is_integral<T>::value > > {
    static void Json(FJson::FValue& dst, T src) {
        dst.Assign(checked_cast<FJson::FInteger>(src));
    }
    static bool RTTI(T& dst, const FJson::FValue& src) {
        const FJson::FInteger* intIFP = src.TypedConstDataIFP<FJson::FInteger>();
        if (nullptr == intIFP) return false;
        dst = T(*intIFP);
        return true;
    }
};
template <typename T>
struct TJson_RTTI_traits<T, Meta::TEnableIf< std::is_floating_point<T>::value > > {
    static void Json(FJson::FValue& dst, T src) {
        dst.Assign(checked_cast<FJson::FFloat>(src));
    }
    static bool RTTI(T& dst, const FJson::FValue& src) {
        const FJson::FFloat* floatIFP = src.TypedConstDataIFP<FJson::FFloat>();
        if (nullptr == floatIFP) return false;
        dst = T(*floatIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FJson::FText> {
    static void Json(FJson::FValue& dst, const FJson::FText& src) {
        dst.Assign(src);
    }
    static bool RTTI(FString& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.TypedConstDataIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = *strIFP;
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FWString> {
    static void Json(FJson::FValue& dst, const FWString& src) {
        dst.Assign(ToString(src));
    }
    static bool RTTI(FWString& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.TypedConstDataIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<RTTI::FName> {
    static void Json(FJson::FValue& dst, const RTTI::FName& src) {
        dst.Assign(ToString(src.MakeView()));
    }
    static bool RTTI(RTTI::FName& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.TypedConstDataIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = RTTI::FName(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FDirpath> {
    static void Json(FJson::FValue& dst, const FDirpath& src) {
        dst.Assign(src.ToString());
    }
    static bool RTTI(FDirpath& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.TypedConstDataIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FFilename> {
    static void Json(FJson::FValue& dst, const FFilename& src) {
        dst.Assign(src.ToString());
    }
    static bool RTTI(FFilename& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.TypedConstDataIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<RTTI::FBinaryData> {
    static void Json(FJson::FValue& dst, const RTTI::FBinaryData& src) {
        dst.Assign(ToString(src.MakeConstView().Cast<const char>()));
    }
    static bool RTTI(RTTI::FBinaryData& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.TypedConstDataIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst.CopyFrom(strIFP->MakeView().Cast<const u8>());
        return true;
    }
};
template <typename T, size_t _Dim>
struct TJson_RTTI_traits<TScalarVector<T, _Dim>> {
    static void Json(FJson::FValue& dst, const TScalarVector<T, _Dim>& src) {
        FJson::FArray& arr = dst.MakeDefault_AssumeNotValid<FJson::FArray>();
        arr.resize(_Dim);
        forrange(i, 0, _Dim)
            RTTI_to_Json_(arr[i], src._data[i]);
    }
    static bool RTTI(TScalarVector<T, _Dim>& dst, const FJson::FValue& src) {
        const FJson::FArray* arrIFP = src.TypedConstDataIFP<FJson::FArray>();
        if (nullptr == arrIFP || _Dim != arrIFP->size()) return false;
        forrange(i, 0, _Dim)
            if (not Json_to_RTTI_(dst._data[i], arrIFP->at(i)))
                return false;
        return true;
    }
};
template <typename T, size_t _W, size_t _H>
struct TJson_RTTI_traits<TScalarMatrix<T, _W, _H>> {
    static void Json(FJson::FValue& dst, const TScalarMatrix<T, _W, _H>& src) {
        FJson::FArray& arr = dst.MakeDefault_AssumeNotValid<FJson::FArray>();
        arr.resize(_W);
        forrange(i, 0, _W)
            RTTI_to_Json_(arr[i], src.Column(i));
    }
    static bool RTTI(TScalarMatrix<T, _W, _H>& dst, const FJson::FValue& src) {
        const FJson::FArray* arrIFP = src.TypedConstDataIFP<FJson::FArray>();
        if (nullptr == arrIFP || _W != arrIFP->size()) return false;
        forrange(i, 0, _W)
            if (not Json_to_RTTI_(dst.Column(i), arrIFP->at(i)))
                return false;
        return true;
    }
};
//----------------------------------------------------------------------------
class FRTTI_to_Json_ final : public RTTI::IAtomVisitor {
public:
    FRTTI_to_Json_() = default;

    void reserve(size_t n) {
        _visiteds.reserve(n);
        _stack.reserve(n);
    }

    void Append(const RTTI::FAtom& atom, FJson::FValue& value) {
		Assert(atom);
		Assert_NoAssume(_stack.empty());

        VerifyRelease(Recurse_(atom, &value));
    }

    void Append(const RTTI::FMetaObject* ref, FJson::FValue& value) {
        Assert(ref);
        Assert_NoAssume(_stack.empty());

        FString name;
        if (ref->RTTI_IsExported()) {
            Assert_NoAssume(not ref->RTTI_Name().empty());

            name = ref->RTTI_Name().MakeView();
        }
        else { // generate temporary name (for internal references)
            Assert_NoAssume(ref->RTTI_Name().empty());

            name = StringFormat("{0}_{1}", ref->RTTI_Class()->Name(), _visiteds.size());
        }

        Insert_AssertUnique(_visiteds, ref, name);

        const RTTI::FMetaClass* metaClass = ref->RTTI_Class();

        FJson::FObject& jsonObject = value.MakeDefault_AssumeNotValid<FJson::FObject>();
        jsonObject.reserve(1/* class */ + 1/* name */ + 1/* props */);

        jsonObject.Add(Json_Class()).Assign(ToString(metaClass->Name().MakeView()));
        jsonObject.Add(Json_Name()).Assign(std::move(name));

        if (ref->RTTI_IsTopObject())
            jsonObject.Add(Json_Flags()).Assign(FJson::FInteger(EJsonRTTI_::TopObject));

        FJson::FObject* jsonProperties = nullptr;

        for (const RTTI::FMetaProperty* prop : metaClass->AllProperties()) {
            RTTI::FAtom atom = prop->Get(*ref);

            if (atom.IsDefaultValue())
                continue;

            if (nullptr == jsonProperties)
                jsonProperties = &jsonObject.Add(Json_Properties()).MakeDefault_AssumeNotValid<FJson::FObject>();

            VerifyRelease(Recurse_(atom, &jsonProperties->Add(prop->Name())));
        }
    }

    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        const size_t n = tuple->Arity();

        FJson::FArray& arr = Head_().MakeDefault_AssumeNotValid<FJson::FArray>();
        arr.resize(n);

        if (arr.empty())
            return true;

        FJson::FValue* pvalue = (&arr[0]);
        return tuple->ForEach(data, [this, &pvalue](const RTTI::FAtom& item) {
            return Recurse_(item, pvalue++);
        });
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        FJson::FArray& arr = Head_().MakeDefault_AssumeNotValid<FJson::FArray>();
        arr.resize(list->Count(data));

        if (arr.empty())
            return true;

        FJson::FValue* pvalue = (&arr[0]);
        return list->ForEach(data, [this, &pvalue](const RTTI::FAtom& item) {
            return Recurse_(item, pvalue++);
        });
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override {
        FJson::FArray& arr = Head_().MakeDefault_AssumeNotValid<FJson::FArray>();
        arr.resize(dico->Count(data));

        if (arr.empty())
            return true;

        FJson::FValue* pvalue = (&arr[0]);
        return dico->ForEach(data, [this, &pvalue](const RTTI::FAtom& key, const RTTI::FAtom& value) {
            FJson::FArray& pair = pvalue->MakeDefault_AssumeNotValid<FJson::FArray>();
            pair.resize_AssumeEmpty(2);
            ++pvalue;
            return (Recurse_(key, &pair.front()) &&
                    Recurse_(value, &pair.back()) );
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
    HASHMAP(Json, const RTTI::FMetaObject*, FString) _visiteds;
    VECTORINSITU(Json, FJson::FValue*, 8) _stack;

    FJson::FValue& Head_() const { return (*_stack.back()); }

    bool Recurse_(const RTTI::FAtom& src, FJson::FValue* pValue) NOEXCEPT {
        Assert(pValue);
        _stack.push_back(pValue);
        const bool result = src.Accept(this);
        Assert(_stack.back() == pValue);
        Assert(pValue->TypeId());
        _stack.pop_back();
        return result;
    }

    template <typename T>
    void ToJson_(const T& value) {
        RTTI_to_Json_(Head_(), value);
    }

    void ToJson_(const RTTI::FAny& any) {
        if (any) {
            FJson::FArray& arr = Head_().MakeDefault_AssumeNotValid<FJson::FArray>();
            arr.reserve(2);

            arr.emplace_back(FJson::FInteger(any.Traits()->TypeId()));
            auto& wrapped = arr.push_back_Default();

            VerifyRelease(Recurse_(any.InnerAtom(), &wrapped));
        }
        else {
            Head_().MakeDefault_AssumeNotValid<FJson::FNull>();
        }
    }

    void ToJson_(const RTTI::PMetaObject& obj) {
        if (obj) {
            FJson::FText& jsonTxt = Head_().MakeDefault_AssumeNotValid<FJson::FText>();

            const auto it = _visiteds.find(obj.get());
            if (_visiteds.end() == it) {
                // import
                const RTTI::FPathName path{ RTTI::FPathName::FromObject(*obj) };
                jsonTxt = ToString(path);
            }
            else {
                // internal
                jsonTxt = it->second;
            }
        }
        else {
            Head_().MakeDefault_AssumeNotValid<FJson::FNull>();
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
        if (nullptr == klassName || nullptr == klassName->TypedConstDataIFP<FJson::FText>()) {
            LOG_DIRECT(Serialize, Error, L"missing meta class name");
            return false;
        }

        const FJson::FValue* const objName = jsonObj.GetIFP(Json_Name());
        if (nullptr == objName || nullptr == objName->TypedConstDataIFP<FJson::FText>()) {
            LOG_DIRECT(Serialize, Error, L"missing meta object name");
            return false;
        }

        bool topObject = false;
        const FJson::FValue* const objFlags = jsonObj.GetIFP(Json_Flags());
        if (objFlags) {
            const FJson::FInteger* const intFlags = objFlags->TypedConstDataIFP<FJson::FInteger>();
            if (nullptr == intFlags)
                LOG_DIRECT(Serialize, Error, L"invalid meta object flags");
            else if (EJsonRTTI_(*intFlags) ^ EJsonRTTI_::TopObject)
                topObject = true;
        }

        const RTTI::FMetaClass* const klass = _link.ResolveClass(
            RTTI::FMetaDatabaseReadable{},
            RTTI::FName{ klassName->TypedConstData<FJson::FText>() });

        if (nullptr == klass) {
            LOG(Serialize, Error, L"unknown meta class <{0}>", klassName->TypedConstDataIFP<FJson::FText>());
            return false;
        }

        RTTI::PMetaObject rttiObj;
        Verify(klass->CreateInstance(rttiObj, true));

        if (topObject)
            _link.AddTopObject(rttiObj.get());

        Insert_AssertUnique(_visiteds, objName->TypedConstData<FJson::FText>(), rttiObj);

        const FJson::FValue* const objProperties = jsonObj.GetIFP(Json_Properties());
        if (objProperties) {
            const FJson::FObject* const jsonProperties = objProperties->TypedConstDataIFP<FJson::FObject>();
            if (nullptr == jsonProperties) {
                LOG_DIRECT(Serialize, Error, L"invalid meta object properties");
                return false;
            }

            for (const auto& it : *jsonProperties) {
                const RTTI::FMetaProperty* const prop = klass->PropertyIFP(it.first.MakeView());
                if (nullptr == prop) {
                    LOG(Serialize, Error, L"unknown meta property <{0}::{1}>",
                        klassName->TypedConstDataIFP<FJson::FText>(), it.first.MakeView());
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
        const FJson::FArray* arrIFP = Head_().TypedConstDataIFP<FJson::FArray>();
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
        const FJson::FArray* arrIFP = Head_().TypedConstDataIFP<FJson::FArray>();
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
        const FJson::FArray* arrIFP = Head_().TypedConstDataIFP<FJson::FArray>();
        if (nullptr == arrIFP)
            return false;

        const size_t n = arrIFP->size();
        dico->Reserve(data, n);

        STACKLOCAL_ATOM(keyData, dico->KeyTraits());
        RTTI::FAtom keyAtom = keyData.MakeAtom();

        forrange(i, 0, n) {
            const FJson::FValue& jsonIt = arrIFP->at(i);
            const FJson::FArray* pairIFP = jsonIt.TypedConstDataIFP<FJson::FArray>();
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
        if (Head_().TypedConstDataIFP<FJson::FNull>())
            return true;

        const FJson::FArray* arrIFP = Head_().TypedConstDataIFP<FJson::FArray>();
        if (nullptr == arrIFP || 2 != arrIFP->size())
            return false;

        const FJson::FInteger* numIFP = arrIFP->front().TypedConstDataIFP<FJson::FInteger>();
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
        if (Head_().TypedConstDataIFP<FJson::FNull>())
            return true;

        RTTI::FPathName path;

        const FJson::FText* const name = Head_().TypedConstDataIFP<FJson::FText>();
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
// RTTI -> Json
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FAny& any, FJson* dst) {
    Assert(dst);

    if (any) {
        FRTTI_to_Json_ toJson;
        toJson.Append(any.InnerAtom(), dst->Root());
    }
    else
        dst->Root().MakeDefault_AssumeNotValid<FJson::FNull>();
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FAtom& atom, FJson* dst) {
    Assert(dst);

    FRTTI_to_Json_ toJson;
    toJson.Append(atom, dst->Root());
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FMetaObject& obj, FJson* dst) {
    Assert(dst);

    FRTTI_to_Json_ toJson;
    toJson.Append(&obj, dst->Root());
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::PMetaObject& pobj, FJson* dst) {
    Assert(dst);

    if (pobj) {
        FRTTI_to_Json_ toJson;
        RTTI_to_Json(*pobj, dst);
    }
    else
        dst->Root().MakeDefault_AssumeNotValid<FJson::FNull>();
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const FTransactionSaver& saved, FJson* dst) {
	RTTI_to_Json(saved.LoadedRefs(), dst);
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const TMemoryView<const RTTI::SMetaObject>& objs, class FJson* dst) {
	Assert(dst);

	FRTTI_to_Json_ toJson;
	toJson.reserve(objs.size());

	FJson::FArray& arr = dst->Root().MakeDefault_AssumeNotValid<FJson::FArray>();
	arr.reserve_AssumeEmpty(objs.size());

	for (const RTTI::SMetaObject& ref : objs)
		toJson.Append(ref.get(), arr.push_back_Default());
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const TMemoryView<const RTTI::SCMetaTransaction>& mnamespace, class FJson* dst) {
    Assert(dst);

    const size_t numObjs = mnamespace.MapReduce(
        [](const RTTI::SCMetaTransaction& n) { return n->Linearized().LoadedRefs.size(); },
        [](size_t a, size_t b) { return a + b; });

	FRTTI_to_Json_ toJson;
	toJson.reserve(numObjs);

	FJson::FArray& arr = dst->Root().MakeDefault_AssumeNotValid<FJson::FArray>();
	arr.reserve_AssumeEmpty(numObjs);

    for (const RTTI::SCMetaTransaction& tr : mnamespace) {
        for (const RTTI::SMetaObject& ref : tr->Linearized().LoadedRefs)
            toJson.Append(ref.get(), arr.push_back_Default());
    }
}
//----------------------------------------------------------------------------
// Meta data reflection
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FMetaClass& mclass, FJson* dst) {
    Assert(dst);

    FJson::FObject& obj = dst->Root().MakeDefault_AssumeNotValid<FJson::FObject>();
    obj[Json_Name()].Assign(ToString(mclass.Name()));
    obj[Json_Flags()].Assign(ToString(mclass.Flags()));

    if (mclass.Module())
        obj[Json_Module()].Assign(ToString(mclass.Module()->Name()));

    if (mclass.Parent())
        obj[Json_Parent()].Assign(ToString(mclass.Parent()->Name()));

    FJson traits;
    RTTI_to_Json(mclass.MakeTraits(), &traits);
    obj[Json_Traits()] = std::move(traits.Root());

    FJson::FObject& funcs = obj[Json_Functions()].MakeDefault_AssumeNotValid<FJson::FObject>();
    for (const auto& it : mclass.AllFunctions()) {
        FJson prop;
        RTTI_to_Json(*it, &prop);
        funcs[it->Name()] = std::move(prop.Root());
    }

    FJson::FObject& props = obj[Json_Properties()].MakeDefault_AssumeNotValid<FJson::FObject>();
    for (const auto& it : mclass.AllProperties()) {
        FJson func;
        RTTI_to_Json(*it, &func);
        props[it->Name()] = std::move(func.Root());
    }
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FMetaEnum& menum, FJson* dst) {
	Assert(dst);

	FJson::FObject& obj = dst->Root().MakeDefault_AssumeNotValid<FJson::FObject>();
	obj[Json_Name()].Assign(ToString(menum.Name()));
	obj[Json_Flags()].Assign(ToString(menum.Flags()));

	if (menum.Module())
		obj[Json_Module()].Assign(ToString(menum.Module()->Name()));

	FJson traits;
	RTTI_to_Json(menum.MakeTraits(), &traits);
	obj[Json_Traits()] = std::move(traits.Root());

	FJson::FObject& values = obj[Json_Values()].MakeDefault_AssumeNotValid<FJson::FObject>();
	for (const auto& it : menum.Values()) {
		values[it.Name].Assign(it.Ord);
	}
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FMetaFunction& func, FJson* dst) {
    Assert(dst);

    FJson::FObject& obj = dst->Root().MakeDefault_AssumeNotValid<FJson::FObject>();
    obj[Json_Name()].Assign(ToString(func.Name()));
    obj[Json_Flags()].Assign(ToString(func.Flags()));

    FJson traits;
    if (func.Result()) {
        RTTI_to_Json(func.Result(), &traits);
        obj[RTTI::FName("Result")] = std::move(traits.Root());
    }

    FJson::FObject& prms = obj[RTTI::FName("Parameters")].MakeDefault_AssumeNotValid<FJson::FObject>();
    for (const RTTI::FMetaParameter& it : func.Parameters()) {
        FJson::FObject& prm = prms[it.Name()].MakeDefault_AssumeNotValid<FJson::FObject>();
        prm[Json_Name()].Assign(ToString(it.Name()));
        prm[Json_Flags()].Assign(ToString(it.Flags()));

        RTTI_to_Json(it.Traits(), &traits);
        prm[Json_Traits()] = std::move(traits.Root());
    }
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FMetaProperty& prop, FJson* dst) {
    Assert(dst);

    FJson::FObject& obj = dst->Root().MakeDefault_AssumeNotValid<FJson::FObject>();
    obj[Json_Name()].Assign(ToString(prop.Name()));
    obj[Json_Flags()].Assign(ToString(prop.Flags()));

    FJson traits;
    RTTI_to_Json(prop.Traits(), &traits);
    obj[Json_Traits()] = std::move(traits.Root());
}

//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FMetaModule& mod, FJson* dst) {
    Assert(dst);

    FJson::FObject& obj = dst->Root().MakeDefault_AssumeNotValid<FJson::FObject>();
    obj[Json_Name()].Assign(ToString(mod.Name()));

    FJson traits;

    FJson::FObject& classes = obj[RTTI::FName("Classes")].MakeDefault_AssumeNotValid<FJson::FObject>();
    for (const RTTI::FMetaClass* it : mod.Classes()) {
        FJson::FObject& mclass = classes[it->Name()].MakeDefault_AssumeNotValid<FJson::FObject>();
        mclass[Json_Name()].Assign(ToString(it->Name()));
        mclass[Json_Flags()].Assign(ToString(it->Flags()));

        RTTI_to_Json(it->MakeTraits(), &traits);
        mclass[Json_Traits()] = std::move(traits.Root());
    }

    FJson::FObject& enums = obj[RTTI::FName("Enums")].MakeDefault_AssumeNotValid<FJson::FObject>();
    for (const RTTI::FMetaEnum* it : mod.Enums()) {
        FJson::FObject& menum = enums[it->Name()].MakeDefault_AssumeNotValid<FJson::FObject>();
        menum[Json_Name()].Assign(ToString(it->Name()));
        menum[Json_Flags()].Assign(ToString(it->Flags()));

        RTTI_to_Json(it->MakeTraits(), &traits);
        menum[Json_Traits()] = std::move(traits.Root());
    }
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::PTypeTraits& traits, FJson* dst) {
    Assert(dst);
    Assert(traits);

    FJson::FObject& obj = dst->Root().MakeDefault_AssumeNotValid<FJson::FObject>();
    obj[Json_Name()].Assign(ToString(traits->TypeName())); // not relevant here
    obj[Json_TypeId()].Assign(checked_cast<i64>(traits->TypeId()));
    obj[Json_Flags()].Assign(ToString(traits->TypeFlags()));
    obj[Json_SizeInBytes()].Assign(checked_cast<i64>(traits->SizeInBytes()));
}
//----------------------------------------------------------------------------
// Json -> RTTI
//----------------------------------------------------------------------------
bool Json_to_RTTI(const FJson& src, FTransactionLinker* link) {
    Assert(link);

    const FJson::FArray* arrIFP = src.Root().TypedConstDataIFP<FJson::FArray>();
    if (nullptr == arrIFP) {
        LOG_DIRECT(Serialize, Error, L"top json entry should be an array");
        return false;
    }

    FJson_to_RTTI_ toRTTI(*link);

    forrange(i, 0, arrIFP->size()) {
        const FJson::FValue& item = arrIFP->at(i);
        if (not item.TypedConstDataIFP<FJson::FObject>()) {
            LOG_DIRECT(Serialize, Error, L"all top entries should be json objects");
            return false;
        }

        if (not toRTTI.Parse(item.FlatData<FJson::FObject>())) {
            LOG_DIRECT(Serialize, Error, L"failed to parse json object");
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
