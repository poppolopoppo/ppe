#pragma once

#include "Serialize.h"

#include "Parser/ParseItem.h"

#include "RTTI/Typedefs.h"

#include "Allocator/PoolAllocator.h"

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParseContext;
FWD_REFPTR(ParseExpression);
FWD_REFPTR(ParseStatement);
//----------------------------------------------------------------------------
class FParseStatement : public FParseItem {
public:
    FParseStatement(const Lexer::FSpan& site);
    virtual ~FParseStatement();

    virtual void Execute(FParseContext *context) const = 0;
    virtual void Invoke(FParseContext *context) const override { Execute(context); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEvalExpr : public FParseStatement {
public:
    FEvalExpr(const Parser::PCParseExpression& expr);
    virtual  ~FEvalExpr();

    const Parser::PCParseExpression& Expr() const { return _expr; }

    virtual void Execute(FParseContext *context) const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    Parser::PCParseExpression _expr;
};
//----------------------------------------------------------------------------
inline const FEvalExpr *MakeEvalExpr(const Parser::PCParseExpression& expr) {
    return new FEvalExpr(expr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FPropertyAssignment : public FParseStatement {
public:
    FPropertyAssignment(const RTTI::FName& name, const Parser::PCParseExpression& value, const Lexer::FSpan& site);
    virtual  ~FPropertyAssignment();

    const RTTI::FName& Name() const { return _name; }
    const Parser::PCParseExpression& Value() const { return _value; }

    virtual void Execute(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    RTTI::FName _name;
    Parser::PCParseExpression _value;
};
//----------------------------------------------------------------------------
inline const FPropertyAssignment *MakePropertyAssignment(
    const RTTI::FName& name,
    const Parser::PCParseExpression& value,
    const Lexer::FSpan& site) {
    return new FPropertyAssignment(name, value, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
