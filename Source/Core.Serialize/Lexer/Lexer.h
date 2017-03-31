#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Allocator/PoolAllocatorTag.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Lexer/LookAheadReader.h"
#include "Core.Serialize/Lexer/Match.h"

#include "Core/IO/String.h"

#include <stdexcept>

namespace Core {
class IStreamReader;
namespace Lexer {
POOL_TAG_DECL(FLexer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLexerException : public Core::Serialize::FSerializeException {
public:
    typedef Core::Serialize::FSerializeException parent_type;

    FLexerException(const char *what, FMatch&& match)
        :   parent_type(what)
        ,   _match(std::move(match)) {}

    virtual ~FLexerException() {}

    const Core::Lexer::FMatch& Match() const { return _match; }

private:
    Core::Lexer::FMatch _match;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLexer {
public:
    FLexer(IStreamReader* input, const FWStringView& sourceFileName, bool allowTypenames);
    ~FLexer();

    const FMatch* Peek();
    const FMatch* Peek(const FSymbol* symbol);

    bool Read(FMatch& match);
    bool ReadUntil(FMatch& match, const char ch);
    bool SkipUntil(const char ch);

    bool ReadIFN(const Core::Lexer::FSymbol* expected);
    bool ReadIFN(FMatch& match, const Core::Lexer::FSymbol* expected);

    bool Expect(const Core::Lexer::FSymbol* expected);
    bool Expect(FMatch& match, const Core::Lexer::FSymbol* expected);

    const FWString& SourceFileName() { return _sourceFileName; }

private:
    bool NextMatch_(FMatch& match);

    FWString _sourceFileName;
    FLookAheadReader _reader;

    FString _lexing;
    FMatch _peek;

    bool _allowTypenames;
    bool _peeking;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLexerStartup {
    static void Start();
    static void Shutdown();
    static void ClearAll_UnusedMemory();

    FLexerStartup() { Start(); }
    ~FLexerStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
