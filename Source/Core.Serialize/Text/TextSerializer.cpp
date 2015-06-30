#include "stdafx.h"

#include "TextSerializer.h"

#include "Core/RTTI/Atom/MetaAtom.h"
#include "Core/RTTI/MetaTransaction.h"

#include "Lexer/Lexer.h"

#include "Parser/ParseExpression.h"
#include "Parser/ParseItem.h"
#include "Parser/ParseList.h"

#include "Grammar.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TextSerializer::TextSerializer(RTTI::MetaTransaction *transaction)
:   _transaction(transaction) {
    Assert(_transaction);
}
//----------------------------------------------------------------------------
TextSerializer::~TextSerializer() {}
//----------------------------------------------------------------------------
void TextSerializer::Deserialize(VECTOR(Transaction, RTTI::PMetaAtom)& atoms, const RAWSTORAGE(Serializer, u8)& input, const char *sourceName /* = nullptr */) {
    Assert(input.SizeInBytes());

    Lexer::Lexer lexer(input.MakeConstView().Cast<const char>(), sourceName);

    Parser::ParseList parseList(&lexer);
    Parser::ParseContext parseContext(_transaction.get());

    while (Parser::PCParseItem result = Grammar_Parse(parseList)) {
        const Parser::ParseExpression *expr = result->As<Parser::ParseExpression>();
        if (expr)
            atoms.push_back(expr->Eval(&parseContext));
    }
}
//----------------------------------------------------------------------------
void TextSerializer::Serialize(RAWSTORAGE(Serializer, u8)& /* output */, const MemoryView<const RTTI::PCMetaAtom>& atoms) {
    Assert(atoms.size());

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
