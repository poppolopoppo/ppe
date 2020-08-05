#pragma once

#include "Network_fwd.h"

#include "IO/StreamProvider.h"

namespace PPE {
namespace Network {
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
    virtual bool IsSeekableO(ESeekOrigin ) const override final { return false; }

    virtual std::streamoff TellO() const override final;
    virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin policy = ESeekOrigin::Begin) override final;

    virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final;
    virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final;

private:
    FSocketBuffered& _socket;
    std::streamoff _tellO;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
