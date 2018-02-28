#include "stdafx.h"

#include "JsonSerializer.h"

#include "Json.h"

#include "Core.RTTI/Any.h"
#include "Core.RTTI/Atom.h"
#include "Core.RTTI/AtomVisitor.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaDatabase.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaTransaction.h"
#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashSet.h"
#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/IO/BufferedStream.h"
#include "Core/IO/Format.h"
#include "Core/IO/FS/ConstNames.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/StreamProvider.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/TextWriter.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarVector.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FJsonSerializer::FJsonSerializer(bool minify/* = true */) : _minify(minify) {}
//----------------------------------------------------------------------------
FJsonSerializer::~FJsonSerializer() {}
//----------------------------------------------------------------------------
void FJsonSerializer::Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName/* = nullptr */) {
    Assert(transaction);
    Assert(input);

    FJson json;

    UsingBufferedStream(input, [&json, sourceName](IBufferedStreamReader* buffered) {
        if (not FJson::Load(&json, FFilename(MakeCStringView(sourceName)), buffered))
            CORE_THROW_IT(FJsonSerializerException("failed to parse Json document"));
    });

    VECTOR_THREAD_LOCAL(Serialize, RTTI::PMetaObject) parsed;
    if (not Json_to_RTTI(parsed, json))
        CORE_THROW_IT(FJsonSerializerException("failed to convert Json to RTTI"));

    for (const RTTI::PMetaObject& obj : parsed)
        transaction->RegisterObject(obj.get());
}
//----------------------------------------------------------------------------
void FJsonSerializer::Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);

    FJson json;
    RTTI_to_Json(json, transaction->TopObjects(), transaction);

    UsingBufferedStream(output, [this, &json](IBufferedStreamWriter* buffered) {
        FTextWriter oss(buffered);
        json.ToStream(oss, _minify);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool Json_IsRTTIToken(const FStringView& str) { return (str.size() > 1 && '$' == str[0]); }
static FTextHeap::FText Json_MetaClass() { return FTextHeap::MakeStaticText("$RTTI_Class"); };
static FTextHeap::FText Json_Export()    { return FTextHeap::MakeStaticText("$RTTI_Export"); };
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
        const FJson::FString* strIFP = src.AsString();
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
        const FJson::FString* strIFP = src.AsString();
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
        const FJson::FString* strIFP = src.AsString();
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
        const FJson::FString* strIFP = src.AsString();
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
        const FJson::FString* strIFP = src.AsString();
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
        const FJson::FString* strIFP = src.AsString();
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
    FRTTI_to_Json_(FJson& doc, const RTTI::FMetaTransaction* outer)
        : _doc(doc)
        , _outer(outer)
    {}

    void PushHead(FJson::FValue& value) {
        Assert(_values.empty());
        _values.push_back(&value);
    }

    void PopHead(FJson::FValue& value) {
        UNUSED(value);
        Assert(1 == _values.size());
        Assert(&value == _values.front());
        _values.pop_back();
    }

    virtual bool Visit(const RTTI::IPairTraits* pair, const RTTI::FAtom& atom) override final {
        bool result = true;

        FJson::FArray& arr = Head_().SetType_AssumeNull(_doc, FJson::TypeArray{});
        arr.resize(2);

        _values.push_back(&arr.front());
        result &= pair->First(atom).Accept(this);
        _values.pop_back();

        if (not result)
            return false;

        _values.push_back(&arr.back());
        result &= pair->Second(atom).Accept(this);
        _values.pop_back();

        return result;
    }

    virtual bool Visit(const RTTI::IListTraits* list, const RTTI::FAtom& atom) override final {
        FJson::FArray& arr = Head_().SetType_AssumeNull(_doc, FJson::TypeArray{});
        arr.resize(list->Count(atom));

        FJson::FValue* pvalue = (&arr[0]);
        return list->ForEach(atom, [this, &pvalue](const RTTI::FAtom& item) {
            _values.push_back(pvalue);
            bool r = item.Accept(this);
            _values.pop_back();

            ++pvalue;
            return r;
        });
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, const RTTI::FAtom& atom) override {
        FJson::FArray& arr = Head_().SetType_AssumeNull(_doc, FJson::TypeArray{});
        arr.resize(dico->Count(atom));

        FJson::FValue* pvalue = (&arr[0]);
        return dico->ForEach(atom, [this, &pvalue](const RTTI::FAtom& key, const RTTI::FAtom& value) {
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
        virtual bool Visit(const RTTI::IScalarTraits* traits, T& value) override final { \
            ToJson_(value); \
            return true; \
        }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    FJson& _doc;
    const RTTI::FMetaTransaction* _outer;
    VECTOR(Serialize, FJson::FValue*) _values;

    FJson::FValue& Head_() const { return (*_values.back()); }

    template <typename T>
    void ToJson_(const T& value) {
        RTTI_to_Json_(_doc, Head_(), value);
    }

    void ToJson_(const RTTI::FAny& any) {
        if (any) {
            FJson::FArray& jsonArr = Head_().SetType_AssumeNull(_doc, FJson::TypeArray{});

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

    void ToJson_(const RTTI::PMetaObject& metaObject) {
        if (metaObject) {
            bool exported = false;
            if (metaObject->RTTI_IsExported()) {
                Assert(not metaObject->RTTI_Name().empty());
                if (_outer && metaObject->RTTI_Outer() != _outer) {
                    Head_().SetValue(_doc.MakeString(StringFormat("<{0}>", metaObject->RTTI_Name())));
                    return;
                }
                exported = true;
            }

            FJson::FObject& jsonObject = Head_().SetType_AssumeNull(_doc, FJson::TypeObject{});

            const RTTI::FMetaClass* metaClass = metaObject->RTTI_Class();

            jsonObject.Add(Json_MetaClass()).SetValue(
                _doc.MakeString(metaClass->Name().MakeView()));

            if (exported)
                jsonObject.Add(Json_Export()).SetValue(
                    _doc.MakeString(metaObject->RTTI_Name().MakeView()));

            for (const RTTI::FMetaProperty* prop : metaClass->AllProperties()) {
                RTTI::FAtom atom = prop->Get(*metaObject);

                if (atom.IsDefaultValue())
                    continue;

                _values.push_back(&jsonObject.Add(_doc.MakeString(prop->Name().MakeView())));
                atom.Accept(this);
                _values.pop_back();
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
    void PushHead(const FJson::FValue& value) {
        Assert(_values.empty());
        _values.push_back(&value);
    }

    void PopHead(const FJson::FValue& value) {
        UNUSED(value);
        Assert(1 == _values.size());
        Assert(&value == _values.front());
        _values.pop_back();
    }

    virtual bool Visit(const RTTI::IPairTraits* pair, const RTTI::FAtom& atom) override final {
        const FJson::FArray* arrIFP = Head_().AsArray();
        if (nullptr == arrIFP || 2 != arrIFP->size())
            return false;

        bool result = true;

        _values.push_back(&arrIFP->front());
        result &= pair->First(atom).Accept(this);
        _values.pop_back();

        if (not result)
            return false;

        _values.push_back(&arrIFP->back());
        result &= pair->Second(atom).Accept(this);
        _values.pop_back();

        return result;
    }

    virtual bool Visit(const RTTI::IListTraits* list, const RTTI::FAtom& atom) override final {
        const FJson::FArray* arrIFP = Head_().AsArray();
        if (nullptr == arrIFP)
            return false;

        const size_t n = arrIFP->size();
        list->Reserve(atom, n);

        forrange(i, 0, n) {
            _values.push_back(&(*arrIFP)[i]);
            bool r = list->AddDefault(atom).Accept(this);
            _values.pop_back();
            if (not r)
                return false;
        }

        return true;
    }

    virtual bool Visit(const RTTI::IDicoTraits* dico, const RTTI::FAtom& atom) override {
        const FJson::FArray* arrIFP = Head_().AsArray();
        if (nullptr == arrIFP)
            return false;

        const size_t n = arrIFP->size();
        dico->Reserve(atom, n);

        const RTTI::PTypeTraits keyTraits(dico->KeyTraits());
        RTTI::FAny key;
        const RTTI::FAtom keyAtom = key.Reset(keyTraits);

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

            RTTI::FAtom valueAtom = dico->AddDefault(atom, keyAtom);

            _values.push_back(&pairIFP->back());
            r &= valueAtom.Accept(this);
            _values.pop_back();

            if (not r)
                return false;
        }

        return true;
    }

#define DECL_ATOM_VIRTUAL_VISIT(_Name, T, _TypeId) \
        virtual bool Visit(const RTTI::IScalarTraits* traits, T& value) override final { \
            return ToRTTI_(value); \
        }
    FOREACH_RTTI_NATIVETYPES(DECL_ATOM_VIRTUAL_VISIT)
#undef DECL_ATOM_VIRTUAL_VISIT

private:
    VECTOR(Serialize, const FJson::FValue*) _values;

    const FJson::FValue& Head_() const { return (*_values.back()); }

    template <typename T>
    bool ToRTTI_(T& value) {
        return Json_to_RTTI_(value, Head_());
    }

    bool ToRTTI_(RTTI::FAny& any) {
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

    bool ToRTTI_(RTTI::PMetaObject& metaObject) {
        Assert(not metaObject);

        if (Head_().AsNull())
            return true;

        // external reference
        if (const FJson::FString* importIFP = Head_().AsString()) {
            if ('<' != importIFP->MakeView().front() || '>' != importIFP->MakeView().back())
                return false;

            const FStringView name = importIFP->MakeView().ShiftFront().ShiftBack();

            metaObject = RTTI::MetaDB().ObjectIFP(name);
            if (nullptr == metaObject)
                CORE_THROW_IT(FJsonSerializerException("failed to import RTTI object from database"));

            return true;
        }

        const FJson::FObject* objIFP = Head_().AsObject();
        if (nullptr == objIFP || objIFP->empty())
            return false;

        const auto metaClassIt = objIFP->find(Json_MetaClass());
        if (objIFP->end() == metaClassIt)
            return false;

        const FJson::FString* strIFP = metaClassIt->second.AsString();
        if (nullptr == strIFP || strIFP->empty())
            return false;

        const RTTI::FMetaClass* metaClassIFP = RTTI::MetaDB().ClassIFP(strIFP->MakeView());
        if (nullptr == metaClassIFP)
            return false;

        metaClassIFP->CreateInstance(metaObject);

        const auto exportIt = objIFP->find(Json_Export());
        if (exportIt != objIFP->end()) {
            const FJson::FString* nameIFP = exportIt->second.AsString();
            if (nameIFP == nullptr || nameIFP->empty())
                return false;

            metaObject->RTTI_Export(RTTI::FName(*nameIFP));
        }

        for (const auto& it : *objIFP) {
            if (Json_IsRTTIToken(it.first)) // skip special tokens (already parsed before)
                continue;

            const RTTI::FMetaProperty* propIFP = metaClassIFP->PropertyIFP(it.first.MakeView());
            if (nullptr == propIFP)
                return false;

            _values.push_back(&it.second);
            bool r = propIFP->Get(*metaObject).Accept(this);
            _values.pop_back();

            if (not r)
                return false;
        }

        return true;
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RTTI_to_Json(FJson& dst, const TMemoryView<const RTTI::PMetaObject>& src, const RTTI::FMetaTransaction* outer/* = nullptr */) {
    FJson::FArray& arr = dst.Root().SetType_AssumeNull(dst, FJson::TypeArray{});
    arr.resize(src.size());

    FRTTI_to_Json_ toJson(dst, outer);

    forrange(i, 0, src.size()) {
        FJson::FValue& item = arr[i];

        toJson.PushHead(item);
        if (not RTTI::MakeAtom(&src[i]).Accept(&toJson))
            AssertNotReached();
        toJson.PopHead(item);
    }
}
//----------------------------------------------------------------------------
bool Json_to_RTTI(VECTOR_THREAD_LOCAL(Serialize, RTTI::PMetaObject)& dst, const FJson& src) {
    const FJson::FArray* arrIFP = src.Root().AsArray();
    if (nullptr == arrIFP)
        return false;

    FJson_to_RTTI_ toRTTI;

    dst.resize(arrIFP->size());
    forrange(i, 0, arrIFP->size()) {
        const FJson::FValue& item = arrIFP->at(i);

        toRTTI.PushHead(item);
        bool r = RTTI::MakeAtom(&dst[i]).Accept(&toRTTI);
        toRTTI.PopHead(item);

        if (not r)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
