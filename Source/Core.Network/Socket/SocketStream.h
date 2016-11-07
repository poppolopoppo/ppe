#pragma once

#include "Core.Network/Network.h"

#include "Core/IO/StreamProvider.h"

namespace Core {
namespace Network {
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSocketStreamWriter : public IStreamWriter {
public:
    explicit FSocketStreamWriter(FSocketBuffered& socket);
    ~FSocketStreamWriter();

    FSocketStreamWriter(const FSocketStreamWriter& ) = delete;
    FSocketStreamWriter& operator =(const FSocketStreamWriter& ) = delete;

    const FSocketBuffered& Socket() const { return _socket; }

public: // IStreamWriter
    virtual bool IsSeekableO(ESeekOrigin ) const override { return false; }

    virtual std::streamoff TellO() const override;
    virtual bool SeekO(std::streamoff offset, ESeekOrigin policy = ESeekOrigin::Begin) override;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override;

private:
    FSocketBuffered& _socket;
    std::streamoff _tellO;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
