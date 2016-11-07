#include "stdafx.h"

#include "MetaTypePromote.h"

#include "MetaAtom.h"
#include "MetaObject.h"
#include "MetaProperty.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarMatrix.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_BEGIN_FROM(_Src) \
    case TMetaType<_Src>::TypeId: { \
        typedef _Src src_type; \
        switch (dstTypeId) {
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_END_FROM() \
        } \
    }; \
    break;
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_TO(_Dst) \
        case TMetaType<_Dst>::TypeId: \
            return _Impl<src_type, _Dst>()(std::forward<_Args>(args)...);
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_TO_INTEGRAL() \
    METATYPE_PROMOTE_TO(int8_t) \
    METATYPE_PROMOTE_TO(int16_t) \
    METATYPE_PROMOTE_TO(int32_t) \
    METATYPE_PROMOTE_TO(int64_t) \
    METATYPE_PROMOTE_TO(uint8_t) \
    METATYPE_PROMOTE_TO(uint16_t) \
    METATYPE_PROMOTE_TO(uint32_t) \
    METATYPE_PROMOTE_TO(uint64_t)
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_TO_FLOATING_POINT() \
    METATYPE_PROMOTE_TO(float) \
    METATYPE_PROMOTE_TO(double)
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_TO_STRING() \
    METATYPE_PROMOTE_TO(FString) \
    METATYPE_PROMOTE_TO(FWString)
//----------------------------------------------------------------------------
template <template <typename, typename> class _Impl>
struct TPromoteTest_ {
    template <typename... _Args>
    bool operator ()(const FMetaTypeId srcTypeId, const FMetaTypeId dstTypeId, _Args&&... args) const;
};
//----------------------------------------------------------------------------
template <template <typename, typename> class _Impl>
template <typename... _Args>
bool TPromoteTest_<_Impl>::operator ()(const FMetaTypeId srcTypeId, const FMetaTypeId dstTypeId, _Args&&... args) const {
    switch (srcTypeId)
    {
        METATYPE_PROMOTE_BEGIN_FROM(int8_t)
        METATYPE_PROMOTE_TO_INTEGRAL()
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(int16_t)
        METATYPE_PROMOTE_TO_INTEGRAL()
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(int32_t)
        METATYPE_PROMOTE_TO_INTEGRAL()
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(int64_t)
        METATYPE_PROMOTE_TO_INTEGRAL()
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_TO_FLOATING_POINT()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(uint8_t)
        METATYPE_PROMOTE_TO_INTEGRAL()
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(uint16_t)
        METATYPE_PROMOTE_TO_INTEGRAL()
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(uint32_t)
        METATYPE_PROMOTE_TO_INTEGRAL()
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(uint64_t)
        METATYPE_PROMOTE_TO_INTEGRAL()
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(float)
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(double)
        METATYPE_PROMOTE_TO(float)
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(bool)
        METATYPE_PROMOTE_TO_STRING()
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<PMetaAtom>)
        METATYPE_PROMOTE_TO(byte2)
        METATYPE_PROMOTE_TO(byte4)
        METATYPE_PROMOTE_TO(ubyte2)
        METATYPE_PROMOTE_TO(ubyte4)
        METATYPE_PROMOTE_TO(short2)
        METATYPE_PROMOTE_TO(short4)
        METATYPE_PROMOTE_TO(ushort2)
        METATYPE_PROMOTE_TO(ushort4)
        METATYPE_PROMOTE_TO(word2)
        METATYPE_PROMOTE_TO(word4)
        METATYPE_PROMOTE_TO(uword2)
        METATYPE_PROMOTE_TO(uword4)
        METATYPE_PROMOTE_TO(float2)
        METATYPE_PROMOTE_TO(float3)
        METATYPE_PROMOTE_TO(float4)
        METATYPE_PROMOTE_TO(float2x2)
        METATYPE_PROMOTE_TO(float3x3)
        METATYPE_PROMOTE_TO(float4x3)
        METATYPE_PROMOTE_TO(float4x4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<byte>)
        METATYPE_PROMOTE_TO(byte2)
        METATYPE_PROMOTE_TO(byte4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<ubyte>)
        METATYPE_PROMOTE_TO(ubyte2)
        METATYPE_PROMOTE_TO(ubyte4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<i16>)
        METATYPE_PROMOTE_TO(short2)
        METATYPE_PROMOTE_TO(short4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<u16>)
        METATYPE_PROMOTE_TO(ushort2)
        METATYPE_PROMOTE_TO(ushort4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<i32>)
        METATYPE_PROMOTE_TO(word2)
        METATYPE_PROMOTE_TO(word4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<u32>)
        METATYPE_PROMOTE_TO(uword2)
        METATYPE_PROMOTE_TO(uword4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<float>)
        METATYPE_PROMOTE_TO(float2)
        METATYPE_PROMOTE_TO(float3)
        METATYPE_PROMOTE_TO(float4)
        METATYPE_PROMOTE_TO(float2x2)
        METATYPE_PROMOTE_TO(float3x3)
        METATYPE_PROMOTE_TO(float4x3)
        METATYPE_PROMOTE_TO(float4x4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TVector<i64>)
        METATYPE_PROMOTE_TO(byte2)
        METATYPE_PROMOTE_TO(byte4)
        METATYPE_PROMOTE_TO(ubyte2)
        METATYPE_PROMOTE_TO(ubyte4)
        METATYPE_PROMOTE_TO(short2)
        METATYPE_PROMOTE_TO(short4)
        METATYPE_PROMOTE_TO(ushort2)
        METATYPE_PROMOTE_TO(ushort4)
        METATYPE_PROMOTE_TO(word2)
        METATYPE_PROMOTE_TO(word4)
        METATYPE_PROMOTE_TO(uword2)
        METATYPE_PROMOTE_TO(uword4)
        METATYPE_PROMOTE_TO(float2)
        METATYPE_PROMOTE_TO(float3)
        METATYPE_PROMOTE_TO(float4)
        METATYPE_PROMOTE_TO(float2x2)
        METATYPE_PROMOTE_TO(float3x3)
        METATYPE_PROMOTE_TO(float4x3)
        METATYPE_PROMOTE_TO(float4x4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(FString)
        METATYPE_PROMOTE_TO(FName)
        METATYPE_PROMOTE_TO(FBinaryData)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::TDictionary<PMetaAtom COMMA PMetaAtom>)
        METATYPE_PROMOTE_TO(FOpaqueData)
        METATYPE_PROMOTE_END_FROM()
    }

    return false;
}
//----------------------------------------------------------------------------
#undef METATYPE_PROMOTE_TO_STRING
#undef METATYPE_PROMOTE_TO_INTEGRAL
#undef METATYPE_PROMOTE_TO
#undef METATYPE_PROMOTE_END_FROM
#undef METATYPE_PROMOTE_BEGIN_FROM
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _From, typename _To>
struct FPromoteMoveImpl_ {
    bool operator ()(FMetaAtom *dst, FMetaAtom *src) const {
        return PromoteMove(dst->Cast<_To>()->Wrapper(), std::move(src->Cast<_From>()->Wrapper()) );
    }

