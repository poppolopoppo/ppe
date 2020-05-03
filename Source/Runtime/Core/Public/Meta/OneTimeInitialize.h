#pragma once

#include "Core_fwd.h"

/*
//
// ONE_TIME_INITIALIZE()
// ---------------------
// Enables thread safe static variable initialization.
// Beware ! The destructor will not be automatically called !
//
*/

#if defined(_MSC_VER)
#   if _MSC_VER >= 1900 // Visual supports "Magic Statics" only since 2015
//  https://blogs.msdn.microsoft.com/vcblog/2014/11/17/c111417-features-in-vs-2015-preview/
#   define PPE_HAS_MAGIC_STATICS
#   endif
#elif defined(__GNUC__) || defined(__CLANG__)
//  GCC and CLANG have full support of "Magic Statics"
#   define PPE_HAS_MAGIC_STATICS
#else
#   error "unsupported platform"
#endif


#ifdef PPE_HAS_MAGIC_STATICS

#   define _ONE_TIME_INITIALIZE_IMPL_(_Type, _Name, _Initializer, _ThreadLocal, _TypenameIfInTemplate) \
    static _ThreadLocal _Type _Name _Initializer

#else

#   include "Misc/TargetPlatform.h"
#   define PPE_INTERLOCKEDCOMPAREEXCHANGE_32(_Var, _Src, _Operand) PPE::FPlatformAtomics::CompareExchange((volatile long*)(&(_Var)), static_cast<long>(_Src), static_cast<long>(_Operand))
#   define PPE_INTERLOCKEDEXCHANGE_PTR(_Var, _Value) PPE::FPlatformAtomics::Exchange((void* volatile*)(&(_Var)), _Value)
#   define PPE_INTERLOCKEDEXCHANGE_32(_Var, _Value) PPE::FPlatformAtomics::Exchange((volatile long*)(&(_Var)), static_cast<long>(_Value))
#   define PPE_SHORTSYNCWAIT() PPE::FPlatformAtomics::ShortSyncWait()

#   include "Meta/AlignedStorage.h"

#   define _ONE_TIME_INITIALIZE_INIT_INVALID   0
#   define _ONE_TIME_INITIALIZE_INIT_CREATING  1
#   define _ONE_TIME_INITIALIZE_INIT_READY     2

#   define _ONE_TIME_INITIALIZE_IMPL_(_Type, _Name, _Initializer, _ThreadLocal, _TypenameIfInTemplate) \
    static _ThreadLocal _TypenameIfInTemplate POD_STORAGE(COMMA_PROTECT(_Type)) _Name##_OneTimeStorage; \
    static _ThreadLocal _Type* volatile     _Name##_OneTimePData = nullptr; \
    static _ThreadLocal volatile u32        _Name##_OneTimeState = _ONE_TIME_INITIALIZE_INIT_INVALID;\
    \
    if(PPE_INTERLOCKEDCOMPAREEXCHANGE_32(_Name##_OneTimeState, 0, 0) != _ONE_TIME_INITIALIZE_INIT_READY) { \
        if (PPE_INTERLOCKEDCOMPAREEXCHANGE_32(_Name##_OneTimeState, _ONE_TIME_INITIALIZE_INIT_CREATING, _ONE_TIME_INITIALIZE_INIT_INVALID) == _ONE_TIME_INITIALIZE_INIT_INVALID) { \
            auto *const _onetime_pdata = new ((void *)(&_Name##_OneTimeStorage)) _TypenameIfInTemplate TRemoveConst<_Type> _Initializer; \
            PPE_INTERLOCKEDEXCHANGE_PTR(_Name##_OneTimePData, _onetime_pdata); \
            PPE_INTERLOCKEDEXCHANGE_32(_Name##_OneTimeState, _ONE_TIME_INITIALIZE_INIT_READY); \
        } \
        else { \
            while(PPE_INTERLOCKEDCOMPAREEXCHANGE_32(_Name##_OneTimeState, 0, 0) != _ONE_TIME_INITIALIZE_INIT_READY) \
                PPE_SHORTSYNCWAIT(); \
        } \
    } \
    \
    _Type& _Name = *(_Name##_OneTimePData); \
    UNUSED(_Name##_OneTimePData)

#endif //!PPE_HAS_MAGIC_STATICS

#define ONE_TIME_INITIALIZE(_Type, _Name, ...) \
    _ONE_TIME_INITIALIZE_IMPL_(COMMA_PROTECT(_Type), _Name, {COMMA_PROTECT(__VA_ARGS__)}, , )
#define ONE_TIME_DEFAULT_INITIALIZE(_Type, _Name) \
    _ONE_TIME_INITIALIZE_IMPL_(COMMA_PROTECT(_Type), _Name, {}, , )

#define ONE_TIME_INITIALIZE_THREAD_LOCAL(_Type, _Name, ...) \
    _ONE_TIME_INITIALIZE_IMPL_(COMMA_PROTECT(_Type), _Name, {COMMA_PROTECT(__VA_ARGS__)}, THREAD_LOCAL, )
#define ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(_Type, _Name) \
    _ONE_TIME_INITIALIZE_IMPL_(COMMA_PROTECT(_Type), _Name, {}, THREAD_LOCAL, )

#define ONE_TIME_INITIALIZE_TPL(_Type, _Name, ...) \
    _ONE_TIME_INITIALIZE_IMPL_(COMMA_PROTECT(_Type), _Name, {COMMA_PROTECT(__VA_ARGS__)}, , typename )
#define ONE_TIME_DEFAULT_INITIALIZE_TPL(_Type, _Name) \
    _ONE_TIME_INITIALIZE_IMPL_(COMMA_PROTECT(_Type), _Name, {}, , typename )

#define ONE_TIME_INITIALIZE_THREAD_LOCAL_TPL(_Type, _Name, ...) \
    _ONE_TIME_INITIALIZE_IMPL_(COMMA_PROTECT(_Type), _Name, {COMMA_PROTECT(__VA_ARGS__)}, THREAD_LOCAL, typename )
#define ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL_TPL(_Type, _Name) \
    _ONE_TIME_INITIALIZE_IMPL_(COMMA_PROTECT(_Type), _Name, {}, THREAD_LOCAL, typename )
