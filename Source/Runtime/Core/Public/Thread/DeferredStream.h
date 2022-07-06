#pragma once

#include "Core_fwd.h"

#include "IO/StreamProvider.h"
#include "Thread/Task/Task.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FDeferredStreamReader : public IStreamReader, Meta::FNonCopyableNorMovable {
public:
    explicit FDeferredStreamReader(IStreamReader* nonBuffered, ETaskPriority priority = ETaskPriority::Normal);
    virtual ~FDeferredStreamReader();

    ETaskPriority Priority() const { return _priority; }

    inline friend void swap(FDeferredStreamReader& lhs, FDeferredStreamReader& rhs) NOEXCEPT = delete;

public: // IStreamReader
    virtual bool Eof() const NOEXCEPT override final { return _nonBuffered->Eof(); }

    virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override final { return _nonBuffered->IsSeekableI(origin); }

    virtual std::streamoff TellI() const NOEXCEPT override final { return _nonBuffered->TellI(); }
    virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final { return _nonBuffered->SeekI(offset, origin); }

    virtual std::streamsize SizeInBytes() const NOEXCEPT override final { return _nonBuffered->SizeInBytes(); }

    virtual bool Read(void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override final;

private:
    IStreamReader* _nonBuffered;
    ETaskPriority _priority;
};
//----------------------------------------------------------------------------
class PPE_CORE_API FDeferredStreamWriter : public IStreamWriter, Meta::FNonCopyableNorMovable {
public:
    explicit FDeferredStreamWriter(IStreamWriter* nonBuffered, ETaskPriority priority = ETaskPriority::Normal);
    virtual ~FDeferredStreamWriter();

    ETaskPriority Priority() const { return _priority; }

    inline friend void swap(FDeferredStreamWriter& lhs, FDeferredStreamWriter& rhs) NOEXCEPT = delete;

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override final { return _nonBuffered->IsSeekableO(origin); }

    virtual std::streamoff TellO() const NOEXCEPT override final { return _nonBuffered->TellO(); }
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final { return _nonBuffered->SeekO(offset, origin); }

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

private:
    IStreamWriter* _nonBuffered;
    ETaskPriority _priority;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Lambda, class = decltype(std::declval<_Lambda>()(std::declval<IStreamReader*>()))>
std::false_type need_buffered_reader_(int);
template <typename _Lambda>
std::true_type need_buffered_reader_(...);
template <typename _Lambda>
constexpr bool need_buffered_reader = decltype(need_buffered_reader_<_Lambda>(0))::value;
} //!details
template <typename _Lambda>
auto UsingDeferredStream(IStreamReader* reader, _Lambda&& lambda) {
    Assert(reader);
    FDeferredStreamReader deferred(reader);
    IF_CONSTEXPR(details::need_buffered_reader<_Lambda>) {
        class FBufferedStreamReader buffered(&deferred);
        return lambda(&buffered);
    }
    else {
        return lambda(&deferred);
    }
}
//----------------------------------------------------------------------------
namespace details {
template <typename _Lambda, class = decltype(std::declval<_Lambda>()(std::declval<IStreamWriter*>()))>
std::false_type need_buffered_writer_(int);
template <typename _Lambda>
std::true_type need_buffered_writer_(...);
template <typename _Lambda>
constexpr bool need_buffered_writer = decltype(need_buffered_writer_<_Lambda>(0))::value;
} //!details
template <typename _Lambda>
auto UsingDeferredStream(IStreamWriter* writer, _Lambda&& lambda) {
    Assert(writer);
    FDeferredStreamWriter deferred(writer);
    IF_CONSTEXPR(details::need_buffered_writer<_Lambda>) {
        class FBufferedStreamWriter buffered(&deferred);
        return lambda(&buffered);
    }
    else {
        return lambda(&deferred);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
