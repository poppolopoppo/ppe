#pragma once

#include "Core.h"

#include "MetaObjectName.h"
#include "MetaPropertyName.h"

#include "ParseItem.h"
#include "PoolAllocator.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParseContext;
FWD_REFPTR(ParseExpression);
FWD_REFPTR(ParseStatement);
//----------------------------------------------------------------------------
class ParseStatement : public ParseItem {
public:
    ParseStatement(const Lexer::Location& site);
    virtual ~ParseStatement();

    virtual void Execute(ParseContext *context) const = 0;
    virtual void Invoke(ParseContext *context) const override { Execute(context); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EvalExpr : public ParseStatement {
public:
    EvalExpr(const Parser::PCParseExpression& expr);
    virtual  ~EvalExpr();

    const Parser::PCParseExpression& Expr() const { return _expr; }

    virtual void Execute(ParseContext *context) const override;

    SINGLETON_POOL_ALLOCATED_DECL(EvalExpr)

private:
    Parser::PCParseExpression _expr;
};
//----------------------------------------------------------------------------
inline const EvalExpr *MakeEvalExpr(const Parser::PCParseExpression& expr) {
    return new EvalExpr(expr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PropertyAssignment : public ParseStatement {
public:
    PropertyAssignment(const RTTI::MetaPropertyName& name, const Parser::PCParseExpression& value);
    virtual  ~PropertyAssignment();

    const RTTI::MetaPropertyName& Name() const { return _name; }
    const Parser::PCParseExpression& Value() const { return _value; }

    virtual void Execute(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(PropertyAssignment)

private:
    RTTI::MetaPropertyName _name;
    Parser::PCParseExpression _value;
};
//----------------------------------------------------------------------------
inline const PropertyAssignment *MakePropertyAssignment(
    const RTTI::MetaPropertyName& name,
    const Parser::PCParseExpression& value) {
    return new PropertyAssignment(name, value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
