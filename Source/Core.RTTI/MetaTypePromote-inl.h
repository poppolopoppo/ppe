#pragma once

#include "Core.RTTI/MetaTypePromote.h"

#include "Core/IO/Stream.h"
#include "Core/IO/String.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define METATYPE_NUMERIC_INT_PROMOTE(_From, _To) \
    template <> struct TMetaTypePromote<_From, _To> {\
        typedef std::true_type enabled; \
        bool operator ()(_To* dst, const _From& value) const { \
            *dst = checked_cast<_To>(value); \
            return true; \
        } \
    }
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(int8_t,   int16_t);
METATYPE_NUMERIC_INT_PROMOTE(int8_t,   int32_t);
METATYPE_NUMERIC_INT_PROMOTE(int8_t,   int64_t);
METATYPE_NUMERIC_INT_PROMOTE(int8_t,   uint8_t);
METATYPE_NUMERIC_INT_PROMOTE(int8_t,   uint16_t);
METATYPE_NUMERIC_INT_PROMOTE(int8_t,   uint32_t);
METATYPE_NUMERIC_INT_PROMOTE(int8_t,   uint64_t);
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(int16_t,  int8_t);
METATYPE_NUMERIC_INT_PROMOTE(int16_t,  int16_t);
METATYPE_NUMERIC_INT_PROMOTE(int16_t,  int32_t);
METATYPE_NUMERIC_INT_PROMOTE(int16_t,  int64_t);
METATYPE_NUMERIC_INT_PROMOTE(int16_t,  uint8_t);
METATYPE_NUMERIC_INT_PROMOTE(int16_t,  uint16_t);
METATYPE_NUMERIC_INT_PROMOTE(int16_t,  uint32_t);
METATYPE_NUMERIC_INT_PROMOTE(int16_t,  uint64_t);
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(int32_t,  int8_t);
METATYPE_NUMERIC_INT_PROMOTE(int32_t,  int16_t);
METATYPE_NUMERIC_INT_PROMOTE(int32_t,  int32_t);
METATYPE_NUMERIC_INT_PROMOTE(int32_t,  int64_t);
METATYPE_NUMERIC_INT_PROMOTE(int32_t,  uint8_t);
METATYPE_NUMERIC_INT_PROMOTE(int32_t,  uint16_t);
METATYPE_NUMERIC_INT_PROMOTE(int32_t,  uint32_t);
METATYPE_NUMERIC_INT_PROMOTE(int32_t,  uint64_t);
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  int8_t);
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  int16_t);
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  int32_t);
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  int64_t);
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  uint8_t);
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  uint16_t);
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  uint32_t);
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  uint64_t);
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  float);
METATYPE_NUMERIC_INT_PROMOTE(int64_t,  double);
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(uint8_t,  int8_t);
METATYPE_NUMERIC_INT_PROMOTE(uint8_t,  int16_t);
METATYPE_NUMERIC_INT_PROMOTE(uint8_t,  int32_t);
METATYPE_NUMERIC_INT_PROMOTE(uint8_t,  int64_t);
METATYPE_NUMERIC_INT_PROMOTE(uint8_t,  uint16_t);
METATYPE_NUMERIC_INT_PROMOTE(uint8_t,  uint32_t);
METATYPE_NUMERIC_INT_PROMOTE(uint8_t,  uint64_t);
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(uint16_t, int8_t);
METATYPE_NUMERIC_INT_PROMOTE(uint16_t, int16_t);
METATYPE_NUMERIC_INT_PROMOTE(uint16_t, int32_t);
METATYPE_NUMERIC_INT_PROMOTE(uint16_t, int64_t);
METATYPE_NUMERIC_INT_PROMOTE(uint16_t, uint8_t);
METATYPE_NUMERIC_INT_PROMOTE(uint16_t, uint16_t);
METATYPE_NUMERIC_INT_PROMOTE(uint16_t, uint32_t);
METATYPE_NUMERIC_INT_PROMOTE(uint16_t, uint64_t);
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(uint32_t, int8_t);
METATYPE_NUMERIC_INT_PROMOTE(uint32_t, int16_t);
METATYPE_NUMERIC_INT_PROMOTE(uint32_t, int32_t);
METATYPE_NUMERIC_INT_PROMOTE(uint32_t, int64_t);
METATYPE_NUMERIC_INT_PROMOTE(uint32_t, uint8_t);
METATYPE_NUMERIC_INT_PROMOTE(uint32_t, uint16_t);
METATYPE_NUMERIC_INT_PROMOTE(uint32_t, uint32_t);
METATYPE_NUMERIC_INT_PROMOTE(uint32_t, uint64_t);
//----------------------------------------------------------------------------
METATYPE_NUMERIC_INT_PROMOTE(uint64_t, int8_t);
METATYPE_NUMERIC_INT_PROMOTE(uint64_t, int16_t);
METATYPE_NUMERIC_INT_PROMOTE(uint64_t, int32_t);
METATYPE_NUMERIC_INT_PROMOTE(uint64_t, int64_t);
METATYPE_NUMERIC_INT_PROMOTE(uint64_t, uint8_t);
METATYPE_NUMERIC_INT_PROMOTE(uint64_t, uint16_t);
METATYPE_NUMERIC_INT_PROMOTE(uint64_t, uint32_t);
METATYPE_NUMERIC_INT_PROMOTE(uint64_t, uint64_t);
//----------------------------------------------------------------------------
#undef METATYPE_NUMERIC_INT_PROMOTE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define METATYPE_NUMERIC_FP_PROMOTE(_From, _To) \
    template <> struct TMetaTypePromote<_From, _To> { \
        typedef std::true_type enabled; \
        bool operator ()(_To* dst, const _From& value) const { \
            *dst = static_cast<_To>(value); \
            return true; \
        } \
    }
