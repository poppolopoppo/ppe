#pragma once

#include "Core_fwd.h"

#include "Container/Resizable.h"
#include "Container/Token_fwd.h"
#include "IO/FileSystem_fwd.h"
#include "Memory/MemoryView.h"
#include "Meta/Enum.h"

namespace PPE {
template <typename T, typename _Allocator>
class TRawStorage;
template <typename T, bool _IsPod>
class TStack;
template <typename T, typename _Allocator>
class TVector;
class FUniqueBuffer;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, Meta::TEnableIf<std::is_integral_v<T> &&
	sizeof(T) >= sizeof(i16) && sizeof(T) <= sizeof(u64)>* = nullptr>
struct TCompactInt {
    T Data{};
	NODISCARD CONSTEXPR T& operator *() { return Data; }
	NODISCARD CONSTEXPR const T& operator *() const { return *Data; }
};
//----------------------------------------------------------------------------
class FArchiveState {
public:
	enum EFlags : u32 {
		AF_NONE         = 0,
		AF_LOADING      = 1<<0,
		AF_DETERMINISM  = 1<<1,
	};
	ENUM_FLAGS_FRIEND(EFlags);

	NODISCARD bool IsDeterministic() const { return (Flags & AF_DETERMINISM); }
	NODISCARD bool IsLoading() const { return (Flags & AF_LOADING); }
	NODISCARD bool IsSaving() const { return (not IsLoading()); }

	EFlags Flags{ AF_NONE };
};
//----------------------------------------------------------------------------
class FArchive : private FArchiveState {
public:
	virtual ~FArchive() = default;

	using FArchiveState::IsDeterministic;
	using FArchiveState::IsLoading;
	using FArchiveState::IsSaving;

	const FArchiveState& State() const { return (*this); }

	virtual void Flush() {}
	virtual void Close() {}

    virtual void Serialize(FRawMemory view) = 0;

    virtual void SerializeBool(bool* value) { Serialize(MakePodView(*value)); }

	virtual void SerializeChar(char* value) { Serialize(MakePodView(*value)); }
	virtual void SerializeChar(wchar_t* value) { SerializePod(*this, value); }

    virtual void SerializeInt(i16* value) { SerializePod(*this, value); }
    virtual void SerializeInt(i32* value) { SerializePod(*this, value); }
    virtual void SerializeInt(i64* value) { SerializePod(*this, value); }
    virtual void SerializeInt(u16* value) { SerializePod(*this, value); }
    virtual void SerializeInt(u32* value) { SerializePod(*this, value); }
    virtual void SerializeInt(u64* value) { SerializePod(*this, value); }

    virtual void SerializeFloat(float* value)  { SerializePod(*this, value); }
    virtual void SerializeFloat(double* value) { SerializePod(*this, value); }

    PPE_CORE_API virtual void SerializeCompactInt(TCompactInt<i16>* value);
    PPE_CORE_API virtual void SerializeCompactInt(TCompactInt<i32>* value);
    PPE_CORE_API virtual void SerializeCompactInt(TCompactInt<i64>* value);
    PPE_CORE_API virtual void SerializeCompactInt(TCompactInt<u16>* value);
    PPE_CORE_API virtual void SerializeCompactInt(TCompactInt<u32>* value);
    PPE_CORE_API virtual void SerializeCompactInt(TCompactInt<u64>* value);

    PPE_CORE_API virtual void SerializeString(FString* str);
    PPE_CORE_API virtual void SerializeString(FWString* str);

	PPE_CORE_API virtual void SerializeFilesystem(FBasenameNoExt* value);
	PPE_CORE_API virtual void SerializeFilesystem(FBasename* value);
	PPE_CORE_API virtual void SerializeFilesystem(FDirpath* value);
	PPE_CORE_API virtual void SerializeFilesystem(FExtname* value);
	PPE_CORE_API virtual void SerializeFilesystem(FFilename* value);

    PPE_CORE_API virtual void SerializeBuffer(FUniqueBuffer* buffer);
    PPE_CORE_API virtual void SerializeCompressed(FUniqueBuffer* buffer);

