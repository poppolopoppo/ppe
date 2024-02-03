// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
#include "IO/TextFormat.h"
#include "IO/StreamProvider.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Maths/ScalarVector.h"

#include "TransactionLinker.h"
#include "TransactionSaver.h"
#include "External/glslang/glslang.zip/windows-x64-Release/include/glslang/SPIRV/SpvBuilder.h"
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
class FRTTI_to_Json_ final : public RTTI::IAtomVisitor, Meta::FNonCopyable {
public:
    enum EFlags : u32 {
        Unknown     = 0,
        Inlined     = 1<<0,
    };

    explicit FRTTI_to_Json_(FJson& doc) NOEXCEPT : FRTTI_to_Json_(doc, Unknown) {}

    FRTTI_to_Json_(FJson& doc, EFlags flags) NOEXCEPT
    :   FRTTI_to_Json_(doc, doc.Root(), flags)
    {}

    FRTTI_to_Json_(FJson& doc, FJson::FValue& root, EFlags flags) NOEXCEPT
    :   _doc(doc)
    ,   _builder(root, doc.Heap())
    ,   _flags(flags) {
        _builder.SetTextMemoization(doc.AnsiText());
        _builder.SetTextMemoization(doc.WideText());
    }

    void Reset(TPtrRef<FJson::FValue> root) {
        _builder.Reset(root);
    }

    void reserve(size_t n) {
        _visiteds.reserve(n);
    }

    bool Append(const RTTI::FAtom& atom) {
        Assert(atom);

        return atom.Accept(this);
    }

    bool Append(const RTTI::FMetaObject* ref) {
        Assert(ref);

        const RTTI::FMetaClass* metaClass = ref->RTTI_Class();

        _builder.BeginObject(1/* class */ + 1/* name */ + 1/* props */ + 1/* anchor */);
        DEFERRED{ _builder.EndObject(); };

        FJson::FText anchor{ForceInit};
        if (ref->RTTI_IsExported()) {
            Assert_NoAssume(not ref->RTTI_Name().empty());

            anchor = _doc.AnsiText().Append(ref->RTTI_Name());

            _builder.KeyValue(FJson::Export, anchor);
        }
        else { // generate temporary name (for internal references)
            Assert_NoAssume(ref->RTTI_Name().empty());

            anchor = TextFormat(TSlabAllocator{ _doc.Heap() }, "{0}_{1}"_literal,
                ref->RTTI_Class()->Name(), _visiteds.size() );
        }

        if (not anchor.empty()) {
            Insert_AssertUnique(_visiteds, ref, anchor);

            _builder.KeyValue(FJson::Id, anchor);
        }

        _builder.KeyValue(FJson::Class, metaClass->Name().MakeLiteral());

        if (ref->RTTI_IsTopObject())
            _builder.KeyValue(FJson::TopObject, true);

        for (const RTTI::FMetaProperty* prop : metaClass->AllProperties()) {
            RTTI::FAtom atom = prop->Get(*ref);

            if (atom.IsDefaultValue())
                continue;

            const FJson::FText key = prop->Name().MakeLiteral();
            Assert_NoAssume(not FJson::IsReservedKeyword(key));

            _builder.BeginKeyValue(key);
            DEFERRED{ _builder.EndKeyValue(); };

            if (not Append(atom))
                return false;
        }

        return true;
    }

    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        const size_t n = tuple->Arity();

        _builder.BeginArray(n);
        DEFERRED{ _builder.EndArray(); };

        if (not n)
            return true;

