#include "stdafx.h"

#include "ParseStatement.h"

#include "ParseContext.h"
#include "ParseExpression.h"
#include "Parser.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Format.h"
#include "Core/IO/StringBuilder.h"

#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/TypeInfos.h"

namespace Core {
namespace Parser {
LOG_CATEGORY(CORE_SERIALIZE_API, Parser)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FParseStatement::FParseStatement(const Lexer::FLocation& site)
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
    const RTTI::FAtom result = _expr->Eval(context);
    if (result) {
        LOG(Parser, Info, L"<{0}> : #{2} = {1}", result.TypeInfos(), result.ToString(), result.HashValue());
        // TODO : any side effect ?
    }
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

    const RTTI::FMetaClass *metaclass = context->ScopeObject()->RTTI_Class();
    Assert(metaclass);

    const RTTI::FMetaProperty *metaproperty = metaclass->PropertyIFP(_name);
    if (!metaproperty)
        CORE_THROW_IT(FParserException("unknowm property name", this));

    const RTTI::FAtom src = _value->Eval(context);
    const RTTI::FAtom dst = metaproperty->Get(*obj);

    if (not src.PromoteMove(dst))
        CORE_THROW_IT(FParserException("invalid property assignment", this));
}
//----------------------------------------------------------------------------
FString FPropertyAssignment::ToString() const {
    return Core::ToString(_name.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
