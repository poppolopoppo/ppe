#pragma once

#include "Serialize.h"

#include "Parser/ParseList.h"

#include "Lexer/Location.h"
#include "Lexer/Symbol.h"

#include "Meta/PointerWFlags.h"

#include <functional>

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FParseResult {
    const char* Error;
    Lexer::FSpan Site;
    Lexer::FSymbol::ETypeId Expected;

    PPE_FAKEBOOL_OPERATOR_DECL() { return (Error ? nullptr : this); }

    bool Succeed() const { return (Error == nullptr); }

    static FParseResult Success(const Lexer::FSpan& site) {
        FParseResult r;
        r.Error = nullptr;
        r.Site = site;
        r.Expected = Lexer::FSymbol::Invalid;
        return r;
    }

    static FParseResult Success(const Lexer::FSpan& from, const Lexer::FSpan& to) {
        return Success(Lexer::FSpan::FromSpan(from, to));
    }

    static FParseResult Failure(const char* message, Lexer::FSymbol::ETypeId expected, const Lexer::FSpan& site) {
        FParseResult r;
        r.Error = message;
        r.Site = site;
        r.Expected = expected;
        return r;
    }

    static FParseResult Failure(const Lexer::FSpan& site) {
        return Failure("failed evaluation", Lexer::FSymbol::Invalid, site);
    }

    static FParseResult Unexpected(Lexer::FSymbol::ETypeId expected, const FParseMatch* found, const FParseList& input) {
        return Failure("unexpected match", expected, found ? found->Site() : input.Site());
    }

    inline friend FParseResult operator &&(const FParseResult& lhs, const FParseResult& rhs) NOEXCEPT {
        if (not lhs) return lhs;
        else if (not rhs) return rhs;
        else return Success(lhs.Site.Offset < rhs.Site.Offset
            ? Lexer::FSpan::FromSpan(lhs.Site, rhs.Site)
            : Lexer::FSpan::FromSpan(rhs.Site, lhs.Site) );
    }
    inline friend FParseResult operator ||(const FParseResult& lhs, const FParseResult& rhs) NOEXCEPT {
        return (lhs ? lhs : rhs);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
