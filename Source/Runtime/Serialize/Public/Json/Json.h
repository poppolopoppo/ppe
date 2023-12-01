#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "Lexer/TextHeap.h"
#include "SerializeExceptions.h"

#include "RTTI_fwd.h"
#include "RTTI/Atom.h"

#include "Container/AssociativeVector.h"
#include "Container/SparseArray.h"
#include "IO/FileSystem_fwd.h"
#include "IO/TextWriter_fwd.h"

#include <variant>

namespace PPE {
class IBufferedStreamReader;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FJsonException : public PPE::Serialize::FSerializeException {
public:
    typedef FSerializeException parent_type;

    FJsonException(const char *what)
        : FJsonException(what, Lexer::FLocation()) {}

    FJsonException(const char *what, const Lexer::FLocation& site)
        : parent_type(what), _site(site) {}

    const Lexer::FLocation& Site() const { return _site; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_SERIALIZE_API virtual FTextWriter& Description(FTextWriter& oss) const override final;
#endif

private:
    Lexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FJson {
public:
    struct FValue;

    using FSlabHeap = SLABHEAP(Json);
    using FTextHeap = TTextHeap<false, ALLOCATOR(Json)>;

    struct FAllocator {
        FSlabHeap Heap;
        FTextHeap Text{ Heap };

        FAllocator() = default;
        PPE_SERIALIZE_API ~FAllocator();
    };

    using FHeapRef = TPtrRef<FAllocator>;

    using FNull = RTTI::PMetaObject;
    using FBool = bool;
    using FInteger = i64;
    using FFloat = double;
    using FText = FTextHeap::FText;
    using FArray = SPARSEARRAY_SLAB(Json, FValue);
    using FObject = ASSOCIATIVE_SPARSEARRAY_SLAB(Json, FText, FValue);

    using FVariant = std::variant<
        std::monostate,
        FNull, FBool, FInteger, FFloat, FText, FArray, FObject >;

    enum EType {
        Null = 0,
        Bool,
        Integer,
        Float,
        String,
        Array,
        Object,
    };

    template <typename T>
    static CONSTEXPR bool IsKnownType{
        std::is_same_v<T, FNull> ||
        std::is_same_v<T, FBool> ||
        std::is_same_v<T, FInteger> ||
        std::is_same_v<T, FFloat> ||
        std::is_same_v<T, FText> ||
        std::is_same_v<T, FArray> ||
        std::is_same_v<T, FObject>
        };

    struct FValue {
        FVariant Data;

        FValue() = default;

        FValue(const FValue&) = default;
        FValue& operator =(const FValue&) = default;

        FValue(FValue&&) = default;
        FValue& operator =(FValue&&) = default;

        explicit FValue(FJson& doc, EType type) { Construct(doc, type); }

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        FValue(T&& rvalue) NOEXCEPT : Data(std::move(rvalue)) {}
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        FValue(const T& value) NOEXCEPT : Data(value) {}

        //RTTI::FTypeId TypeId() const { return InnerAtom().TypeId(); }

        PPE_SERIALIZE_API void Reset() NOEXCEPT;
        bool Valid() const { return not std::holds_alternative<std::monostate>(Data); }

        PPE_SERIALIZE_API bool Equals(const FValue& other) const NOEXCEPT;

        friend bool operator ==(const FValue& lhs, const FValue& rhs) NOEXCEPT { return lhs.Equals(rhs); }
        friend bool operator !=(const FValue& lhs, const FValue& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

        PPE_SERIALIZE_API hash_t HashValue() const NOEXCEPT;

        friend hash_t hash_value(const FValue& v) NOEXCEPT { return v.HashValue(); }

        //RTTI::FAtom InnerAtom() const NOEXCEPT;

        //friend RTTI::FAtom MakeAtom(const FValue& v) NOEXCEPT { return v.InnerAtom(); }

        template <typename T, typename... _Args>
        Meta::TEnableIf<
            IsKnownType<T> &&
            Meta::has_constructor<T, _Args...>::value,
            T& > Construct(_Args&&... args) {
            Assign(T{ std::forward<_Args>(args)... });
            return Get<T>();
        }

        PPE_SERIALIZE_API void Construct(FJson& doc, EType type);

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        void Assign(T&& rvalue) { Data = std::move(rvalue); }
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        void Assign(const T& value) { Data = value; }

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        T& Get() { return std::get<T>(Data); }
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        const T& Get() const { return std::get<T>(Data); }

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        T* GetIFP() { return std::get_if<T>(&Data); }
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        const T* GetIFP() const { return std::get_if<T>(&Data); }

        FNull& ToNull() { return Get<FNull>(); }
        FBool& ToBool() { return Get<FBool>(); }
        FInteger& ToInteger() { return Get<FInteger>(); }
        FFloat& ToFloat() { return Get<FFloat>(); }
        FText& ToString() { return Get<FText>(); }
        FArray& ToArray() { return Get<FArray>(); }
        FObject& ToObject() { return Get<FObject>(); }

        const FNull& ToNull() const { return Get<FNull>(); }
        const FBool& ToBool() const { return Get<FBool>(); }
        const FInteger& ToInteger() const { return Get<FInteger>(); }
        const FFloat& ToFloat() const { return Get<FFloat>(); }
        const FText& ToString() const { return Get<FText>(); }
        const FArray& ToArray() const { return Get<FArray>(); }
        const FObject& ToObject() const { return Get<FObject>(); }

        FNull* AsNull() { return GetIFP<FNull>(); }
        FBool* AsBool() { return GetIFP<FBool>(); }
        FInteger* AsInteger() { return GetIFP<FInteger>(); }
        FFloat* AsFloat() { return GetIFP<FFloat>(); }
        FText* AsString() { return GetIFP<FText>(); }
        FArray* AsArray() { return GetIFP<FArray>(); }
        FObject* AsObject() { return GetIFP<FObject>(); }

        const FNull* AsNull() const { return GetIFP<FNull>(); }
        const FBool* AsBool() const { return GetIFP<FBool>(); }
        const FInteger* AsInteger() const { return GetIFP<FInteger>(); }
        const FFloat* AsFloat() const { return GetIFP<FFloat>(); }
        const FText* AsString() const { return GetIFP<FText>(); }
        const FArray* AsArray() const { return GetIFP<FArray>(); }
        const FObject* AsObject() const { return GetIFP<FObject>(); }
    };

    FValue MakeValue(EType type) NOEXCEPT {
        return FValue{ *this, type };
    }

    FText MakeText(const FStringView& str, bool mergeable = true) NOEXCEPT {
        return Text().MakeText(str, mergeable);
    }

    static CONSTEXPR FText LiteralText(const FStringView& str) NOEXCEPT {
        return FTextHeap::MakeStaticText(str);
    }

    static const FText Id;
    static const FText Ref;
    static const FText Class;
    static const FText Export;
    static const FText Inner;
    static const FText TopObject;
    static const FText TypeId;

    NODISCARD static bool IsReservedKeyword(const FJson::FText& str) NOEXCEPT;

public:
    explicit FJson(FHeapRef alloc) NOEXCEPT
    :   _alloc(alloc) {
        Assert_NoAssume(_alloc);
    }

    ~FJson();

    FJson(FJson&& ) = default;
    FJson& operator =(FJson&& ) = default;

    FValue& Root() { return _root; }
    const FValue& Root() const { return _root; }

    FSlabHeap& Heap() { return _alloc->Heap; }
    FTextHeap& Text() { return _alloc->Text; }

    const FSlabHeap& Heap() const { return _alloc->Heap; }
    const FTextHeap& Text() const { return _alloc->Text; }

    void Clear_ForgetMemory();
    void Clear_ReleaseMemory();

    void ToStream(FTextWriter& oss, bool minify = false) const;
    void ToStream(FWTextWriter& oss, bool minify = false) const;

    NODISCARD static bool Load(FJson* json, const FFilename& filename);
    NODISCARD static bool Load(FJson* json, const FFilename& filename, IBufferedStreamReader* input);
    NODISCARD static bool Load(FJson* json, const FWStringView& filename, IBufferedStreamReader* input);
    NODISCARD static bool Load(FJson* json, const FWStringView& filename, const FStringView& content);

    NODISCARD static bool Append(FJson* json, const FWStringView& filename, IBufferedStreamReader* input);
    NODISCARD static bool Append(FJson* json, const FWStringView& filename, const FStringView& content);

private:
    FHeapRef _alloc;
    FValue _root;
};
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const FJson& json) {
    json.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR Serialize::FJson::FText operator "" _json (const char* str, size_t len) {
    return Serialize::FJson::LiteralText( FStringView(str, len) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