    bool operator ()(FMetaAtom *dst, void *src, FMetaTypeId srcTypeId) const {
        UNUSED(srcTypeId);
        Assert(srcTypeId == TMetaType<_From>::TypeId);
        return PromoteMove(dst->Cast<_To>()->Wrapper(), std::move(*reinterpret_cast<_From *>(src)));
    }

    bool operator ()(FMetaTypeId dstTypeId, void *dst, FMetaAtom *src) const {
        UNUSED(dstTypeId);
        Assert(dstTypeId == TMetaType<_To>::TypeId);
        return PromoteMove(*reinterpret_cast<_To *>(dst), std::move(src->Cast<_From>()->Wrapper()));
    }
};
//----------------------------------------------------------------------------
template <typename _From, typename _To>
struct FPromoteCopyImpl_ {
    bool operator ()(FMetaAtom *dst, const FMetaAtom *src) const {
        return PromoteCopy(dst->Cast<_To>()->Wrapper(), src->Cast<_From>()->Wrapper());
    }

    bool operator ()(FMetaAtom *dst, const void *src, FMetaTypeId srcTypeId) const {
        UNUSED(srcTypeId);
        Assert(srcTypeId == TMetaType<_From>::TypeId);
        return PromoteCopy(dst->Cast<_To>()->Wrapper(), *reinterpret_cast<const _From *>(src));
    }

