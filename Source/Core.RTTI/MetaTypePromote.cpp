#include "stdafx.h"

#include "MetaTypePromote.h"

#include "MetaAtom.h"
#include "MetaObject.h"
#include "MetaProperty.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
        case MetaType<_Dst>::TypeId: { \
            _Impl<src_type, _Dst>()(std::forward<_Args>(args)...); \
            return true; \
        }
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_TO_NUMERIC() \
    METATYPE_PROMOTE_TO(int8_t) \
    METATYPE_PROMOTE_TO(int16_t) \
    METATYPE_PROMOTE_TO(int32_t) \
    METATYPE_PROMOTE_TO(int64_t) \
    METATYPE_PROMOTE_TO(uint8_t) \
    METATYPE_PROMOTE_TO(uint16_t) \
    METATYPE_PROMOTE_TO(uint32_t) \
    METATYPE_PROMOTE_TO(uint64_t)
//----------------------------------------------------------------------------
#define METATYPE_PROMOTE_TO_STRING() \
    METATYPE_PROMOTE_TO(String) \
    METATYPE_PROMOTE_TO(WString)
//----------------------------------------------------------------------------
namespace {
    template <template <typename, typename> class _Impl>
    struct PromoteTest_ {
        template <typename... _Args>
        bool operator ()(const MetaTypeId srcTypeId, const MetaTypeId dstTypeId, _Args&&... args) {
            switch (srcTypeId)
            {
                METATYPE_PROMOTE_BEGIN_FROM(int8_t)
                METATYPE_PROMOTE_TO_NUMERIC()
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(int16_t)
                METATYPE_PROMOTE_TO_NUMERIC()
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(int32_t)
                METATYPE_PROMOTE_TO_NUMERIC()
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(int64_t)
                METATYPE_PROMOTE_TO_NUMERIC()
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(uint8_t)
                METATYPE_PROMOTE_TO_NUMERIC()
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(uint16_t)
                METATYPE_PROMOTE_TO_NUMERIC()
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(uint32_t)
                METATYPE_PROMOTE_TO_NUMERIC()
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(uint64_t)
                METATYPE_PROMOTE_TO_NUMERIC()
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(float)
                METATYPE_PROMOTE_TO(double)
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(double)
                METATYPE_PROMOTE_TO(float)
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()

                METATYPE_PROMOTE_BEGIN_FROM(bool)
                METATYPE_PROMOTE_TO_STRING()
                METATYPE_PROMOTE_END_FROM()
            }

            return false;
        }
    };
}
//----------------------------------------------------------------------------
#undef METATYPE_PROMOTE_TO_STRING
#undef METATYPE_PROMOTE_TO_NUMERIC
#undef METATYPE_PROMOTE_TO
#undef METATYPE_PROMOTE_END_FROM
#undef METATYPE_PROMOTE_BEGIN_FROM
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
    template <typename _From, typename _To>
    struct PromoteMoveImpl_ {
        void operator ()(MetaAtom *dst, MetaAtom *src) const {
            PromoteMove<_From, _To>(dst->Cast<_To>()->Wrapper(), std::move(src->Cast<_From>()->Wrapper()) );
        }

        void operator ()(MetaAtom *dst, void *src, MetaTypeId srcTypeId) const {
            Assert(srcTypeId == MetaType<_From>::TypeId);
            PromoteMove<_From, _To>(dst->Cast<_To>()->Wrapper(), std::move(*reinterpret_cast<_From *>(src)));
        }

        void operator ()(MetaTypeId dstTypeId, void *dst, MetaAtom *src) const {
            Assert(dstTypeId == MetaType<_To>::TypeId);
            PromoteMove<_From, _To>(*reinterpret_cast<_To *>(dst), std::move(src->Cast<_From>()->Wrapper()));
        }
    };

    template <typename _From, typename _To>
    struct PromoteCopyImpl_ {
        void operator ()(MetaAtom *dst, const MetaAtom *src) const {
            PromoteCopy<_From, _To>(dst->Cast<_To>()->Wrapper(), src->Cast<_From>()->Wrapper());
        }

        void operator ()(MetaAtom *dst, const void *src, MetaTypeId srcTypeId) const {
            Assert(srcTypeId == MetaType<_From>::TypeId);
            PromoteCopy<_From, _To>(dst->Cast<_To>()->Wrapper(), *reinterpret_cast<const _From *>(src));
        }

        void operator ()(MetaTypeId dstTypeId, void *dst, const MetaAtom *src) const {
            Assert(dstTypeId == MetaType<_To>::TypeId);
            PromoteCopy<_From, _To>(*reinterpret_cast<_To *>(dst), src->Cast<_From>()->Wrapper());
        }
    };
}
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

    return PromoteTest_<PromoteMoveImpl_>()(srcTypeId, dstTypeId, dst, src);
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

    return PromoteTest_<PromoteCopyImpl_>()(srcTypeId, dstTypeId, dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(MetaAtom *dst, void *src, MetaTypeId srcTypeId) {
    Assert(dst);
    Assert(src);

    const MetaTypeId dstTypeId = dst->TypeInfo().Id;

    return PromoteTest_<PromoteMoveImpl_>()(srcTypeId, dstTypeId, dst, src, srcTypeId);
}
//----------------------------------------------------------------------------
bool PromoteCopy(MetaAtom *dst, const void *src, MetaTypeId srcTypeId) {
    Assert(dst);
    Assert(src);

    const MetaTypeId dstTypeId = dst->TypeInfo().Id;

    return PromoteTest_<PromoteCopyImpl_>()(srcTypeId, dstTypeId, dst, src, srcTypeId);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(MetaTypeId dstTypeId, void *dst, MetaAtom *src) {
    Assert(dst);
    Assert(src);

    const MetaTypeId srcTypeId = src->TypeInfo().Id;

    return PromoteTest_<PromoteMoveImpl_>()(srcTypeId, dstTypeId, dstTypeId, dst, src);
}
//----------------------------------------------------------------------------
bool PromoteCopy(MetaTypeId dstTypeId, void *dst, const MetaAtom *src) {
    Assert(dst);
    Assert(src);

    const MetaTypeId srcTypeId = src->TypeInfo().Id;

    return PromoteTest_<PromoteCopyImpl_>()(srcTypeId, dstTypeId, dstTypeId, dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
