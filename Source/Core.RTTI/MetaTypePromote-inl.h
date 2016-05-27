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
    template <> struct MetaTypePromote<_From, _To> {\
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
    template <> struct MetaTypePromote<_From, _To> { \
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
template <> struct MetaTypePromote<String, WString> {
    typedef std::true_type enabled;
    bool operator ()(WString* dst, const String& value) const {
        *dst = ToWString(value);
        return true;
    }
};
//----------------------------------------------------------------------------
template <> struct MetaTypePromote<WString, String> {
    typedef std::true_type enabled;
    bool operator ()(String* dst, const WString& value) const {
        *dst = ToString(value);
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename U, typename V, size_t _Dim>
struct MetaTypePromote<RTTI::Vector<U>, ScalarVector<V, _Dim> > {
    typedef typename MetaTypePromote<U, V>::enabled enabled;
    bool operator ()(ScalarVector<V, _Dim>* dst, const RTTI::Vector<U>& value) const {
        if (value.size() != _Dim)
            return false;

        forrange(i, 0, _Dim)
            if (not MetaTypePromote<U, V>()(&dst->_data[i], value[i]))
                return false;

        return true;
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct MetaTypePromote<RTTI::Vector<PMetaAtom>, ScalarVector<T, _Dim> > {
    typedef std::true_type enabled;
    bool operator ()(ScalarVector<T, _Dim>* dst, const RTTI::Vector<PMetaAtom>& value) const {
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
struct MetaTypePromote<RTTI::Vector<U>, ScalarMatrix<V, _Width, _Height> >  {
    typedef typename MetaTypePromote<U, V>::enabled enabled;
    bool operator ()(ScalarMatrix<V, _Width, _Height>* dst, const RTTI::Vector<U>& value) const {
        if (value.size() != _Width * _Height)
            return false;

        ScalarMatrixData<V, _Width, _Height>& data = dst->data();
        forrange(i, 0, _Width*_Height)
            if (not MetaTypePromote<U, V>()(&data.raw[i], value[i]))
                return false;

        return true;
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _Width, size_t _Height>
struct MetaTypePromote<RTTI::Vector<PMetaAtom>, ScalarMatrix<T, _Width, _Height> > {
    typedef std::true_type enabled;
    bool operator ()(ScalarMatrix<T, _Width, _Height>* dst, const RTTI::Vector<PMetaAtom>& value) const {
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
struct MetaTypePromote<String, RTTI::Name> {
    typedef std::true_type enabled;
    bool operator ()(RTTI::Name* dst, const String& value) const {
        *dst = MakeStringSlice(value);
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <>
struct MetaTypePromote<RTTI::Dictionary<PMetaAtom, PMetaAtom>, RTTI::OpaqueData> {
    typedef std::true_type enabled;
    bool operator ()(RTTI::OpaqueData* dst, const RTTI::Dictionary<PMetaAtom, PMetaAtom>& value) const {
        dst->reserve(value.size());

        for (const Pair<PMetaAtom, PMetaAtom>& it : value) {
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
struct MetaTypePromote<String, RTTI::BinaryData> {
    typedef std::true_type enabled;
    bool operator ()(RTTI::BinaryData* dst, const String& value) const {
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
    template <> struct MetaTypePromote<_From, BasicString<_Char> > { \
        typedef std::true_type enabled; \
        bool operator ()(BasicString<_Char>* dst, const _From& rvalue) const { \
            _Char buffer[sizeof(size_t)<<3]; \
            BasicOCStrStream<_Char> oss(buffer); \
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
bool PromoteMove(MetaAtom *dst, T *src) {
    static_assert(MetaType<T>::Enabled, "T is not a valid rtti type");

    const MetaTypeId dstTypeId = dst->TypeInfo().Id;
    constexpr MetaTypeId srcTypeId = MetaType<T>::TypeId;

    if (dstTypeId == srcTypeId) {
        dst->Cast<T>()->Wrapper() = std::move(*src);
        return true;
    }
    else if (dstTypeId == MetaType<PMetaAtom>::TypeId) {
        dst->Cast<PMetaAtom>()->Wrapper() = MakeAtom(std::move(*src));
        return true;
    }
    else if (srcTypeId == MetaType<PMetaAtom>::TypeId) {
        Assert((std::is_same<T, PMetaAtom>::value));
        const PMetaAtom& srcAtom = *reinterpret_cast<PMetaAtom*>(src);
        return (srcAtom ? PromoteMove(dst, srcAtom.get()) : false);
    }
    else {
        return PromoteMove(dst, src, MetaType<T>::TypeId);
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(MetaAtom *dst, const T *src) {
    static_assert(MetaType<T>::Enabled, "T is not a valid rtti type");

    const MetaTypeId dstTypeId = dst->TypeInfo().Id;
    constexpr MetaTypeId srcTypeId = MetaType<T>::TypeId;

    if (dstTypeId == srcTypeId) {
        dst->Cast<T>()->Wrapper() = *src;
        return true;
    }
    else if (dstTypeId == MetaType<PMetaAtom>::TypeId) {
        dst->Cast<PMetaAtom>()->Wrapper() = MakeAtom(*src);
        return true;
    }
    else if (srcTypeId == MetaType<PMetaAtom>::TypeId) {
        Assert((std::is_same<T, PMetaAtom>::value));
        const PMetaAtom& srcAtom = *reinterpret_cast<PMetaAtom*>(src);
        return (srcAtom ? PromoteCopy(dst, srcAtom.get()) : false);
    }
    else {
        return PromoteCopy(dst, src, MetaType<T>::TypeId);
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteMove(T *dst, MetaAtom *src) {
    static_assert(MetaType<T>::Enabled, "T is not a valid rtti type");

    constexpr MetaTypeId dstTypeId = MetaType<T>::TypeId;
    const MetaTypeId srcTypeId = src->TypeInfo().Id;

    if (dstTypeId == srcTypeId) {
        *dst = std::move(src->Cast<T>()->Wrapper());
        return true;
    }
    else if (dstTypeId == MetaType<PMetaAtom>::TypeId) {
        Assert((std::is_same<T, PMetaAtom>::value));
        const PMetaAtom& dstAtom = *reinterpret_cast<PMetaAtom*>(dst);
        return (dstAtom ? PromoteMove(dstAtom.get(), src) : false);
    }
    else if (srcTypeId == MetaType<PMetaAtom>::TypeId) {
        const PMetaAtom& srcAtom = src->Cast<PMetaAtom>()->Wrapper()
        return (srcAtom ? PromoteMove(dst, srcAtom.get()) : false);
    }
    else {
        return PromoteMove(MetaType<T>::TypeId, dst, src);
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(T *dst, const MetaAtom *src) {
    static_assert(MetaType<T>::Enabled, "T is not a valid rtti type");

    constexpr MetaTypeId dstTypeId = MetaType<T>::TypeId;
    const MetaTypeId srcTypeId = src->TypeInfo().Id;

    if (dstTypeId == srcTypeId) {
        *dst = src->Cast<T>()->Wrapper();
        return true;
    }
    else if (dstTypeId == MetaType<PMetaAtom>::TypeId) {
        Assert((std::is_same<T, PMetaAtom>::value));
        const PMetaAtom& dstAtom = *reinterpret_cast<PMetaAtom*>(dst);
        return (dstAtom ? PromoteCopy(dstAtom.get(), src) : false);
    }
    else if (srcTypeId == MetaType<PMetaAtom>::TypeId) {
        const PMetaAtom& srcAtom = src->Cast<PMetaAtom>()->Wrapper();
        return (srcAtom ? PromoteCopy(dst, srcAtom.get()) : false);
    }
    else {
        return PromoteCopy(MetaType<T>::TypeId, dst, src);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