    bool operator ()(FMetaTypeId dstTypeId, void *dst, const FMetaAtom *src) const {
        UNUSED(dstTypeId);
        Assert(dstTypeId == TMetaType<_To>::TypeId);
        return PromoteCopy(*reinterpret_cast<_To *>(dst), src->Cast<_From>()->Wrapper());
    }
};
//----------------------------------------------------------------------------
static constexpr FMetaTypeId AtomTypeId = TMetaType<PMetaAtom>::TypeId;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(FMetaAtom *dst, FMetaAtom *src) {
    Assert(dst);
    Assert(src);

    const FMetaTypeId dstTypeId = dst->TypeInfo().Id;
    const FMetaTypeId srcTypeId = src->TypeInfo().Id;

    if (srcTypeId == dstTypeId) {
        dst->MoveFrom(src);
        return true;
    }
    else if (dstTypeId == AtomTypeId) {
        PMetaAtom& dstAtom = dst->Cast<PMetaAtom>()->Wrapper();
        dstAtom = src->WrapMoveTo();
        return true;
    }
    else if (srcTypeId == AtomTypeId) {
        const PMetaAtom& srcAtom = src->Cast<PMetaAtom>()->Wrapper();
        return (srcAtom ? PromoteMove(dst, srcAtom.get()) : false);
    }
    else {
        return TPromoteTest_<FPromoteMoveImpl_>()(srcTypeId, dstTypeId, dst, src);
    }
}
//----------------------------------------------------------------------------
bool PromoteCopy(FMetaAtom *dst, const FMetaAtom *src) {
    Assert(dst);
    Assert(src);

    const FMetaTypeId dstTypeId = dst->TypeInfo().Id;
    const FMetaTypeId srcTypeId = src->TypeInfo().Id;

    if (srcTypeId == dstTypeId) {
        dst->CopyFrom(src);
        return true;
    }
    else if (dstTypeId == AtomTypeId) {
        PMetaAtom& dstAtom = dst->Cast<PMetaAtom>()->Wrapper();
        dstAtom = src->WrapCopyTo();
        return true;
    }
    else if (srcTypeId == AtomTypeId) {
        const PMetaAtom& srcAtom = src->Cast<PMetaAtom>()->Wrapper();
        return (srcAtom ? PromoteCopy(dst, srcAtom.get()) : false);
    }
    else {
        return TPromoteTest_<FPromoteCopyImpl_>()(srcTypeId, dstTypeId, dst, src);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(FMetaAtom *dst, void *src, FMetaTypeId srcTypeId) {
    Assert(dst);
    Assert(src);

    const FMetaTypeId dstTypeId = dst->TypeInfo().Id;

    // must be handled before :
    Assert(srcTypeId != dstTypeId);
    Assert(srcTypeId != AtomTypeId);
    Assert(dstTypeId != AtomTypeId);

    return TPromoteTest_<FPromoteMoveImpl_>()(srcTypeId, dstTypeId, dst, src, srcTypeId);
}
//----------------------------------------------------------------------------
bool PromoteCopy(FMetaAtom *dst, const void *src, FMetaTypeId srcTypeId) {
    Assert(dst);
    Assert(src);

    const FMetaTypeId dstTypeId = dst->TypeInfo().Id;

    // must be handled before :
    Assert(srcTypeId != dstTypeId);
    Assert(srcTypeId != AtomTypeId);
    Assert(dstTypeId != AtomTypeId);

    return TPromoteTest_<FPromoteCopyImpl_>()(srcTypeId, dstTypeId, dst, src, srcTypeId);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(FMetaTypeId dstTypeId, void *dst, FMetaAtom *src) {
    Assert(dst);
    Assert(src);

    const FMetaTypeId srcTypeId = src->TypeInfo().Id;

    // must be handled before :
    Assert(srcTypeId != dstTypeId);
    Assert(srcTypeId != AtomTypeId);
    Assert(dstTypeId != AtomTypeId);

    return TPromoteTest_<FPromoteMoveImpl_>()(srcTypeId, dstTypeId, dstTypeId, dst, src);
}
//----------------------------------------------------------------------------
bool PromoteCopy(FMetaTypeId dstTypeId, void *dst, const FMetaAtom *src) {
    Assert(dst);
    Assert(src);

    const FMetaTypeId srcTypeId = src->TypeInfo().Id;

    // must be handled before :
    Assert(srcTypeId != dstTypeId);
    Assert(srcTypeId != AtomTypeId);
    Assert(dstTypeId != AtomTypeId);

    return TPromoteTest_<FPromoteCopyImpl_>()(srcTypeId, dstTypeId, dstTypeId, dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
