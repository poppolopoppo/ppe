#include "stdafx.h"

#include "MetaTypePromote.h"

#include "MetaAtom.h"
#include "MetaObject.h"
#include "MetaProperty.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_BEGIN_FROM(_Src) \
    case MetaType<_Src>::TypeId: { \
        typedef _Src src_type; \
        switch (dstTypeId) {
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_END_FROM() \
        } \
    }; \
    break;
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_TO(_Dst) \
        case MetaType<_Dst>::TypeId: \
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
    METATYPE_PROMOTE_TO(String) \
    METATYPE_PROMOTE_TO(WString)
//----------------------------------------------------------------------------
template <template <typename, typename> class _Impl>
struct PromoteTest_ {
    template <typename... _Args>
    bool operator ()(const MetaTypeId srcTypeId, const MetaTypeId dstTypeId, _Args&&... args) const;
};
//----------------------------------------------------------------------------
template <template <typename, typename> class _Impl>
template <typename... _Args>
bool PromoteTest_<_Impl>::operator ()(const MetaTypeId srcTypeId, const MetaTypeId dstTypeId, _Args&&... args) const {
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

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<PMetaAtom>)
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

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<byte>)
        METATYPE_PROMOTE_TO(byte2)
        METATYPE_PROMOTE_TO(byte4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<ubyte>)
        METATYPE_PROMOTE_TO(ubyte2)
        METATYPE_PROMOTE_TO(ubyte4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<i16>)
        METATYPE_PROMOTE_TO(short2)
        METATYPE_PROMOTE_TO(short4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<u16>)
        METATYPE_PROMOTE_TO(ushort2)
        METATYPE_PROMOTE_TO(ushort4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<i32>)
        METATYPE_PROMOTE_TO(word2)
        METATYPE_PROMOTE_TO(word4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<u32>)
        METATYPE_PROMOTE_TO(uword2)
        METATYPE_PROMOTE_TO(uword4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<float>)
        METATYPE_PROMOTE_TO(float2)
        METATYPE_PROMOTE_TO(float3)
        METATYPE_PROMOTE_TO(float4)
        METATYPE_PROMOTE_TO(float2x2)
        METATYPE_PROMOTE_TO(float3x3)
        METATYPE_PROMOTE_TO(float4x3)
        METATYPE_PROMOTE_TO(float4x4)
        METATYPE_PROMOTE_END_FROM()

        METATYPE_PROMOTE_BEGIN_FROM(RTTI::Vector<i64>)
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

        METATYPE_PROMOTE_BEGIN_FROM(String)
        METATYPE_PROMOTE_TO(BinaryData)
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
struct PromoteMoveImpl_ {
    bool operator ()(MetaAtom *dst, MetaAtom *src) const {
        return PromoteMove(dst->Cast<_To>()->Wrapper(), std::move(src->Cast<_From>()->Wrapper()) );
    }

    bool operator ()(MetaAtom *dst, void *src, MetaTypeId srcTypeId) const {
        Assert(srcTypeId == MetaType<_From>::TypeId);
        return PromoteMove(dst->Cast<_To>()->Wrapper(), std::move(*reinterpret_cast<_From *>(src)));
    }

    bool operator ()(MetaTypeId dstTypeId, void *dst, MetaAtom *src) const {
        Assert(dstTypeId == MetaType<_To>::TypeId);
        return PromoteMove(*reinterpret_cast<_To *>(dst), std::move(src->Cast<_From>()->Wrapper()));
    }
};
//----------------------------------------------------------------------------
template <typename _From, typename _To>
struct PromoteCopyImpl_ {
    bool operator ()(MetaAtom *dst, const MetaAtom *src) const {
        return PromoteCopy(dst->Cast<_To>()->Wrapper(), src->Cast<_From>()->Wrapper());
    }

    bool operator ()(MetaAtom *dst, const void *src, MetaTypeId srcTypeId) const {
        Assert(srcTypeId == MetaType<_From>::TypeId);
        return PromoteCopy(dst->Cast<_To>()->Wrapper(), *reinterpret_cast<const _From *>(src));
    }

    bool operator ()(MetaTypeId dstTypeId, void *dst, const MetaAtom *src) const {
        Assert(dstTypeId == MetaType<_To>::TypeId);
        return PromoteCopy(*reinterpret_cast<_To *>(dst), src->Cast<_From>()->Wrapper());
    }
};
//----------------------------------------------------------------------------
static constexpr MetaTypeId AtomTypeId = MetaType<PMetaAtom>::TypeId;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(MetaAtom *dst, MetaAtom *src) {
    Assert(dst);
    Assert(src);

    const MetaTypeId dstTypeId = dst->TypeInfo().Id;
    const MetaTypeId srcTypeId = src->TypeInfo().Id;

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
        return PromoteTest_<PromoteMoveImpl_>()(srcTypeId, dstTypeId, dst, src);
    }
}
//----------------------------------------------------------------------------
bool PromoteCopy(MetaAtom *dst, const MetaAtom *src) {
    Assert(dst);
    Assert(src);

    const MetaTypeId dstTypeId = dst->TypeInfo().Id;
    const MetaTypeId srcTypeId = src->TypeInfo().Id;

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
        return PromoteTest_<PromoteCopyImpl_>()(srcTypeId, dstTypeId, dst, src);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(MetaAtom *dst, void *src, MetaTypeId srcTypeId) {
    Assert(dst);
    Assert(src);

    const MetaTypeId dstTypeId = dst->TypeInfo().Id;

    // must be handled before :
    Assert(srcTypeId != dstTypeId);
    Assert(srcTypeId != AtomTypeId);
    Assert(dstTypeId != AtomTypeId);

    return PromoteTest_<PromoteMoveImpl_>()(srcTypeId, dstTypeId, dst, src, srcTypeId);
}
//----------------------------------------------------------------------------
bool PromoteCopy(MetaAtom *dst, const void *src, MetaTypeId srcTypeId) {
    Assert(dst);
    Assert(src);

    const MetaTypeId dstTypeId = dst->TypeInfo().Id;

    // must be handled before :
    Assert(srcTypeId != dstTypeId);
    Assert(srcTypeId != AtomTypeId);
    Assert(dstTypeId != AtomTypeId);

    return PromoteTest_<PromoteCopyImpl_>()(srcTypeId, dstTypeId, dst, src, srcTypeId);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(MetaTypeId dstTypeId, void *dst, MetaAtom *src) {
    Assert(dst);
    Assert(src);

    const MetaTypeId srcTypeId = src->TypeInfo().Id;

    // must be handled before :
    Assert(srcTypeId != dstTypeId);
    Assert(srcTypeId != AtomTypeId);
    Assert(dstTypeId != AtomTypeId);

    return PromoteTest_<PromoteMoveImpl_>()(srcTypeId, dstTypeId, dstTypeId, dst, src);
}
//----------------------------------------------------------------------------
bool PromoteCopy(MetaTypeId dstTypeId, void *dst, const MetaAtom *src) {
    Assert(dst);
    Assert(src);

    const MetaTypeId srcTypeId = src->TypeInfo().Id;

    // must be handled before :
    Assert(srcTypeId != dstTypeId);
    Assert(srcTypeId != AtomTypeId);
    Assert(dstTypeId != AtomTypeId);

    return PromoteTest_<PromoteCopyImpl_>()(srcTypeId, dstTypeId, dstTypeId, dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
