#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Allocator/PoolAllocatorTag.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Lexer/LookAheadReader.h"
#include "Core.Serialize/Lexer/Match.h"

#include "Core/IO/String.h"

#include <stdexcept>

namespace Core {
namespace Lexer {
POOL_TAG_DECL(Lexer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LexerException : public Core::Serialize::SerializeException {
public:
    typedef Core::Serialize::SerializeException parent_type;

    LexerException(const char *what, Match&& match)
        :   parent_type(what)
        ,   _match(std::move(match)) {}

    virtual ~LexerException() {}

    const Core::Lexer::Match& Match() const { return _match; }

private:
    Core::Lexer::Match _match;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Lexer {
public:
    Lexer(const StringView& input, const WStringView& sourceFileName, bool allowTypenames);
    ~Lexer();

    const Match *Peek();
    const Match *Peek(const Symbol* symbol);

    bool Read(Match& match);
    bool ReadUntil(Match& match, const char ch);
    bool SkipUntil(const char ch);

    bool Expect(Match& match, const Core::Lexer::Symbol* expected);

    void Rewind(const Match& at);
    void Seek(size_t offset);
    size_t Tell() const { return _reader.Tell(); }

    const WString& SourceFileName() { return _sourceFileName; }

private:
    bool NextMatch_(Match& match);

    WString _sourceFileName;
    LookAheadReader _reader;

    String _lexing;
    Match _peek;

    bool _allowTypenames;
    bool _peeking;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct LexerStartup {
    static void Start();
    static void Shutdown();
    static void ClearAll_UnusedMemory();

    LexerStartup() { Start(); }
    ~LexerStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
