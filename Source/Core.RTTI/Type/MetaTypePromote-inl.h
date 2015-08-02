#pragma once

#include "Core.RTTI/Type/MetaTypePromote.h"

#include "Core/IO/Stream.h"
#include "Core/IO/String.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define METATYPE_NUMERIC_INT_PROMOTE(_From, _To) \
    template <> struct MetaTypePromote<_From, _To> : public std::unary_function<_From&&, _To> { \
        typedef std::true_type enabled; \
        _To operator ()(const _From& value) const { \
            return checked_cast<_To>(value); \
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
    template <> struct MetaTypePromote<_From, _To> : public std::unary_function<_From&&, _To> { \
        typedef std::true_type enabled; \
        _To operator ()(const _From& value) const { \
        return static_cast<_To>(value); \
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
template <> struct MetaTypePromote<String, WString> : public std::unary_function<const String&, WString> {
    typedef std::true_type enabled;
    WString operator ()(const String& value) const {
        return ToWString(value);
    }
};
//----------------------------------------------------------------------------
template <> struct MetaTypePromote<WString, String> : public std::unary_function<const WString&, String>{
    typedef std::true_type enabled;
    String operator ()(const WString& value) const {
        return ToString(value);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define METATYPE_STRINGIZE_PROMOTE_IMPL(_From, _Char) \
    template <> struct MetaTypePromote<_From, BasicString<_Char> > : public std::unary_function<_From&&, BasicString<_Char> > { \
        typedef std::true_type enabled; \
        BasicString<_Char> operator ()(const _From& rvalue) const { \
            _Char buffer[sizeof(size_t)<<3]; \
            { \
                BasicOCStrStream<_Char> oss(buffer); \
                oss << rvalue; \
            } \
            return buffer; \
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
