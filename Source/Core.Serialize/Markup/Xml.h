#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Markup/Markup.h"

#include "Core/IO/String.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FXml : public FMarkup {
public:
    FXml();
    virtual ~FXml();

    const FString& Version() const { return _version; }
    const FString& Encoding() const { return _encoding; }
    const FString& Standalone() const { return _standalone; }

    void SetVersion(FString&& str) { _version = std::move(str); }
    void SetEncoding(FString&& str) { _encoding = std::move(str); }
    void SetStandalone(FString&& str) { _standalone = std::move(str); }

    using FMarkup::Load;

protected: // FMarkup
    virtual bool ReadHeaders(Lexer::FLexer& lexer) override;
    virtual void WriteHeaders(FTextWriter& oss, bool minify) const override;

private:
    FString _version;
    FString _encoding;
    FString _standalone;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
