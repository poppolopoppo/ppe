#pragma once

#include "Core/Core.h"

#include "Core/Time/Timepoint.h"

#ifndef FINAL_RELEASE
#   define BENCHMARK_SCOPE(_CATEGORY, _MSG) \
        const ::Core::FBenchmarkScope CONCAT(__benchmarkScope, __LINE__)((_CATEGORY), (_MSG))
#   define IOBENCHMARK_SCOPE(_CATEGORY, _MSG, _SIZE_IN_BYTES_PTR) \
        const ::Core::FIOBenchmarkScope CONCAT(__IObenchmarkScope, __LINE__)((_CATEGORY), (_MSG), (_SIZE_IN_BYTES_PTR))
#else
#   define BENCHMARK_SCOPE(_CATEGORY, _MSG) NOOP()
#   define IOBENCHMARK_SCOPE(_CATEGORY, _MSG, _SIZE_IN_BYTES_PTR) NOOP()
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTimedScope {
public:
    FTimedScope() : _startedAt(FTimepoint::Now()) {}

    const FTimepoint& StartedAt() const { return _startedAt; }
    FTimespan Elapsed() const { return FTimepoint::ElapsedSince(_startedAt); }

private:
    FTimepoint _startedAt;
};
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
class FBenchmarkScope : public FTimedScope {
public:
    FBenchmarkScope(const wchar_t* category, const wchar_t* message);
    ~FBenchmarkScope();

private:
    const wchar_t* const _category;
    const wchar_t* const _message;
};
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
class FIOBenchmarkScope : public FTimedScope {
public:
    FIOBenchmarkScope(const wchar_t* category, const wchar_t* message, const std::streamsize* pSizeInBytes);
    ~FIOBenchmarkScope();

private:
    const wchar_t* const _category;
    const wchar_t* const _message;
    const std::streamsize* _pSizeInBytes;
};
#endif //!FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
