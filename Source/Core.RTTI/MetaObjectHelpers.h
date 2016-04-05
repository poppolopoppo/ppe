#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/IO/String.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Equals(const MetaObject& lhs, const MetaObject& rhs);
//----------------------------------------------------------------------------
inline bool NotEquals(const MetaObject& lhs, const MetaObject& rhs) { return !Equals(lhs, rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DeepEquals(const MetaObject& lhs, const MetaObject& rhs);
//----------------------------------------------------------------------------
inline bool NotDeepEquals(const MetaObject& lhs, const MetaObject& rhs) { return !DeepEquals(lhs, rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const MetaObject& object);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(MetaObject& dst, MetaObject& src);
//----------------------------------------------------------------------------
void Copy(MetaObject& dst, const MetaObject& src);
//----------------------------------------------------------------------------
void Swap(MetaObject& lhs, MetaObject& rhs);
//----------------------------------------------------------------------------
MetaObject *NewCopy(const MetaObject& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DeepCopy(MetaObject& dst, const MetaObject& src);
//----------------------------------------------------------------------------
MetaObject *NewDeepCopy(const MetaObject& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
String ToString(const RTTI::MetaObject& object);
String ToString(const RTTI::PMetaObject& pobject);
String ToString(const RTTI::PCMetaObject& pobject);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::MetaObject& object) {
    return oss << ToString(object);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::PMetaObject& pobject) {
    return oss << ToString(pobject);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::PCMetaObject& pobject) {
    return oss << ToString(pobject);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
