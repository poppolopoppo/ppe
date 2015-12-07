#include "stdafx.h"

#include "ParseStatement.h"

#include "ParseContext.h"
#include "ParseExpression.h"
#include "Parser.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/Format.h"

#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"

#include <iostream>

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ParseStatement::ParseStatement(const Lexer::Location& site)
:   ParseItem(site) {}
//----------------------------------------------------------------------------
ParseStatement::~ParseStatement() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(EvalExpr, )
//----------------------------------------------------------------------------
EvalExpr::EvalExpr(const Parser::PCParseExpression& expr)
:   ParseStatement(expr->Site())
,   _expr(expr) {
    Assert(expr);
}
//----------------------------------------------------------------------------
EvalExpr::~EvalExpr() {}
//----------------------------------------------------------------------------
void EvalExpr::Execute(ParseContext *context) const {
    const RTTI::PCMetaAtom result = _expr->Eval(context);
    if (result)
        Format(std::cout, "<{0}> = {1} ({2})\n", result->TypeInfo(), result->ToString(), result->HashValue());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(PropertyAssignment, )
//----------------------------------------------------------------------------
PropertyAssignment::PropertyAssignment(
    const RTTI::MetaPropertyName& name,
    const Parser::PCParseExpression& value)
:   ParseStatement(value->Site())
,   _name(name)
,   _value(value) {
    Assert(!name.empty());
    Assert(value);
}
//----------------------------------------------------------------------------
PropertyAssignment::~PropertyAssignment() {}
//----------------------------------------------------------------------------
void PropertyAssignment::Execute(ParseContext *context) const {
    Assert(context);

    RTTI::MetaObject *obj = context->ScopeObject();
    Assert(obj);

    const RTTI::MetaClass *metaclass = context->ScopeObject()->RTTI_MetaClass();
    Assert(metaclass);

    const RTTI::MetaProperty *metaproperty = metaclass->PropertyIFP(_name);
    if (!metaproperty)
        throw ParserException("unknowm property name", this);

    const RTTI::PMetaAtom value = _value->Eval(context);

    if (!metaproperty->UnwrapMove(obj, value.get()))
        throw ParserException("invalid property assignment", this);
}
//----------------------------------------------------------------------------
String PropertyAssignment::ToString() const {
    return _name.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
