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
#include "Maths/ScalarVector.h"

#include "TransactionLinker.h"
#include "TransactionSaver.h"
#include "IO/FormatHelpers.h"

namespace PPE {
namespace Serialize {
EXTERN_LOG_CATEGORY(PPE_SERIALIZE_API, Serialize)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FDiscardJson_ {
    FJson::FAllocator alloc;
    FJson json{ alloc };

    FDiscardJson_() = default;
    ~FDiscardJson_() {
        json.Clear_ForgetMemory();
    }
};
//----------------------------------------------------------------------------
} //!namespace
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

    FDiscardJson_ doc;

    UsingBufferedStream(&input, [&doc, linker](IBufferedStreamReader* buffered) {
        if (not FJson::Load(&doc.json, linker->Filename(), buffered))
            PPE_THROW_IT(FJsonSerializerException("failed to parse Json document"));
    });

    if (not Json_to_RTTI(doc.json, linker))
        PPE_THROW_IT(FJsonSerializerException("failed to convert Json to RTTI"));
}
//----------------------------------------------------------------------------
void FJsonSerializer::Serialize(const FTransactionSaver& saver, IStreamWriter* output) const {
    Assert(output);

    FDiscardJson_ doc;

    RTTI_to_Json(saver, &doc.json);

    UsingBufferedStream(output, [this, &doc](IBufferedStreamWriter* buffered) {
        FTextWriter oss(buffered);
        doc.json.ToStream(oss, Minify());
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
template <typename T, class = void>
struct TJson_RTTI_traits;
template <typename T>
void RTTI_to_Json_(FJson::FValue& dst, FJson& doc, const T& src) {
    TJson_RTTI_traits<T>::Json(dst, doc, src);
}
template <typename T>
bool Json_to_RTTI_(T& dst, const FJson::FValue& src) {
    return TJson_RTTI_traits<T>::RTTI(dst, src);
}
//----------------------------------------------------------------------------
template <>
struct TJson_RTTI_traits<FJson::FBool> {
    static void Json(FJson::FValue& dst, FJson& , FJson::FBool src) {
        dst.Assign(src);
    }
    static bool RTTI(FJson::FBool& dst, const FJson::FValue& src) {
        const FJson::FBool* boolIFP = src.GetIFP<FJson::FBool>();
        if (nullptr == boolIFP) return false;
        dst = *boolIFP;
        return true;
    }
};
template <typename T>
struct TJson_RTTI_traits<T, Meta::TEnableIf< std::is_integral<T>::value > > {
    static void Json(FJson::FValue& dst, FJson& , T src) {
        dst.Assign(checked_cast<FJson::FInteger>(src));
    }
    static bool RTTI(T& dst, const FJson::FValue& src) {
        const FJson::FInteger* intIFP = src.GetIFP<FJson::FInteger>();
        if (nullptr == intIFP) return false;
        dst = T(*intIFP);
        return true;
    }
};
template <typename T>
struct TJson_RTTI_traits<T, Meta::TEnableIf< std::is_floating_point<T>::value > > {
    static void Json(FJson::FValue& dst, FJson& , T src) {
        dst.Assign(checked_cast<FJson::FFloat>(src));
    }
    static bool RTTI(T& dst, const FJson::FValue& src) {
        const FJson::FFloat* floatIFP = src.GetIFP<FJson::FFloat>();
        if (nullptr == floatIFP) return false;
        dst = T(*floatIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FString> {
    static void Json(FJson::FValue& dst, FJson& doc, const FString& src) {
        dst.Assign(doc.MakeText(src, false));
    }
    static bool RTTI(FString& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.GetIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = *strIFP;
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FWString> {
    static void Json(FJson::FValue& dst, FJson& doc, const FWString& src) {
        dst.Assign(doc.MakeText(WCHAR_TO_UTF_8(src), false));
    }
    static bool RTTI(FWString& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.GetIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<RTTI::FName> {
    static void Json(FJson::FValue& dst, FJson& , const RTTI::FName& src) {
        dst.Assign(FJson::LiteralText(src.MakeView()));
    }
    static bool RTTI(RTTI::FName& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.GetIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = RTTI::FName(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FDirpath> {
    static void Json(FJson::FValue& dst, FJson& doc, const FDirpath& src) {
        dst.Assign(doc.MakeText(src.ToString()));
    }
    static bool RTTI(FDirpath& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.GetIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<FFilename> {
    static void Json(FJson::FValue& dst, FJson& doc, const FFilename& src) {
        dst.Assign(doc.MakeText(src.ToString()));
    }
    static bool RTTI(FFilename& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.GetIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst = ToWString(*strIFP);
        return true;
    }
};
template <>
struct TJson_RTTI_traits<RTTI::FBinaryData> {
    static void Json(FJson::FValue& dst, FJson& doc, const RTTI::FBinaryData& src) {
        const size_t len = Base64EncodeSize(src.MakeView());
        char* const data = static_cast<char*>(doc.Heap().Allocate(sizeof(char) * len));
        Base64Encode(src.MakeView(), { data, len });
        dst.Assign(FJson::LiteralText({ data, len }));
    }
    static bool RTTI(RTTI::FBinaryData& dst, const FJson::FValue& src) {
        const FJson::FText* strIFP = src.GetIFP<FJson::FText>();
        if (nullptr == strIFP) return false;
        dst.Resize_DiscardData(Base64DecodeSize(strIFP->MakeView()));
        return Base64Decode(strIFP->MakeView(), dst.MakeView());
    }
};
//----------------------------------------------------------------------------
class FRTTI_to_Json_ final : public RTTI::IAtomVisitor, Meta::FNonCopyable {
public:
    enum EFlags : u32 {
        Unknown     = 0,
        Inlined     = 1<<0,
    };

    explicit FRTTI_to_Json_(FJson& doc) NOEXCEPT : FRTTI_to_Json_(doc, Unknown) {}
    FRTTI_to_Json_(FJson& doc, EFlags flags) NOEXCEPT
    :   _doc(doc)
    ,   _flags(flags)
    {}

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

        const RTTI::FMetaClass* metaClass = ref->RTTI_Class();

        FJson::FObject& jsonObject = value.Construct<FJson::FObject>(_doc.Heap());
        jsonObject.reserve(1/* class */ + 1/* name */ + 1/* props */ + 1/* anchor */);

        FJson::FText anchor;
        if (ref->RTTI_IsExported()) {
            Assert_NoAssume(not ref->RTTI_Name().empty());

            jsonObject.Add(FJson::Export).Assign(
                FJson::LiteralText(ref->RTTI_Name().MakeView()));

            anchor = _doc.MakeText(ref->RTTI_Name());
        }
        else { // generate temporary name (for internal references)
            Assert_NoAssume(ref->RTTI_Name().empty());

            anchor = _doc.MakeText(StringFormat("{0}_{1}",
                ref->RTTI_Class()->Name(), _visiteds.size() ));
        }

        if (not anchor.empty()) {
            jsonObject.Add(FJson::Id).Assign(anchor);
            Insert_AssertUnique(_visiteds, ref, anchor);
        }

        jsonObject.Add(FJson::Class).Assign(FJson::LiteralText(metaClass->Name()));

        if (ref->RTTI_IsTopObject())
            jsonObject.Add(FJson::TopObject).Assign(true);

        for (const RTTI::FMetaProperty* prop : metaClass->AllProperties()) {
            RTTI::FAtom atom = prop->Get(*ref);

            if (atom.IsDefaultValue())
                continue;

            auto key = FJson::LiteralText(prop->Name());
            Assert_NoAssume(not FJson::IsReservedKeyword(key));

            VerifyRelease( Recurse_(atom, &jsonObject.Add(std::move(key))) );
        }
    }

    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        FJson::FArray& arr = Head_().Construct<FJson::FArray>(_doc.Heap());
        const size_t n = tuple->Arity();
        if (not n)
            return true;

        Reserve(arr, n);

        return tuple->ForEach(data, [this, &arr](const RTTI::FAtom& item) {
            const auto it = Emplace_Back(arr);
            return Recurse_(item, std::addressof(*it));
        });
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        FJson::FArray& arr = Head_().Construct<FJson::FArray>(_doc.Heap());
        const size_t n = list->Count(data);
        if (not n)
            return true;

        Reserve(arr, n);

        return list->ForEach(data, [this, &arr](const RTTI::FAtom& item) {
            const auto it = Emplace_Back(arr);
            return Recurse_(item, std::addressof(*it));
        });
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override {
        FJson::FArray& arr = Head_().Construct<FJson::FArray>(_doc.Heap());
        const size_t n = dico->Count(data);
        if (not n)
            return true;

        Reserve(arr, n);

        return dico->ForEach(data, [this, &arr](const RTTI::FAtom& key, const RTTI::FAtom& value) {
            FJson::FArray& pair = Emplace_Back(arr)->Construct<FJson::FArray>(_doc.Heap());
            pair.Reserve(2);
            return (Recurse_(key, std::addressof(*Emplace_Back(pair))) &&
                    Recurse_(value, std::addressof(*Emplace_Back(pair))) );
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
    HASHMAP(Json, const RTTI::FMetaObject*, FJson::FText) _visiteds;
    VECTORINSITU(Json, FJson::FValue*, 8) _stack;
    FJson& _doc;
    EFlags _flags;

    FJson::FValue& Head_() const { return (*_stack.back()); }

    NODISCARD bool Recurse_(const RTTI::FAtom& src, FJson::FValue* pValue) NOEXCEPT {
        Assert(pValue);
        _stack.push_back(pValue);
        const bool result = src.Accept(this);
        Assert(_stack.back() == pValue);
        _stack.pop_back();
        return result;
    }

    template <typename T>
    void ToJson_(const T& value) {
        RTTI_to_Json_(Head_(), _doc, value);
    }

    void ToJson_(const RTTI::FAny& any) {
        if (any) {
            FJson::FObject& obj = Head_().Construct<FJson::FObject>(_doc.Heap());
            obj.reserve(2);

            obj.Add(FJson::TypeId).Assign(FJson::LiteralText(any.Traits()->TypeName()));

            FJson::FValue& inner = obj.Add(FJson::Inner);
            VerifyRelease( Recurse_(any.InnerAtom(), std::addressof(inner)) );
        }
        else {
            Head_().Construct<FJson::FNull>();
        }
    }

    void ToJson_(const RTTI::PMetaObject& obj) {
        if (obj) {
            FJson::FText link;
            const auto it = _visiteds.find(obj.get());
            if (_visiteds.end() == it) {
                if (!(_flags & Inlined) && obj->RTTI_IsExported()) {
                    // import
                    const RTTI::FPathName path{ RTTI::FPathName::FromObject(*obj) };
                    link = _doc.Text().MakeText(ToString(path));
                }
                else {
                    Append(obj.get(), Head_());
                    return;
                }
            }
            else {
                // internal
                link = it->second;
            }

            FJson::FObject& jsonObj = Head_().Construct<FJson::FObject>(_doc.Heap());
            jsonObj.Add(FJson::Ref).Assign(std::move(link));
        }
        else {
            Head_().Construct<FJson::FNull>();
        }
    }
};
//----------------------------------------------------------------------------
class FJson_to_RTTI_ : public RTTI::IAtomVisitor {
public:
    explicit FJson_to_RTTI_(FTransactionLinker& link) NOEXCEPT
    :   _link(link)
    {}

    bool ParseAtom(const RTTI::FAtom& dst, const FJson::FValue& src) {
        Assert(dst);
        Assert(src.Valid());

        if (const FJson::FObject* const pJsonObj = src.AsObject()) {
            RTTI::PMetaObject rttiObj;
            if (Unlikely(not ParseObject(&rttiObj, *pJsonObj))) {
                LOG(Serialize, Error, L"failed to parse json object for <{0}>", dst.Traits()->NamedTypeInfos());
                return false;
            }

            Assert(rttiObj);

            if (not MakeAtom(&rttiObj).PromoteMove(dst)) {
                LOG(Serialize, Error, L"incompatible object <{0}> parsed from json for <{1}>", rttiObj->RTTI_Class()->Name(), dst.Traits()->NamedTypeInfos());
                return false;
            }
        }
        else {
            _values.push_back(std::addressof(src));

            if (Unlikely(not dst.Accept(this))) {
                LOG(Serialize, Error, L"failed to parse json value for <{0}>", dst.Traits()->NamedTypeInfos());
                return false;
            }

            Verify(_values.pop_back_ReturnBack() == std::addressof(src));
        }

        return true;
    }

    bool ParseObject(RTTI::PMetaObject* outRttiObj, const FJson::FObject& jsonObj) {
        Assert_NoAssume(_values.empty());

        const FJson::FValue* const klassName = jsonObj.GetIFP(FJson::Class);
        if (nullptr == klassName || nullptr == klassName->GetIFP<FJson::FText>()) {
            LOG_DIRECT(Serialize, Error, L"missing meta class name");
            return false;
        }

        const FJson::FValue* const objAnchor = jsonObj.GetIFP(FJson::Id);
        if (nullptr == objAnchor || nullptr == objAnchor->GetIFP<FJson::FText>()) {
            LOG_DIRECT(Serialize, Error, L"missing meta object anchor");
            return false;
        }

        bool exported = false;
        const FJson::FText* objName = nullptr;
        if (const FJson::FValue* const jsonExported = jsonObj.GetIFP(FJson::Export)) {
            objName = jsonExported->GetIFP<FJson::FText>();
            if (nullptr == objName) {
                LOG_DIRECT(Serialize, Error, L"wrong type for meta object export");
                return false;
            }
            if (objName->empty()) {
                LOG_DIRECT(Serialize, Error, L"empty meta object export");
                return false;
            }
            if (not RTTI::FName::IsValidToken(objName->MakeView())) {
                LOG_DIRECT(Serialize, Error, L"invalid meta object export format");
                return false;
            }
            exported = true;
        }

        bool topObject = false;
        if (const FJson::FValue* const jsonTopObject = jsonObj.GetIFP(FJson::TopObject)) {
            const FJson::FBool* const boolTopObject = jsonTopObject->GetIFP<FJson::FBool>();
            if (nullptr == boolTopObject) {
                LOG_DIRECT(Serialize, Error, L"invalid meta object top object flag");
                return false;
            }
            topObject = *boolTopObject;
        }

        const RTTI::FMetaClass* const klass = _link.ResolveClass(
            RTTI::FMetaDatabaseReadable{},
            RTTI::FName{ klassName->Get<FJson::FText>() });

        if (nullptr == klass) {
            LOG(Serialize, Error, L"unknown meta class <{0}>", klassName->GetIFP<FJson::FText>());
            return false;
        }

        RTTI::PMetaObject rttiObj;
        Verify(klass->CreateInstance(rttiObj, true));

        if (topObject)
            _link.AddTopObject(rttiObj.get());

        Insert_AssertUnique(_visiteds, objAnchor->Get<FJson::FText>(), rttiObj);

        for (const RTTI::FMetaProperty* prop : klass->AllProperties()) {
            const FJson::FText key = FJson::LiteralText(prop->Name());

            const auto it = jsonObj.find(key);
            if (jsonObj.end() == it)
                continue; // assume default value for missing properties

            Assert_NoAssume(_values.empty());
            _values.push_back(&it->second);

            const bool valid = prop->Get(*rttiObj).Accept(this);

            Assert_NoAssume(1 == _values.size());
            Assert_NoAssume(&it->second == _values.front());
            Verify(_values.pop_back_ReturnBack() == &it->second);

            if (not valid)
                return false;
        }

        if (exported)
            rttiObj->RTTI_Export(RTTI::FName{ *objName });

        if (outRttiObj)
            *outRttiObj = std::move(rttiObj);

        return true;
    }

    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        const FJson::FArray* const arrIFP = Head_().GetIFP<FJson::FArray>();
        if (nullptr == arrIFP)
            return false;

        const size_t n = tuple->Arity();
        if (n != arrIFP->size())
            return false;

        size_t i = 0;
        for (const FJson::FValue& item : *arrIFP) {
            _values.push_back(std::addressof(item));
            const bool result = tuple->At(data, i++).Accept(this);
            Verify(_values.pop_back_ReturnBack() == std::addressof(item));

            if (not result)
                return false;
        }

        Assert_NoAssume(n == i);
        return true;
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        const FJson::FArray* const arrIFP = Head_().GetIFP<FJson::FArray>();
        if (nullptr == arrIFP)
            return false;

        const size_t n = arrIFP->size();
        list->Reserve(data, n);

        for (const FJson::FValue& item : *arrIFP) {
            _values.push_back(std::addressof(item));
            const bool result = list->AddDefault(data).Accept(this);
            Verify(_values.pop_back_ReturnBack() == std::addressof(item));

            if (not result)
                return false;
        }

        return true;
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override {
        const FJson::FArray* const arrIFP = Head_().GetIFP<FJson::FArray>();
        if (nullptr == arrIFP)
            return false;

        const size_t n = arrIFP->size();
        dico->Reserve(data, n);

        STACKLOCAL_ATOM(keyData, dico->KeyTraits());
        const RTTI::FAtom keyAtom = keyData.MakeAtom();

        for (const FJson::FValue& jsonIt : *arrIFP) {
            const FJson::FArray* pairIFP = jsonIt.GetIFP<FJson::FArray>();
            if (nullptr == pairIFP || 2 != pairIFP->size())
                return false;

            auto pairIt = pairIFP->begin();

            bool r = true;

            _values.push_back(&*pairIt);
            r &= keyAtom.Accept(this);
            Verify(_values.pop_back_ReturnBack() == &*pairIt++);

            if (not r)
                return false;

            RTTI::FAtom valueAtom = dico->AddDefaultMove(data, keyAtom);

            _values.push_back(&*pairIt);
            r &= valueAtom.Accept(this);
            Verify(_values.pop_back_ReturnBack() == &*pairIt++);

            if (not r)
                return false;

            Assert_NoAssume(pairIFP->end() == pairIt);
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
    static const T* GetIFP_(const FJson::FObject& obj, const FJson::FText& name) NOEXCEPT {
        if (const FJson::FValue* const valueIFP = obj.GetIFP(name))
            return valueIFP->GetIFP<T>();
        return nullptr;
    }

    template <typename T>
    bool ToRTTI_(const RTTI::IScalarTraits* , T& value) {
        return Json_to_RTTI_(value, Head_());
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , RTTI::FAny& any) {
        if (Head_().GetIFP<FJson::FNull>())
            return true;

        const FJson::FObject* objIFP = Head_().GetIFP<FJson::FObject>();
        if (nullptr == objIFP || objIFP->size() < 2)
            PPE_THROW_IT(FJsonSerializerException("expected a Json object"));

        auto* pTypeId= GetIFP_<FJson::FText>(*objIFP, FJson::TypeId);
        if (not pTypeId)
            PPE_THROW_IT(FJsonSerializerException("Any value must have 'type_id' property"));

        auto* pInnerValue= objIFP->GetIFP(FJson::Inner);
        if (not pInnerValue)
            PPE_THROW_IT(FJsonSerializerException("Any value must have 'inner' property"));

        {
            RTTI::FMetaDatabaseReadable db;
            if (const RTTI::PTypeTraits traits = db->TraitsIFP(pTypeId->MakeView()))
                any.Reset(traits);
            else
                PPE_THROW_IT(FJsonSerializerException("unknown traits"));
        }

        _values.push_back(pInnerValue);
        const bool result = any.InnerAtom().Accept(this);
        Verify(_values.pop_back_ReturnBack() == pInnerValue);

        return result;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* scalar, RTTI::PMetaObject& obj) {
        Assert_NoAssume(not obj);
        if (Head_().GetIFP<FJson::FNull>())
            return true;

        const FJson::FObject* const span = Head_().GetIFP<FJson::FObject>();
        if (nullptr == span)
            PPE_THROW_IT(FJsonSerializerException("expected a Json object"));
        if (span->empty())
            PPE_THROW_IT(FJsonSerializerException("expected a non empty Json object"));

        const FJson::FValue* const ref = span->GetIFP(FJson::Ref);
        if (nullptr == span)
            PPE_THROW_IT(FJsonSerializerException("expected a Json $ref"));

        RTTI::FPathName path;

        const FJson::FText* const name = ref->GetIFP<FJson::FText>();
        if (nullptr == name)
            PPE_THROW_IT(FJsonSerializerException("expected a Json string"));
        if (name->empty())
            PPE_THROW_IT(FJsonSerializerException("expected a non empty Json string"));
        if (not RTTI::FPathName::Parse(&path, name->MakeView()))
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
        FRTTI_to_Json_ toJson(*dst, FRTTI_to_Json_::Inlined);
        toJson.Append(any.InnerAtom(), dst->Root());
    }
    else
        dst->Root().Construct<FJson::FNull>();
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FAtom& atom, FJson* dst) {
    Assert(dst);

    FRTTI_to_Json_ toJson(*dst, FRTTI_to_Json_::Inlined);
    toJson.Append(atom, dst->Root());
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FMetaObject& obj, FJson* dst) {
    Assert(dst);

    FRTTI_to_Json_ toJson(*dst, FRTTI_to_Json_::Inlined);
    toJson.Append(&obj, dst->Root());
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::PMetaObject& pobj, FJson* dst) {
    Assert(dst);

    if (pobj) {
        FRTTI_to_Json_ toJson(*dst, FRTTI_to_Json_::Inlined);
        RTTI_to_Json(*pobj, dst);
    }
    else
        dst->Root().Construct<FJson::FNull>();
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const FTransactionSaver& saved, FJson* dst) {
    RTTI_to_Json(saved.LoadedRefs(), dst);
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const TMemoryView<const RTTI::SMetaObject>& objs, class FJson* dst) {
    Assert(dst);

    FRTTI_to_Json_ toJson(*dst);
    toJson.reserve(objs.size());

    FJson::FArray& arr = dst->Root().Construct<FJson::FArray>(dst->Heap());
    Reserve(arr, objs.size());

    for (const RTTI::SMetaObject& ref : objs)
        toJson.Append(ref.get(), *Emplace_Back(arr));
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const TMemoryView<const RTTI::SCMetaTransaction>& mnamespace, class FJson* dst) {
    Assert(dst);

    const size_t numObjs = mnamespace.MapReduce(
        [](const RTTI::SCMetaTransaction& n) NOEXCEPT {
            return n->Linearized().LoadedRefs.size();
        },
        Meta::TPlus<>{});

    FRTTI_to_Json_ toJson(*dst);
    toJson.reserve(numObjs);

    FJson::FArray& arr = dst->Root().Construct<FJson::FArray>(dst->Heap());
    Reserve(arr, numObjs);

    for (const RTTI::SCMetaTransaction& tr : mnamespace) {
        for (const RTTI::SMetaObject& ref : tr->Linearized().LoadedRefs)
            toJson.Append(ref.get(), *Emplace_Back(arr));
    }
}
//----------------------------------------------------------------------------
// Json -> RTTI
//----------------------------------------------------------------------------
bool Json_to_RTTI(const FJson& src, FTransactionLinker* link) {
    Assert(link);

    const FJson::FArray* arrIFP = src.Root().GetIFP<FJson::FArray>();
    if (nullptr == arrIFP) {
        LOG_DIRECT(Serialize, Error, L"top json entry should be an array");
        return false;
    }

    FJson_to_RTTI_ toRTTI(*link);

    for (const FJson::FValue& item : *arrIFP) {
        if (not item.GetIFP<FJson::FObject>()) {
            LOG_DIRECT(Serialize, Error, L"all top entries should be json objects");
            return false;
        }

        if (not toRTTI.ParseObject(nullptr, item.Get<FJson::FObject>())) {
            LOG_DIRECT(Serialize, Error, L"failed to parse json object");
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool Json_to_RTTI(const RTTI::FAtom& dst, const FJson& src, FTransactionLinker* link) {
    Assert(link);

    FJson_to_RTTI_ toRTTI(*link);
    return toRTTI.ParseAtom(dst, src.Root());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
