#pragma once

#include "Core/Core.h"

#ifdef FINAL_RELEASE
#   define USE_CORE_DEBUGFUNCTION 0
#else
#   define USE_CORE_DEBUGFUNCTION 1
#endif

#if USE_CORE_DEBUGFUNCTION

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Tricking the compiler to not remove some function used in natvis
//----------------------------------------------------------------------------
namespace details {
struct FDebugFunction {
    bool Registered;
    void* const Func;
    FDebugFunction* Next;

    template <typename _Ret, typename... _Args>
    FDebugFunction(_Ret(*f)(_Args...))
        : Registered(false), Func(f) {
        FDebugFunction*& head = Head();
        Next = head;
        head = this;
    }

    static CORE_API FDebugFunction*& Head() {
        ONE_TIME_INITIALIZE(FDebugFunction*, GHead_, nullptr);
        return (GHead_);
    }

    static CORE_API void MarkAsUsed(bool enabled) {
        for (FDebugFunction* f = Head(); f; f = f->Next) {
            Assert(f->Registered != enabled);
            f->Registered = enabled;
        }
    }
};
} //!details
//----------------------------------------------------------------------------
#define DEBUG_FUNCTION_START() ::Core::details::FDebugFunction::MarkAsUsed(true)
#define DEBUG_FUNCTION_SHUTDOWN() ::Core::details::FDebugFunction::MarkAsUsed(false)
//----------------------------------------------------------------------------
#define DEBUG_FUNCTION(_API, _NAME, _RET, _ARGS, ...) \
    PRAGMA_DISABLE_OPTIMIZATION \
    _API NO_INLINE _RET _NAME _ARGS { __VA_ARGS__ } \
    namespace details { \
    static ::Core::details::FDebugFunction CONCAT(GDebugFunction_, _NAME){ &_NAME }; \
    } \
    PRAGMA_ENABLE_OPTIMIZATION
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#else

#define DEBUG_FUNCTION_START() NOOP()
#define DEBUG_FUNCTION_SHUTDOWN() NOOP()
#define DEBUG_FUNCTION(_API, _NAME, _RET, _ARGS, ...)

#endif //!USE_CORE_DEBUGFUNCTION