        return tuple->ForEach(data, [&](const RTTI::FAtom& item) {
            return Append(item);
        });
    }

    virtual bool Visit(const RTTI::IListTraits* list, void* data) override final {
        const size_t n = list->Count(data);

        _builder.BeginArray(n);
        DEFERRED{ _builder.EndArray(); };

        if (not n)
            return true;

        return list->ForEach(data, [&](const RTTI::FAtom& item) {
            return Append(item);
        });
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, void* data) override {
        const size_t n = dico->Count(data);

        _builder.BeginArray(n);
        DEFERRED { _builder.EndArray(); };

        if (not n)
            return true;

        return dico->ForEach(data, [&](const RTTI::FAtom& key, const RTTI::FAtom& value) {
            _builder.BeginArray(2/* pair */);
            DEFERRED { _builder.EndArray(); };
            return Append(key) && Append(value);
        });
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
        virtual bool Visit(const RTTI::IScalarTraits* , T& value) override final { \
            return ToJson_(value); \
        }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    HASHMAP(Json, const RTTI::FMetaObject*, FJson::FText) _visiteds;
    FJson& _doc;
    FJson::FBuilder _builder;
    EFlags _flags;

    bool ToJson_(FJson::FBool value) {
        _builder.Write(value);
        Assert_NoAssume(std::get<FJson::FBool>(*_builder.Peek()) == value);
        return true;
    }

    template <typename T, Meta::TEnableIf< std::is_integral_v<T> && std::is_signed_v<T> >* = nullptr>
    bool ToJson_(T value) {
        _builder.Write(checked_cast<FJson::FInteger>(value));
        Assert_NoAssume(std::get<FJson::FInteger>(*_builder.Peek()) == value);
        return true;
    }
    template <typename T, Meta::TEnableIf< std::is_integral_v<T> && std::is_unsigned_v<T> >* = nullptr>
    bool ToJson_(T value) {
        _builder.Write(checked_cast<Opaq::uinteger>(value));
        Assert_NoAssume(std::get<Opaq::uinteger>(*_builder.Peek()) == value);
        return true;
    }
    template <typename T, Meta::TEnableIf< std::is_floating_point_v<T> >* = nullptr>
    bool ToJson_(T value) {
        _builder.Write(static_cast<FJson::FFloat>(value));
        Assert_NoAssume(std::get<FJson::FFloat>(*_builder.Peek()) == value);
        return true;
    }

    bool ToJson_(const FString& str) {
        _builder.Write(str.MakeView());
        Assert_NoAssume(std::get<FJson::FText>(*_builder.Peek()) == str.MakeView());
        return true;
    }

    bool ToJson_(const FWString& str) {
        _builder.Write(str.MakeView());
        Assert_NoAssume(std::get<FJson::FWText>(*_builder.Peek()) == str.MakeView());
        return true;
    }

    bool ToJson_(const RTTI::FName& name) {
        _builder.Write(name.MakeLiteral());
        Assert_NoAssume(std::get<FJson::FText>(*_builder.Peek()) == name.MakeLiteral());
        return true;
    }

    bool ToJson_(const FDirpath& path) {
        FileSystem::char_type wstringArr[FileSystem::MaxPathLength];
        _builder.Write(path.ToWCStr(wstringArr));
        Assert_NoAssume(std::get<FJson::FWText>(*_builder.Peek()) == MakeCStringView(wstringArr));
        return true;
    }
    bool ToJson_(const FFilename& path) {
        if (not path.empty()) {
            _builder.BeginArray();
            DEFERRED{ _builder.EndArray(); };

            ToJson_(path.Dirpath());

            FileSystem::char_type wstringArr[FileSystem::MaxPathLength];
            _builder.Write(path.Basename().ToWCStr(wstringArr));
        }
        else {
            _builder.Write(FJson::FNull{});
        }
        return true;
    }

    bool ToJson_(const RTTI::FBinaryData& binary) {
        const size_t len = Base64EncodeSize(binary.MakeView());
        char* const data = static_cast<char*>(_doc.Heap().Allocate(sizeof(char) * (len+1)));

        Base64Encode(binary.MakeView(), { data, len });
        data[len] = '\0';

        _builder.Write(FJson::FText::MakeForeignText(data));
        return true;
    }

    bool ToJson_(const RTTI::FAny& any) {
        if (any) {
            _builder.BeginObject(2);
            DEFERRED{ _builder.EndObject(); };

            _builder.KeyValue(FJson::TypeId, any.Traits()->TypeName());

            _builder.BeginKeyValue(FJson::Inner);
            DEFERRED{ _builder.EndKeyValue(); };

            return Append(any.InnerAtom());
        }
        else {
            _builder.Write(FJson::FNull{});
            return true;
        }
    }

    bool ToJson_(const RTTI::PMetaObject& obj) {
        if (obj) {
            FJson::FText link{ForceInit};
            const auto it = _visiteds.find(obj.get());
            if (_visiteds.end() == it) {
                if (!(_flags & Inlined) && obj->RTTI_IsExported()) {
                    // import
                    link = TextFormat(_doc.Allocator(), "{0}", RTTI::FPathName::FromObject(*obj));
                }
                else {
                    return Append(obj.get());
                }
            }
            else {
                // internal
                link = it->second;
            }

            _builder.Object([&]() {
                _builder.KeyValue(FJson::Ref, std::move(link));
            }, 1);
        }
        else {
            _builder.Write(FJson::FNull{});
        }
        return true;
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

        if (const FJson::FObject* const pJsonObj = std::get_if<FJson::FObject>(&src)) {
            RTTI::PMetaObject rttiObj;

            if (Unlikely(not ParseObject(&rttiObj, *pJsonObj))) {
                PPE_LOG(Serialize, Error, "failed to parse json object for <{0}>", dst.Traits()->NamedTypeInfos());
                return false;
            }

            Assert(rttiObj);

            if (not MakeAtom(&rttiObj).PromoteMove(dst)) {
                PPE_LOG(Serialize, Error, "incompatible object <{0}> parsed from json for <{1}>", rttiObj->RTTI_Class()->Name(), dst.Traits()->NamedTypeInfos());
                return false;
            }
        }
        else {
            _values.push_back(std::addressof(src));

            if (Unlikely(not dst.Accept(this))) {
                PPE_LOG(Serialize, Error, "failed to parse json value for <{0}>", dst.Traits()->NamedTypeInfos());
                return false;
            }

            Verify(_values.pop_back_ReturnBack() == std::addressof(src));
        }

        return true;
    }

    bool ParseObject(RTTI::PMetaObject* outRttiObj, const FJson::FObject& jsonObj) {
        Assert_NoAssume(_values.empty());

        const auto className = XPathAs<FJson::FText>(jsonObj, FJson::Class);
        if (not className) {
            PPE_LOG(Serialize, Error, "missing meta class name");
            return false;
        }

        const auto objAnchor = XPathAs<FJson::FText>(jsonObj, FJson::Id);
        if (not objAnchor) {
            PPE_LOG(Serialize, Error, "missing meta object anchor");
            return false;
        }

        bool exported = false;
        Meta::TOptionalReference<const FJson::FText> objName{};
        if (const auto jsonExported = XPath(jsonObj, FJson::Export)) {
            objName = std::get_if<FJson::FText>(jsonExported->get());
            if (nullptr == objName) {
                PPE_LOG(Serialize, Error, "wrong type for meta object export");
                return false;
            }
            if (objName->empty()) {
                PPE_LOG(Serialize, Error, "empty meta object export");
                return false;
            }
            if (not RTTI::FName::IsValidToken(objName->MakeView())) {
                PPE_LOG(Serialize, Error, "invalid meta object export format");
                return false;
            }
            exported = true;
        }

        bool topObject = false;
        if (const auto jsonTopObject = XPath(jsonObj, FJson::TopObject)) {
            const FJson::FBool* const boolTopObject = std::get_if<FJson::FBool>(jsonTopObject->get());
            if (nullptr == boolTopObject) {
                PPE_LOG(Serialize, Error, "invalid meta object top object flag");
                return false;
            }
            topObject = *boolTopObject;
        }

        const RTTI::FMetaClass* const klass = _link.ResolveClass(
            RTTI::FMetaDatabaseReadable{},
            RTTI::FName{ className->MakeView() });

        if (nullptr == klass) {
            PPE_LOG(Serialize, Error, "unknown meta class <{0}>", className->MakeView());
            return false;
        }

        RTTI::PMetaObject rttiObj;
        Verify(klass->CreateInstance(rttiObj, true));

        if (topObject)
            _link.AddTopObject(rttiObj.get());

        Insert_AssertUnique(_visiteds, *objAnchor, rttiObj);

        for (const RTTI::FMetaProperty* prop : klass->AllProperties()) {
            const FJson::FText key = prop->Name().MakeLiteral();

            const auto propValue = XPath(jsonObj, key);
            if (not propValue)
                continue; // assume default value for missing properties

            Assert_NoAssume(_values.empty());
            _values.push_back(propValue->get());

            const bool valid = prop->Get(*rttiObj).Accept(this);

            Assert_NoAssume(1 == _values.size());
            Assert_NoAssume(propValue->get() == _values.front());
            Verify(_values.pop_back_ReturnBack() == propValue->get());

            if (not valid)
                return false;
        }

        if (exported && objName)
            rttiObj->RTTI_Export(RTTI::FName{ objName->MakeView() });

        if (outRttiObj)
            *outRttiObj = std::move(rttiObj);

        return true;
    }

    virtual bool Visit(const RTTI::ITupleTraits* tuple, void* data) override final {
        const FJson::FArray* const arrIFP = std::get_if<FJson::FArray>(&Head_());
        PPE_LOG_CHECK(Serialize, arrIFP);

        const size_t n = tuple->Arity();
        PPE_LOG_CHECK(Serialize, n == arrIFP->size());

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
        const FJson::FArray* const arrIFP = std::get_if<FJson::FArray>(&Head_());
        PPE_LOG_CHECK(Serialize, arrIFP);

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
        const FJson::FArray* const arrIFP = std::get_if<FJson::FArray>(&Head_());
        PPE_LOG_CHECK(Serialize, arrIFP);

        const size_t n = arrIFP->size();
        dico->Reserve(data, n);

        STACKLOCAL_ATOM(keyData, dico->KeyTraits());
        const RTTI::FAtom keyAtom = keyData.MakeAtom();

        for (const FJson::FValue& jsonIt : *arrIFP) {
            const FJson::FArray* pairIFP = std::get_if<FJson::FArray>(&jsonIt);
            PPE_LOG_CHECK(Serialize, pairIFP && pairIFP->size() == 2);

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

    template <typename T, Meta::TEnableIf< std::is_integral_v<T> && std::is_signed_v<T> >* = nullptr>
    bool ToRTTI_(const RTTI::IScalarTraits* , T& dst) {
        if (const Opaq::integer* pValue = std::get_if<Opaq::integer>(&Head_())) {
            dst = checked_cast<T>(*pValue);
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json signed integer: {}", Head_());
        return false;
    }

    template <typename T, Meta::TEnableIf< std::is_integral_v<T> && std::is_unsigned_v<T> >* = nullptr>
    bool ToRTTI_(const RTTI::IScalarTraits* , T& dst) {
        if (const Opaq::uinteger* pValue = std::get_if<Opaq::uinteger>(&Head_())) {
            dst = checked_cast<T>(*pValue);
            return true;
        }
        if (const Opaq::integer* pValue = std::get_if<Opaq::integer>(&Head_())) {
            dst = checked_cast<T>(*pValue);
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json unsigned integer: {}", Head_());
        return false;
    }

    template <typename T, Meta::TEnableIf< std::is_floating_point_v<T> >* = nullptr>
    bool ToRTTI_(const RTTI::IScalarTraits* , T& dst) {
        if (const Opaq::floating_point* pValue = std::get_if<Opaq::floating_point>(&Head_())) {
            dst = static_cast<T>(*pValue);
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json floating point: {}", Head_());
        return false;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , bool& dst) {
        if (const FJson::FBool* pValue = std::get_if<FJson::FBool>(&Head_())) {
            dst = *pValue;
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json boolean: {}", Head_());
        return false;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , FString& dst) {
        if (const FJson::FText* pValue = std::get_if<FJson::FText>(&Head_())) {
            dst = pValue->MakeView();
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json ansi string: {}", Head_());
        return false;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , FWString& dst) {
        if (const FJson::FWText* pValue = std::get_if<FJson::FWText>(&Head_())) {
            dst = pValue->MakeView();
            return true;
        }
        if (const FJson::FText* pValue = std::get_if<FJson::FText>(&Head_())) {
            dst = UTF_8_TO_WCHAR(pValue->MakeView());
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json wide string: {}", Head_());
        return false;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , RTTI::FName& dst) {
        if (const FJson::FText* pValue = std::get_if<FJson::FText>(&Head_())) {
            dst = RTTI::FName(pValue->MakeView());
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json named token: {}", Head_());
        return false;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , FDirpath& dst) {
        if (const FJson::FWText* pValue = std::get_if<FJson::FWText>(&Head_())) {
            dst = pValue->MakeView();
            return true;
        }
        if (const FJson::FText* pValue = std::get_if<FJson::FText>(&Head_())) {
            dst = UTF_8_TO_WCHAR(pValue->MakeView());
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json director path: {}", Head_());
        return false;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , FFilename& dst) {
        if (const FJson::FArray* pArray = std::get_if<FJson::FArray>(&Head_())) {
            PPE_LOG_CHECK(Serialize, pArray->size() == 2);

            if (const FJson::FWText* pDirectory = std::get_if<FJson::FWText>(&pArray->at(0))) {
                FDirpath directory(pDirectory->MakeView());
                if (const FJson::FWText* pBasename = std::get_if<FJson::FWText>(&pArray->at(1))) {
                    FBasename basename(pBasename->MakeView());
                    dst = FFilename{ std::move(directory), std::move(basename) };
                    return true;
                }
            }

            if (const FJson::FText* pDirectory = std::get_if<FJson::FText>(&pArray->at(0))) {
                FDirpath directory(UTF_8_TO_WCHAR(pDirectory->MakeView()));
                if (const FJson::FText* pBasename = std::get_if<FJson::FText>(&pArray->at(1))) {
                    FBasename basename(UTF_8_TO_WCHAR(pBasename->MakeView()));
                    dst = FFilename{ std::move(directory), std::move(basename) };
                    return true;
                }
            }
        }
        if (std::get_if<FJson::FNull>(&Head_())) {
            dst = FFilename{};
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json file path: {}", Head_());
        return false;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , RTTI::FBinaryData& dst) {
        if (const FJson::FText* pValue = std::get_if<FJson::FText>(&Head_())) {
            dst.Resize_DiscardData(Base64DecodeSize(pValue->MakeView()));
            PPE_LOG_CHECK(Serialize, Base64Decode(pValue->MakeView(), dst.MakeView()));
            return true;
        }
        PPE_LOG(Serialize, Error, "invalid json binary data: {}", Head_());
        return false;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* , RTTI::FAny& any) {
        if (Head_().Nil())
            return true;

        const FJson::FObject* const objIFP = std::get_if<FJson::FObject>(&Head_());
        if (nullptr == objIFP || objIFP->size() < 2)
            PPE_THROW_IT(FJsonSerializerException("expected a Json object"));

        const auto pTypeId = XPathAs<FJson::FText>(*objIFP, FJson::TypeId);
        if (not pTypeId)
            PPE_THROW_IT(FJsonSerializerException("Any value must have 'type_id' property"));

        const auto pInnerValue= XPath(*objIFP, FJson::Inner);
        if (not pInnerValue)
            PPE_THROW_IT(FJsonSerializerException("Any value must have 'inner' property"));

        {
            RTTI::FMetaDatabaseReadable db;
            if (const RTTI::PTypeTraits traits = db->TraitsIFP(pTypeId->MakeView()))
                any.Reset(traits);
            else
                PPE_THROW_IT(FJsonSerializerException("unknown traits"));
        }

        _values.push_back(*pInnerValue);
        const bool result = any.InnerAtom().Accept(this);
        Verify(_values.pop_back_ReturnBack() == pInnerValue->get());

        return result;
    }

    bool ToRTTI_(const RTTI::IScalarTraits* scalar, RTTI::PMetaObject& obj) {
        Assert_NoAssume(not obj);
        if (Head_().Nil())
            return true;

        const FJson::FObject* const span = std::get_if<FJson::FObject>(&Head_());
        if (nullptr == span)
            PPE_THROW_IT(FJsonSerializerException("expected a Json object"));
        if (span->empty())
            PPE_THROW_IT(FJsonSerializerException("expected a non empty Json object"));

        const auto ref = XPathAs<FJson::FText>(*span, FJson::Ref);
        if (nullptr == ref)
            PPE_THROW_IT(FJsonSerializerException("expected a Json $ref"));

        RTTI::FPathName path;

        if (ref->empty())
            PPE_THROW_IT(FJsonSerializerException("expected a non empty Json string"));
        if (not RTTI::FPathName::Parse(&path, ref->MakeView()))
            PPE_THROW_IT(FJsonSerializerException("malformed RTTI path name"));

        const auto it = _visiteds.find(*ref);
        if (_visiteds.end() == it) {
            // assuming this is an import
            obj.reset(_link.ResolveImport(
                RTTI::FMetaDatabaseReadable{},
                path, RTTI::PTypeTraits{ scalar }));
        }
        else {
            // retrieve from already parsed object
            obj = it->second;

            if (ref->MakeView().StartsWith('~'))
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
        VerifyRelease(toJson.Append(any.InnerAtom()));
    }
    else {
        dst->Root().Reset();
    }
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FAtom& atom, FJson* dst) {
    Assert(dst);

    FRTTI_to_Json_ toJson(*dst, FRTTI_to_Json_::Inlined);
    VerifyRelease(toJson.Append(atom));
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::FMetaObject& obj, FJson* dst) {
    Assert(dst);

    FRTTI_to_Json_ toJson(*dst, FRTTI_to_Json_::Inlined);
    VerifyRelease(toJson.Append(&obj));
}
//----------------------------------------------------------------------------
void RTTI_to_Json(const RTTI::PMetaObject& pobj, FJson* dst) {
    Assert(dst);

    if (pobj) {
        FRTTI_to_Json_ toJson(*dst, FRTTI_to_Json_::Inlined);
        RTTI_to_Json(*pobj, dst);
    }
    else
        dst->Root().Reset();
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

    FJson::FArray arr{ dst->Allocator() };
    Reserve(arr, objs.size());

    for (const RTTI::SMetaObject& ref : objs) {
        toJson.Reset(*Emplace_Back(arr));
        VerifyRelease(toJson.Append(ref.get()));
    }

    dst->Root() = std::move(arr);
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

    FJson::FArray arr{ dst->Allocator() };
    Reserve(arr, numObjs);

    for (const RTTI::SCMetaTransaction& tr : mnamespace) {
        for (const RTTI::SMetaObject& ref : tr->Linearized().LoadedRefs) {
            toJson.Reset(*Emplace_Back(arr));
            VerifyRelease(toJson.Append(ref.get()));
        }
    }
}
//----------------------------------------------------------------------------
// Json -> RTTI
//----------------------------------------------------------------------------
bool Json_to_RTTI(const FJson& src, FTransactionLinker* link) {
    Assert(link);

    const FJson::FArray* const arrIFP = std::get_if<FJson::FArray>(&src.Root());
    if (nullptr == arrIFP) {
        PPE_LOG(Serialize, Error, "top json entry should be an array");
        return false;
    }

    FJson_to_RTTI_ toRTTI(*link);

    for (const FJson::FValue& item : *arrIFP) {
        const FJson::FObject* const objIFP = std::get_if<FJson::FObject>(&item);
        if (not objIFP) {
            PPE_LOG(Serialize, Error, "all top entries should be json objects");
            return false;
        }

        if (not toRTTI.ParseObject(nullptr, *objIFP)) {
            PPE_LOG(Serialize, Error, "failed to parse json object");
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
