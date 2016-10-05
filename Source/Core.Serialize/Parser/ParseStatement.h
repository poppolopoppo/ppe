#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Parser/ParseItem.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Allocator/PoolAllocator.h"

namespace Core {
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
    FParseStatement(const FLexer::FLocation& site);
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
    FPropertyAssignment(const RTTI::FName& name, const Parser::PCParseExpression& value);
    virtual  ~FPropertyAssignment();

    const RTTI::FName& FName() const { return _name; }
    const Parser::PCParseExpression& FValue() const { return _value; }

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
    const Parser::PCParseExpression& value) {
    return new FPropertyAssignment(name, value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
