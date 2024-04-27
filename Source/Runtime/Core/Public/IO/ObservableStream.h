#pragma once

#include "Core_fwd.h"

#include "IO/StreamProvider.h"
#include "Memory/PtrRef.h"
#include "Misc/Function.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FObservableStreamReader final : public IStreamReader {
public:
    using FOnRead = TFunction<void(std::streamoff position, void** storage, std::streamsize* size)>;

    FObservableStreamReader(TPtrRef<IStreamReader> reader) NOEXCEPT
    :   _reader(std::move(reader))
    {}

    FObservableStreamReader(TPtrRef<IStreamReader> reader, FOnRead&& onRead) NOEXCEPT
    :   _reader(std::move(reader))
    ,   _onRead(std::move(onRead))
    {}

    FObservableStreamReader(TPtrRef<IStreamReader> reader, const FOnRead& onRead)
    :   _reader(std::move(reader))
    ,   _onRead(onRead)
    {}

    TPtrRef<IStreamReader> Reader() const { return _reader; }
    void SetReader(TPtrRef<IStreamReader> value) { _reader = value; }

    const FOnRead& OnRead() const { return _onRead; }
    void SetOnRead(const FOnRead& value) { _onRead = value; }
    void SetOnRead(FOnRead&& rvalue) { _onRead = std::move(rvalue); }

public: // IStreamReader interface
    PPE_CORE_API virtual bool Eof() const NOEXCEPT override;

    PPE_CORE_API virtual bool IsSeekableI(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override;

    PPE_CORE_API virtual std::streamoff TellI() const NOEXCEPT override;

    PPE_CORE_API virtual std::streamoff SeekI(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override;

    PPE_CORE_API virtual std::streamsize SizeInBytes() const NOEXCEPT override;

    PPE_CORE_API virtual bool Read(void* storage, std::streamsize sizeInBytes) override;

    PPE_CORE_API virtual size_t ReadSome(void* storage, size_t eltsize, size_t count) override;

    virtual IBufferedStreamReader* ToBufferedI() NOEXCEPT override final {
        return nullptr; // don't support buffered interface, even if _reader do (would require more work to handle buffered api observation)
    }

private:
    TPtrRef<IStreamReader> _reader;
    FOnRead _onRead{ NoFunction };
    mutable std::streamoff _position{ 0 };
};
//----------------------------------------------------------------------------
class FObservableStreamWriter final : public IStreamWriter {
public:
    using FOnWrite = TFunction<void(std::streamoff position, void const** storage, std::streamsize* size)>;

    FObservableStreamWriter(TPtrRef<IStreamWriter> writer) NOEXCEPT
    :   _writer(std::move(writer))
    {}

    FObservableStreamWriter(TPtrRef<IStreamWriter> writer, FOnWrite&& onWrite) NOEXCEPT
    :   _writer(std::move(writer))
    ,   _onWrite(std::move(onWrite))
    {}

    FObservableStreamWriter(TPtrRef<IStreamWriter> writer, const FOnWrite& onWrite)
    :   _writer(std::move(writer))
    ,   _onWrite(onWrite)
    {}

    TPtrRef<IStreamWriter> Writer() const { return _writer; }
    void SetWriter(TPtrRef<IStreamWriter> value) { _writer = value; }

    const FOnWrite& OnWrite() const { return _onWrite; }
    void SetOnWrite(const FOnWrite& value) { _onWrite = value; }
    void SetOnWrite(FOnWrite&& rvalue) { _onWrite = std::move(rvalue); }

public: // IStreamWriter interface
    PPE_CORE_API virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override;

    PPE_CORE_API virtual std::streamoff TellO() const NOEXCEPT override;

    PPE_CORE_API virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override;

    PPE_CORE_API virtual bool Write(const void* storage, std::streamsize sizeInBytes) override;

    PPE_CORE_API virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override;

    virtual IBufferedStreamWriter* ToBufferedO() NOEXCEPT override final {
        return nullptr; // don't support buffered interface, even if _reader do (would require more work to handle buffered api observation)
    }

private:
    TPtrRef<IStreamWriter> _writer;
    FOnWrite _onWrite{ NoFunction };
    mutable std::streamoff _position{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FObservableStreamReader StreamReaderWithProgress(TPtrRef<IStreamReader> reader, const FStringView& message);
//----------------------------------------------------------------------------
template <typename _Lambda>
auto UsingStreamWithProgress(TPtrRef<IStreamReader> reader, const FStringView& message, _Lambda&& lambda)
    -> decltype(std::declval<_Lambda&&>()(std::declval<FObservableStreamReader*>())) {
    FObservableStreamReader wprogress{ StreamReaderWithProgress(reader, message) };
    return lambda(&wprogress);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "IO/StreamProvider-inl.h"