    template <typename T, Meta::TEnableIf<Meta::is_pod_v<T>>* = nullptr>
    friend void SerializePod(FArchive& ar, T* value) { ar.Serialize(MakePodView(*value)); }
    template <typename T>
    friend void SerializeCompactSigned(FArchive& ar, TCompactInt<T>* value);
    template <typename T>
    friend void SerializeCompactUnsigned(FArchive& ar, TCompactInt<T>* value);
	template <typename T>
	friend void SerializeResizableArray(FArchive& ar, TResizable<T> array);
	template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
	friend void SerializeToken(FArchive& ar, TToken<_Tag, _Char, _Sensitive, _TokenTraits>* token);

protected:
	FArchiveState& State() { return (*this); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FORCE_INLINE FArchive& operator <<(FArchive& ar, TRawStorage<T, _Allocator>* storage) {
	SerializeResizableArray(ar, MakeResizable(storage));
	return ar;
}
//----------------------------------------------------------------------------
template <typename T, bool _bIsPOd>
FORCE_INLINE FArchive& operator <<(FArchive& ar, TStack<T, _bIsPOd>* stack) {
	SerializeResizableArray(ar, MakeResizable(stack));
	return ar;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FORCE_INLINE FArchive& operator <<(FArchive& ar, TVector<T, _Allocator>* vector) {
	SerializeResizableArray(ar, MakeResizable(vector));
	return ar;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
FORCE_INLINE FArchive& operator <<(FArchive& ar, TToken<_Tag, _Char, _Sensitive, _TokenTraits>* token) {
	SerializeToken(ar, token);
	return ar;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define PPE_ARCHIVE_SERIALIZE_OPERATOR(_Type, _Func) \
    FORCE_INLINE FArchive& operator <<(FArchive& ar, _Type value) { \
        ar._Func(value); \
        return ar; \
    }
//----------------------------------------------------------------------------
PPE_ARCHIVE_SERIALIZE_OPERATOR(FRawMemory, Serialize)
//----------------------------------------------------------------------------
PPE_ARCHIVE_SERIALIZE_OPERATOR(bool*, SerializeBool)
//----------------------------------------------------------------------------
PPE_ARCHIVE_SERIALIZE_OPERATOR(char*, SerializeChar)
PPE_ARCHIVE_SERIALIZE_OPERATOR(wchar_t*, SerializeChar)
//----------------------------------------------------------------------------
PPE_ARCHIVE_SERIALIZE_OPERATOR(i16*, SerializeInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(i32*, SerializeInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(i64*, SerializeInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(u16*, SerializeInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(u32*, SerializeInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(u64*, SerializeInt)
//----------------------------------------------------------------------------
PPE_ARCHIVE_SERIALIZE_OPERATOR(float*, SerializeFloat)
PPE_ARCHIVE_SERIALIZE_OPERATOR(double*, SerializeFloat)
//----------------------------------------------------------------------------
PPE_ARCHIVE_SERIALIZE_OPERATOR(TCompactInt<i16>*, SerializeCompactInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(TCompactInt<i32>*, SerializeCompactInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(TCompactInt<i64>*, SerializeCompactInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(TCompactInt<u16>*, SerializeCompactInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(TCompactInt<u32>*, SerializeCompactInt)
PPE_ARCHIVE_SERIALIZE_OPERATOR(TCompactInt<u64>*, SerializeCompactInt)
//----------------------------------------------------------------------------
PPE_ARCHIVE_SERIALIZE_OPERATOR(FString*, SerializeString)
PPE_ARCHIVE_SERIALIZE_OPERATOR(FWString*, SerializeString)
//----------------------------------------------------------------------------
PPE_ARCHIVE_SERIALIZE_OPERATOR(FUniqueBuffer*, SerializeBuffer)
//----------------------------------------------------------------------------
#undef PPE_ARCHIVE_SERIALIZE_OPERATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