//----------------------------------------------------------------------------
METATYPE_NUMERIC_FP_PROMOTE(float, double);
METATYPE_NUMERIC_FP_PROMOTE(double, float);
//----------------------------------------------------------------------------
#undef METATYPE_NUMERIC_FP_PROMOTE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <> struct TMetaTypePromote<FString, FWString> {
    typedef std::true_type enabled;
    bool operator ()(FWString* dst, const FString& value) const {
        *dst = ToWString(value);
        return true;
    }
};
//----------------------------------------------------------------------------
template <> struct TMetaTypePromote<FWString, FString> {
    typedef std::true_type enabled;
    bool operator ()(FString* dst, const FWString& value) const {
        *dst = ToString(value);
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V, size_t _Dim>
struct TMetaTypePromote<RTTI::TVector<U>, TScalarVector<V, _Dim> > {
    typedef typename TMetaTypePromote<U, V>::enabled enabled;
    bool operator ()(TScalarVector<V, _Dim>* dst, const RTTI::TVector<U>& value) const {
        if (value.size() != _Dim)
            return false;

        forrange(i, 0, _Dim)
            if (not TMetaTypePromote<U, V>()(&dst->_data[i], value[i]))
                return false;

        return true;
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TMetaTypePromote<RTTI::TVector<PMetaAtom>, TScalarVector<T, _Dim> > {
    typedef std::true_type enabled;
    bool operator ()(TScalarVector<T, _Dim>* dst, const RTTI::TVector<PMetaAtom>& value) const {
        if (value.size() != _Dim)
            return false;

        forrange(i, 0, _Dim) {
            if (nullptr == value[i] ||
                false == PromoteCopy(&dst->_data[i], value[i].get()))
                return false;
        }

        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V, size_t _Width, size_t _Height>
struct TMetaTypePromote<RTTI::TVector<U>, TScalarMatrix<V, _Width, _Height> >  {
    typedef typename TMetaTypePromote<U, V>::enabled enabled;
    bool operator ()(TScalarMatrix<V, _Width, _Height>* dst, const RTTI::TVector<U>& value) const {
        if (value.size() != _Width * _Height)
            return false;

        ScalarMatrixData<V, _Width, _Height>& data = dst->data();
        forrange(i, 0, _Width*_Height)
            if (not TMetaTypePromote<U, V>()(&data.raw[i], value[i]))
                return false;

        return true;
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
struct TMetaTypePromote<RTTI::TVector<PMetaAtom>, TScalarMatrix<T, _Width, _Height> > {
    typedef std::true_type enabled;
    bool operator ()(TScalarMatrix<T, _Width, _Height>* dst, const RTTI::TVector<PMetaAtom>& value) const {
        if (value.size() != _Width * _Height)
            return false;

        ScalarMatrixData<T, _Width, _Height>& data = dst->data();
        forrange(i, 0, _Width*_Height) {
            if (nullptr == value[i] ||
                false == PromoteCopy(&data.raw[i], value[i].get()))
                return false;
        }

        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <>
struct TMetaTypePromote<FString, FName> {
    typedef std::true_type enabled;
    bool operator ()(FName* dst, const FString& value) const {
        *dst = FName(value);
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <>
struct TMetaTypePromote<RTTI::TDictionary<PMetaAtom, PMetaAtom>, RTTI::FOpaqueData> {
    typedef std::true_type enabled;
    bool operator ()(RTTI::FOpaqueData* dst, const RTTI::TDictionary<PMetaAtom, PMetaAtom>& value) const {
        dst->reserve(value.size());

        for (const TPair<PMetaAtom, PMetaAtom>& it : value) {
            auto& d = dst->Vector().push_back_Default();

            if (false == PromoteCopy(&d.first, it.first.get()))
                return false;

            d.second = it.second;
        }

        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <>
struct TMetaTypePromote<FString, RTTI::FBinaryData> {
    typedef std::true_type enabled;
    bool operator ()(RTTI::FBinaryData* dst, const FString& value) const {
        dst->Resize_DiscardData(value.size());
        Assert(dst->SizeInBytes() == value.size());
        memcpy(dst->data(), value.c_str(), dst->SizeInBytes());
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define METATYPE_STRINGIZE_PROMOTE_IMPL(_From, _Char) \
    template <> struct TMetaTypePromote<_From, TBasicString<_Char> > { \
        typedef std::true_type enabled; \
        bool operator ()(TBasicString<_Char>* dst, const _From& rvalue) const { \
            _Char buffer[sizeof(size_t)<<3]; \
            TBasicOCStrStream<_Char> oss(buffer); \
            oss << rvalue; \
            *dst = oss.NullTerminatedStr(); \
            return true; \
        } \
    }
#define METATYPE_STRINGIZE_PROMOTE(_From) \
    METATYPE_STRINGIZE_PROMOTE_IMPL(_From, char); \
    METATYPE_STRINGIZE_PROMOTE_IMPL(_From, wchar_t)
//----------------------------------------------------------------------------
METATYPE_STRINGIZE_PROMOTE(bool);
METATYPE_STRINGIZE_PROMOTE(int8_t);
METATYPE_STRINGIZE_PROMOTE(int16_t);
METATYPE_STRINGIZE_PROMOTE(int32_t);
METATYPE_STRINGIZE_PROMOTE(int64_t);
METATYPE_STRINGIZE_PROMOTE(uint8_t);
METATYPE_STRINGIZE_PROMOTE(uint16_t);
METATYPE_STRINGIZE_PROMOTE(uint32_t);
METATYPE_STRINGIZE_PROMOTE(uint64_t);
METATYPE_STRINGIZE_PROMOTE(float);
METATYPE_STRINGIZE_PROMOTE(double);
//----------------------------------------------------------------------------
#undef METATYPE_STRINGIZE_PROMOTE
#undef METATYPE_STRINGIZE_PROMOTE_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool PromoteMove(FMetaAtom *dst, T *src) {
    static_assert(TMetaType<T>::Enabled, "T is not a valid rtti type");

    const FMetaTypeId dstTypeId = dst->TypeInfo().Id;
    constexpr FMetaTypeId srcTypeId = TMetaType<T>::TypeId;

    if (dstTypeId == srcTypeId) {
        dst->Cast<T>()->Wrapper() = std::move(*src);
        return true;
    }
    else if (dstTypeId == TMetaType<PMetaAtom>::TypeId) {
        dst->Cast<PMetaAtom>()->Wrapper() = MakeAtom(std::move(*src));
        return true;
    }
    else if (srcTypeId == TMetaType<PMetaAtom>::TypeId) {
        Assert((std::is_same<T, PMetaAtom>::value));
        const PMetaAtom& srcAtom = *reinterpret_cast<PMetaAtom*>(src);
        return (srcAtom ? PromoteMove(dst, srcAtom.get()) : false);
    }
    else {
        return PromoteMove(dst, src, TMetaType<T>::TypeId);
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(FMetaAtom *dst, const T *src) {
    static_assert(TMetaType<T>::Enabled, "T is not a valid rtti type");

    const FMetaTypeId dstTypeId = dst->TypeInfo().Id;
    constexpr FMetaTypeId srcTypeId = TMetaType<T>::TypeId;

    if (dstTypeId == srcTypeId) {
        dst->Cast<T>()->Wrapper() = *src;
        return true;
    }
    else if (dstTypeId == TMetaType<PMetaAtom>::TypeId) {
        dst->Cast<PMetaAtom>()->Wrapper() = MakeAtom(*src);
        return true;
    }
    else if (srcTypeId == TMetaType<PMetaAtom>::TypeId) {
        Assert((std::is_same<T, PMetaAtom>::value));
        const PMetaAtom& srcAtom = *reinterpret_cast<PMetaAtom*>(src);
        return (srcAtom ? PromoteCopy(dst, srcAtom.get()) : false);
    }
    else {
        return PromoteCopy(dst, src, TMetaType<T>::TypeId);
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteMove(T *dst, FMetaAtom *src) {
    static_assert(TMetaType<T>::Enabled, "T is not a valid rtti type");

    constexpr FMetaTypeId dstTypeId = TMetaType<T>::TypeId;
    const FMetaTypeId srcTypeId = src->TypeInfo().Id;

    if (dstTypeId == srcTypeId) {
        *dst = std::move(src->Cast<T>()->Wrapper());
        return true;
    }
    else if (dstTypeId == TMetaType<PMetaAtom>::TypeId) {
        Assert((std::is_same<T, PMetaAtom>::value));
        const PMetaAtom& dstAtom = *reinterpret_cast<PMetaAtom*>(dst);
        return (dstAtom ? PromoteMove(dstAtom.get(), src) : false);
    }
    else if (srcTypeId == TMetaType<PMetaAtom>::TypeId) {
        const PMetaAtom& srcAtom = src->Cast<PMetaAtom>()->Wrapper()
        return (srcAtom ? PromoteMove(dst, srcAtom.get()) : false);
    }
    else {
        return PromoteMove(TMetaType<T>::TypeId, dst, src);
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(T *dst, const FMetaAtom *src) {
    static_assert(TMetaType<T>::Enabled, "T is not a valid rtti type");

    constexpr FMetaTypeId dstTypeId = TMetaType<T>::TypeId;
    const FMetaTypeId srcTypeId = src->TypeInfo().Id;

    if (dstTypeId == srcTypeId) {
        *dst = src->Cast<T>()->Wrapper();
        return true;
    }
    else if (dstTypeId == TMetaType<PMetaAtom>::TypeId) {
        Assert((std::is_same<T, PMetaAtom>::value));
        const PMetaAtom& dstAtom = *reinterpret_cast<PMetaAtom*>(dst);
        return (dstAtom ? PromoteCopy(dstAtom.get(), src) : false);
    }
    else if (srcTypeId == TMetaType<PMetaAtom>::TypeId) {
        const PMetaAtom& srcAtom = src->Cast<PMetaAtom>()->Wrapper();
        return (srcAtom ? PromoteCopy(dst, srcAtom.get()) : false);
    }
    else {
        return PromoteCopy(TMetaType<T>::TypeId, dst, src);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
