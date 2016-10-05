#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/IO/String.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Equals(const FMetaObject& lhs, const FMetaObject& rhs);
//----------------------------------------------------------------------------
inline bool NotEquals(const FMetaObject& lhs, const FMetaObject& rhs) { return !Equals(lhs, rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DeepEquals(const FMetaObject& lhs, const FMetaObject& rhs);
//----------------------------------------------------------------------------
inline bool NotDeepEquals(const FMetaObject& lhs, const FMetaObject& rhs) { return !DeepEquals(lhs, rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const FMetaObject& object);
//----------------------------------------------------------------------------
u128 Fingerprint128(const FMetaObject& object);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(FMetaObject& dst, FMetaObject& src);
//----------------------------------------------------------------------------
void Copy(FMetaObject& dst, const FMetaObject& src);
//----------------------------------------------------------------------------
void Swap(FMetaObject& lhs, FMetaObject& rhs);
//----------------------------------------------------------------------------
FMetaObject *NewCopy(const FMetaObject& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DeepCopy(FMetaObject& dst, const FMetaObject& src);
//----------------------------------------------------------------------------
FMetaObject *NewDeepCopy(const FMetaObject& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const RTTI::FMetaObject& object);
FString ToString(const RTTI::PMetaObject& pobject);
FString ToString(const RTTI::PCMetaObject& pobject);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::FMetaObject& object) {
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
