#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaType.h"

#include "Core.Serialize/Parser/ParseItem.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParseContext;
FWD_REFPTR(ParseExpression);
FWD_REFPTR(ParseStatement);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParseExpression : public FParseItem {
public:
    FParseExpression(const Lexer::FLocation& site);
    virtual ~FParseExpression();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const = 0;
    virtual void Invoke(FParseContext *context) const override { Eval(context); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TLiteral : public FParseExpression {
public:
    typedef typename RTTI::TMetaAtomWrapper< T >::type atom_type;

    explicit TLiteral(T&& rvalue, const Lexer::FLocation& site);
    virtual ~TLiteral();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TRefPtr< atom_type > _literal;
};
//----------------------------------------------------------------------------
template <typename T>
TLiteral<T> *MakeLiteral(T&& rvalue, const Lexer::FLocation& site) {
    return new TLiteral<T>(std::move(rvalue), site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVariableExport : public FParseExpression {
public:
    enum EFlags {
        Public,
        Private,
        Global,
    };

    explicit FVariableExport(const RTTI::FName& name, const PCParseExpression& value, const EFlags scope, const Lexer::FLocation& site);
    virtual ~FVariableExport();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    RTTI::FName _name;
    PCParseExpression _value;
    EFlags _scope;
};
//----------------------------------------------------------------------------
inline FVariableExport *MakeVariableExport(const RTTI::FName& name, const PCParseExpression& value, const FVariableExport::EFlags scope, const Lexer::FLocation& site) {
    return new FVariableExport(name, value, scope, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVariableReference : public FParseExpression {
public:
    explicit FVariableReference(const RTTI::FName& name, const Lexer::FLocation& site);
    virtual ~FVariableReference();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    RTTI::FName _name;
};
//----------------------------------------------------------------------------
inline FVariableReference *MakeVariableReference(const RTTI::FName& name, const Lexer::FLocation& site) {
    return new FVariableReference(name, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class TUnaryFunction : public FParseExpression {
public:
    explicit TUnaryFunction(_Functor&& functor, const FParseExpression *expr, const Lexer::FLocation& site);
    virtual ~TUnaryFunction();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    _Functor _functor;
    PCParseExpression _expr;
};
//----------------------------------------------------------------------------
template <typename T>
TUnaryFunction<T> *MakeUnaryFunction(T&& functor, const FParseExpression *expr, const Lexer::FLocation& site) {
    return new TUnaryFunction<T>(std::move(functor), expr, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class TBinaryFunction : public FParseExpression {
public:
    explicit TBinaryFunction(_Functor&& functor, const FParseExpression *lhs, const FParseExpression *rhs, const Lexer::FLocation& site);
    virtual ~TBinaryFunction();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    _Functor _functor;
    PCParseExpression _lhs;
    PCParseExpression _rhs;
};
//----------------------------------------------------------------------------
template <typename T>
TBinaryFunction<T> *MakeBinaryFunction(T&& functor, const FParseExpression *lhs, const FParseExpression *rhs, const Lexer::FLocation& site) {
    return new TBinaryFunction<T>(std::move(functor), lhs, rhs, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Test>
class TTernary : public FParseExpression {
public:
    explicit TTernary(_Test&& test, const FParseExpression *pif, const FParseExpression *ptrue, const FParseExpression *pfalse, const Lexer::FLocation& site);
    virtual ~TTernary();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    _Test _test;
    PCParseExpression _if;
    PCParseExpression _true;
    PCParseExpression _false;
};
//----------------------------------------------------------------------------
template <typename _Test>
TTernary<_Test> *MakeTernary(_Test&& test, const FParseExpression *pif, const FParseExpression *ptrue, const FParseExpression *pfalse, const Lexer::FLocation& site) {
    return new TTernary<_Test>(std::move(test), pif, ptrue, pfalse, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FObjectDefinition : public FParseExpression {
public:
    explicit FObjectDefinition(const RTTI::FName& name, const Lexer::FLocation& site);
    virtual ~FObjectDefinition();

    void AddStatement(const FParseStatement *statement);

    template <typename _It>
    void AddStatements(_It&& begin, _It&& end) {
        _statements.insert(_statements.end(), begin, end);
    }

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    RTTI::FName _name;
    VECTOR(Parser, PCParseStatement) _statements;
};
//----------------------------------------------------------------------------
template <typename _It>
inline FObjectDefinition *MakeObjectDefinition(
    const RTTI::FName& name,
    const Lexer::FLocation& site,
    _It&& statementsBegin, _It&& statementsEnd) {
    FObjectDefinition *const def = new FObjectDefinition(name, site);
    def->AddStatements(statementsBegin, statementsEnd);
    return def;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FPropertyReference : public FParseExpression {
public:
    explicit FPropertyReference(const PCParseExpression& object, const RTTI::FName& member, const Lexer::FLocation& site);
    virtual ~FPropertyReference();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    PCParseExpression _object;
    RTTI::FName _member;
};
//----------------------------------------------------------------------------
inline FPropertyReference *MakePropertyReference(
    const PCParseExpression& object,
    const RTTI::FName& member,
    const Lexer::FLocation& site) {
    return new FPropertyReference(object, member, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TPair : public FParseExpression {
public:
    explicit TPair(const PCParseExpression& lhs, const PCParseExpression& rhs, const Lexer::FLocation& site);
    virtual ~TPair();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    PCParseExpression _lhs;
    PCParseExpression _rhs;
};
//----------------------------------------------------------------------------
inline Parser::TPair *MakePair(
    const PCParseExpression& lhs,
    const PCParseExpression& rhs,
    const Lexer::FLocation& site) {
    return new Parser::TPair(lhs, rhs, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TArray : public FParseExpression {
public:
    explicit TArray(const Lexer::FLocation& site);
    TArray(const TMemoryView<const PCParseExpression>& items, const Lexer::FLocation& site);
    virtual ~TArray();

    size_t size() const { return _items.size(); }
    bool empty() const { return _items.empty(); }
    void reserve(size_t capacity) { return _items.reserve(capacity); }
    void push_back(const PCParseExpression& expr) { _items.push_back(expr); }

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    VECTOR_THREAD_LOCAL(Parser, PCParseExpression) _items;
};
//----------------------------------------------------------------------------
inline Parser::TArray *MakeArray(
    const TMemoryView<const PCParseExpression>& items,
    const Lexer::FLocation& site) {
    return new Parser::TArray(items, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TDictionary : public FParseExpression {
public:
    explicit TDictionary(const Lexer::FLocation& site);
    TDictionary(const TMemoryView<const Core::TPair<PCParseExpression, PCParseExpression>>& items, const Lexer::FLocation& site);
    virtual ~TDictionary();

    size_t size() const { return _items.size(); }
    bool empty() const { return _items.empty(); }
    void reserve(size_t capacity) { return _items.reserve(capacity); }
    void insert(const PCParseExpression& key, const PCParseExpression& value) { _items.Insert_AssertUnique(key, value); }

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ASSOCIATIVE_VECTOR_THREAD_LOCAL(Parser, PCParseExpression, PCParseExpression) _items;
};
//----------------------------------------------------------------------------
inline Parser::TDictionary *MakeDictionary(
    const TMemoryView<const Core::TPair<PCParseExpression, PCParseExpression>>& items,
    const Lexer::FLocation& site) {
    return new Parser::TDictionary(items, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FCastExpr : public FParseExpression {
public:
    FCastExpr(RTTI::FMetaTypeId typeId, const FParseExpression* expr, const Lexer::FLocation& site);
    virtual ~FCastExpr();

    virtual RTTI::FMetaAtom *Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    RTTI::FMetaTypeId _typeId;
    PCParseExpression _expr;
};
//----------------------------------------------------------------------------
inline FCastExpr *MakeCastExpr(RTTI::FMetaTypeId typeId, const FParseExpression* expr, const Lexer::FLocation& site) {
    return new FCastExpr(typeId, expr, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core

#include "Core.Serialize/Parser/ParseExpression-inl.h"
