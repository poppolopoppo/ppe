// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/ObservableStream.h"

#include "Diagnostic/FeedbackContext.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FObservableStreamReader::Eof() const NOEXCEPT {
    return _reader->Eof();
}
//----------------------------------------------------------------------------
bool FObservableStreamReader::IsSeekableI(ESeekOrigin origin/* = ESeekOrigin::All */) const NOEXCEPT {
    return _reader->IsSeekableI(origin);
}
//----------------------------------------------------------------------------
std::streamoff FObservableStreamReader::TellI() const NOEXCEPT {
    _position = _reader->TellI();
    return _position;
}
//----------------------------------------------------------------------------
std::streamoff FObservableStreamReader::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    _position = _reader->SeekI(offset, origin);
    return _position;
}
//----------------------------------------------------------------------------
std::streamsize FObservableStreamReader::SizeInBytes() const NOEXCEPT {
    return _reader->SizeInBytes();
}
//----------------------------------------------------------------------------
bool FObservableStreamReader::Read(void* storage, std::streamsize sizeInBytes) {
    if (_reader->Read(storage, sizeInBytes)) {
        _position += sizeInBytes;
        _onRead(_position, &storage, &sizeInBytes);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
size_t FObservableStreamReader::ReadSome(void* storage, size_t eltsize, size_t count) {
    const size_t numElts = _reader->ReadSome(storage, eltsize, count);

    std::streamsize sizeInBytes = (numElts * static_cast<std::streamsize>(eltsize));
    _position += sizeInBytes;
    _onRead(_position, &storage, &sizeInBytes);
    Assert_NoAssume(Meta::IsAligned(eltsize, sizeInBytes));

    return (sizeInBytes / eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FObservableStreamWriter::IsSeekableO(ESeekOrigin origin/* = ESeekOrigin::All */) const NOEXCEPT {
    return _writer->IsSeekableO(origin);
}
//----------------------------------------------------------------------------
std::streamoff FObservableStreamWriter::TellO() const NOEXCEPT {
    _position = _writer->TellO();
    return _position;
}
//----------------------------------------------------------------------------
std::streamoff FObservableStreamWriter::SeekO(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    _position = _writer->SeekO(offset, origin);
    return _position;
}
//----------------------------------------------------------------------------
bool FObservableStreamWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    _onWrite(_position, &storage, &sizeInBytes);

    if (_writer->Write(storage, sizeInBytes)) {
        _position += sizeInBytes;
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
size_t FObservableStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    std::streamsize sizeInBytes = (eltsize * count);
    _onWrite(_position, &storage, &sizeInBytes);

    const size_t writtenSize = _writer->WriteSome(storage, eltsize, count);
    _position += writtenSize;

    return writtenSize;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FObservableStreamReader StreamReaderWithProgress(TPtrRef<IStreamReader> reader, const FStringView& message) {
    return FObservableStreamReader(reader, [pbar(FFeedbackProgressBar{message, 100}), totalInBytes(reader->SizeInBytes())]
        (std::streamoff pos, void** storage, std::streamsize* sizeInBytes) {
            Unused(storage, sizeInBytes);
            pbar.Set(checked_cast<size_t>((pos * 100) / totalInBytes));
        });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
