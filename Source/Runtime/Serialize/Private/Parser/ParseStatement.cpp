﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Parser/ParseStatement.h"

#include "Parser/ParseContext.h"
#include "Parser/ParseExpression.h"
#include "Parser/Parser.h"

#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "RTTI/TypeInfos.h"

#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"

namespace PPE {
namespace Parser {
LOG_CATEGORY(PPE_SERIALIZE_API, Parser)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FParseStatement::FParseStatement(const Lexer::FSpan& site)
:   FParseItem(site) {}
//----------------------------------------------------------------------------
FParseStatement::~FParseStatement() = default;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FEvalExpr::FEvalExpr(const Parser::PCParseExpression& expr)
:   FParseStatement(expr->Site())
,   _expr(expr) {
    Assert(expr);
}
//----------------------------------------------------------------------------
FEvalExpr::~FEvalExpr() = default;
//----------------------------------------------------------------------------
void FEvalExpr::Execute(FParseContext *context) const {
    const RTTI::FAtom result = _expr->Eval(context);
    if (result) {
        PPE_LOG(Parser, Info, "<{0}> : #{2} = {1}", result.NamedTypeInfos(), result.ToString(), result.HashValue());
        // TODO : any side effect ?
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPropertyDefinition::FPropertyDefinition(
    const RTTI::FName& name,
    const Parser::PCParseExpression& value,
    const Lexer::FSpan& site)
:   FParseStatement(site)
,   _name(name)
,   _value(value) {
    Assert(!name.empty());
    Assert(value);
}
//----------------------------------------------------------------------------
FPropertyDefinition::~FPropertyDefinition() = default;
//----------------------------------------------------------------------------
void FPropertyDefinition::Execute(FParseContext *context) const {
    Assert(context);

    RTTI::FMetaObject *obj = context->ScopeObject();
    Assert(obj);

    const RTTI::FMetaClass *metaclass = context->ScopeObject()->RTTI_Class();
    Assert(metaclass);

    const RTTI::FMetaProperty *metaproperty = metaclass->PropertyIFP(_name);
    if (!metaproperty)
        PPE_THROW_IT(FParserException("unknowm property name", this));

    const RTTI::FAtom src = _value->Eval(context);
    const RTTI::FAtom dst = metaproperty->Get(*obj);

    if (not src)
        PPE_THROW_IT(FParserException("can't assign void expression", _value.get()));

    if (not src.PromoteMove(dst))
        PPE_THROW_IT(FParserException("invalid property assignment", this));
}
//----------------------------------------------------------------------------
FString FPropertyDefinition::ToString() const {
    return PPE::ToString(_name.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
