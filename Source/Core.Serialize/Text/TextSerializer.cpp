#include "stdafx.h"

#include "TextSerializer.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaTransaction.h"

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
void TextSerializer::Deserialize(VECTOR(Transaction, RTTI::PMetaObject)& objects, const MemoryView<const u8>& input, const wchar_t *sourceName /* = nullptr */) {
    Assert(input.SizeInBytes());

    Lexer::Lexer lexer(input.Cast<const char>(), sourceName);

    Parser::ParseList parseList(&lexer);
    Parser::ParseContext parseContext(_transaction.get());

    while (Parser::PCParseItem result = Grammar_Parse(parseList)) {
        const Parser::ParseExpression *expr = result->As<Parser::ParseExpression>();
        if (expr) {
            const RTTI::PMetaAtom atom = expr->Eval(&parseContext);
            const auto* object = atom->As<RTTI::PMetaObject>();
            AssertRelease(object);
            objects.emplace_back(object->Wrapper());
        }
    }
}
//----------------------------------------------------------------------------
void TextSerializer::Serialize(IStreamWriter* output, const MemoryView<const RTTI::PMetaObject>& objects) {
    Assert(output);
    UNUSED(output);
    UNUSED(objects);
    // TODO: reverse grammar.cpp + MetaAtomWrapCopyVisitor
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
