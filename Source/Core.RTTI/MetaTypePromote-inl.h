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
template <typename T, size_t _Width, size_t _Height>
struct MetaTypePromote<RTTI::Vector<PMetaAtom>, ScalarMatrix<T, _Width, _Height> > :
    public std::unary_function<const RTTI::Vector<PMetaAtom>&, ScalarMatrix<T, _Width, _Height> > {
    typedef std::true_type enabled;
    bool operator ()(ScalarMatrix<T, _Width, _Height>* dst, const RTTI::Vector<PMetaAtom>& value) const {
        if (value.size() != _Width * _Height)
            return false;

        forrange(i, 0, _Width*_Height) {
            if (nullptr == value[i] ||
                false == PromoteCopy(&dst->data().raw[i], value[i].get()))
                return false;
        }

        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define METATYPE_STRINGIZE_PROMOTE_IMPL(_From, _Char) \
    template <> struct MetaTypePromote<_From, DefaultString<_Char>::type > { \
        typedef std::true_type enabled; \
        bool operator ()(DefaultString<_Char>::type* dst, const _From& rvalue) const { \
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
} //!namespace RTTI
} //!namespace Core
