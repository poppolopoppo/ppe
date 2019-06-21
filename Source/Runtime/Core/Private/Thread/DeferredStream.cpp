#include "stdafx.h"

#include "Thread/DeferredStream.h"

#include "IO/BufferedStream.h"
#include "Thread/ThreadPool.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeferredStreamReader::FDeferredStreamReader(IStreamReader* nonBuffered, ETaskPriority priority /* = ETaskPriority::Normal */)
    : _nonBuffered(nonBuffered)
    , _priority(priority) {
    Assert(_nonBuffered);
    AssertRelease_NoAssume(_nonBuffered->ToBufferedI() == nullptr);
}
//----------------------------------------------------------------------------
FDeferredStreamReader::~FDeferredStreamReader() = default;
//----------------------------------------------------------------------------
bool FDeferredStreamReader::Read(void* storage, std::streamsize sizeInBytes) {
    Assert(_nonBuffered);

    struct {
        bool Succeed;
        void* Storage;
        std::streamsize SizeInBytes;
    }   query{ false, storage, sizeInBytes };

    FIOThreadPool::Get().RunAndWaitFor([this, &query](ITaskContext&) {
        query.Succeed = _nonBuffered->Read(query.Storage, query.SizeInBytes);
    },  _priority);

    return query.Succeed;
}
//----------------------------------------------------------------------------
size_t FDeferredStreamReader::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(_nonBuffered);
    Assert(eltsize);
    Assert(count);

    struct {
        size_t Read;
        void* Storage;
        size_t EltSize, Count;
    }   query{ false, storage, eltsize, count };

    FIOThreadPool::Get().RunAndWaitFor([this, &query](ITaskContext&) {
        query.Read = _nonBuffered->ReadSome(query.Storage, query.EltSize, query.Count);
    },  _priority);

    return query.Read;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeferredStreamWriter::FDeferredStreamWriter(IStreamWriter* nonBuffered, ETaskPriority priority /* = ETaskPriority::Normal */)
:   _nonBuffered(nonBuffered)
,   _priority(priority) {
    Assert(_nonBuffered);
    AssertRelease_NoAssume(_nonBuffered->ToBufferedO() == nullptr);
}
//----------------------------------------------------------------------------
FDeferredStreamWriter::~FDeferredStreamWriter() = default;
//----------------------------------------------------------------------------
bool FDeferredStreamWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    Assert(_nonBuffered);

    struct {
        bool Succeed;
        const void* Storage;
        std::streamsize SizeInBytes;
    }   query{ false, storage, sizeInBytes };

    FIOThreadPool::Get().RunAndWaitFor([this, &query](ITaskContext&) {
        query.Succeed = _nonBuffered->Write(query.Storage, query.SizeInBytes);
    },  _priority);

    return query.Succeed;
}
//----------------------------------------------------------------------------
size_t FDeferredStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(_nonBuffered);
    Assert(eltsize);
    Assert(count);

    struct {
        size_t Read;
        const void* Storage;
        size_t EltSize, Count;
    }   query{ false, storage, eltsize, count };

    FIOThreadPool::Get().RunAndWaitFor([this, &query](ITaskContext&) {
        query.Read = _nonBuffered->WriteSome(query.Storage, query.EltSize, query.Count);
    },  _priority);

    return query.Read;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
