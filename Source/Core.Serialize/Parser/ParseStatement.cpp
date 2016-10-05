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
FParseStatement::FParseStatement(const FLexer::FLocation& site)
:   FParseItem(site) {}
//----------------------------------------------------------------------------
FParseStatement::~FParseStatement() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FEvalExpr, )
//----------------------------------------------------------------------------
FEvalExpr::FEvalExpr(const Parser::PCParseExpression& expr)
:   FParseStatement(expr->Site())
,   _expr(expr) {
    Assert(expr);
}
//----------------------------------------------------------------------------
FEvalExpr::~FEvalExpr() {}
//----------------------------------------------------------------------------
void FEvalExpr::Execute(FParseContext *context) const {
    const RTTI::PCMetaAtom result = _expr->Eval(context);
    if (result)
        Format(std::cout, "<{0}> = {1} ({2})\n", result->TypeInfo(), result->ToString(), result->HashValue());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FPropertyAssignment, )
//----------------------------------------------------------------------------
FPropertyAssignment::FPropertyAssignment(
    const RTTI::FName& name,
    const Parser::PCParseExpression& value)
:   FParseStatement(value->Site())
,   _name(name)
,   _value(value) {
    Assert(!name.empty());
    Assert(value);
}
//----------------------------------------------------------------------------
FPropertyAssignment::~FPropertyAssignment() {}
//----------------------------------------------------------------------------
void FPropertyAssignment::Execute(FParseContext *context) const {
    Assert(context);

    RTTI::FMetaObject *obj = context->ScopeObject();
    Assert(obj);

    const RTTI::FMetaClass *metaclass = context->ScopeObject()->RTTI_MetaClass();
    Assert(metaclass);

    const RTTI::FMetaProperty *metaproperty = metaclass->PropertyIFP(_name);
    if (!metaproperty)
        CORE_THROW_IT(FParserException("unknowm property name", this));

    const RTTI::PMetaAtom value = _value->Eval(context);

    if (!metaproperty->UnwrapMove(obj, value.get()))
        CORE_THROW_IT(FParserException("invalid property assignment", this));
}
//----------------------------------------------------------------------------
FString FPropertyAssignment::ToString() const {
    return _name.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
