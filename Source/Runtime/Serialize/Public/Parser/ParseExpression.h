#pragma once

#include "Serialize.h"

#include "Atom.h"
#include "MetaObject.h"
#include "NativeTypes.h"

#include "Parser/Parser.h"
#include "Parser/ParseItem.h"

#include "Allocator/LinearHeapAllocator.h"
#include "Allocator/PoolAllocator.h"
#include "Container/AssociativeVector.h"
#include "Container/Pair.h"
#include "Container/Vector.h"

namespace PPE {
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
class PPE_SERIALIZE_API FParseExpression : public FParseItem {
public:
    FParseExpression(const Lexer::FLocation& site);
    virtual ~FParseExpression();

    virtual RTTI::FAtom Eval(FParseContext *context) const = 0;
    virtual void Invoke(FParseContext *context) const override { Eval(context); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TLiteral : public FParseExpression {
public:
    explicit TLiteral(T&& rvalue, const Lexer::FLocation& site);
    explicit TLiteral(const T& value, const Lexer::FLocation& site);
    virtual ~TLiteral();

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    T _literal;
};
//----------------------------------------------------------------------------
template <typename T>
auto* MakeLiteral(T&& rvalue, const Lexer::FLocation& site) {
    return new TLiteral< Meta::TDecay<T> >(std::forward<T>(rvalue), site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FVariableExport : public FParseExpression {
public:
    enum EFlags {
        Public,
        Private,
        Global,
    };

    explicit FVariableExport(const RTTI::FName& name, const PCParseExpression& value, const EFlags scope, const Lexer::FLocation& site);
    virtual ~FVariableExport();

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
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
class PPE_SERIALIZE_API FVariableReference : public FParseExpression {
public:
    FVariableReference(const RTTI::FPathName& pathName, const Lexer::FLocation& site);
    virtual ~FVariableReference();

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    RTTI::FPathName _pathName;

};
//----------------------------------------------------------------------------
inline FVariableReference *MakeVariableReference(const RTTI::FPathName& pathName, const Lexer::FLocation& site) {
    return new FVariableReference(pathName, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class TUnaryFunction : public FParseExpression {
public:
    explicit TUnaryFunction(_Functor&& functor, const FParseExpression *expr, const Lexer::FLocation& site);
    virtual ~TUnaryFunction();

    virtual RTTI::FAtom Eval(FParseContext *context) const override;

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

    virtual RTTI::FAtom Eval(FParseContext *context) const override;

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

    virtual RTTI::FAtom Eval(FParseContext *context) const override;

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
class PPE_SERIALIZE_API FObjectDefinition : public FParseExpression {
public:
    FObjectDefinition(const RTTI::FName& name, const Lexer::FLocation& site);
    virtual ~FObjectDefinition();

    void AddStatement(const FParseStatement *statement);

    template <typename _It>
    void AddStatements(_It&& begin, _It&& end) {
        _statements.insert(_statements.end(), begin, end);
    }

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    RTTI::FName _name;
    VECTORINSITU(Parser, PCParseStatement, 3) _statements;
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
class PPE_SERIALIZE_API FPropertyReference : public FParseExpression {
public:
    FPropertyReference(const PCParseExpression& object, const RTTI::FName& member, const Lexer::FLocation& site);
    virtual ~FPropertyReference();

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
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
class PPE_SERIALIZE_API FTupleExpr : public FParseExpression {
public:
    using elements_type = VECTORINSITU(Parser, PCParseExpression, 4);

    explicit FTupleExpr(const Lexer::FLocation& site);
    FTupleExpr(elements_type&& relements, const Lexer::FLocation& site);
    virtual ~FTupleExpr();

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    elements_type _elements;
};
//----------------------------------------------------------------------------
inline Parser::FTupleExpr *MakeTupleExpr(FTupleExpr::elements_type&& relements, const Lexer::FLocation& site) {
    return new Parser::FTupleExpr(std::move(relements), site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FArrayExpr : public FParseExpression {
public:
    using items_type = VECTORINSITU(Parser, PCParseExpression, 4);

    explicit FArrayExpr(const Lexer::FLocation& site);
    FArrayExpr(items_type&& ritems, const Lexer::FLocation& site);
    virtual ~FArrayExpr();

    size_t size() const { return _items.size(); }
    bool empty() const { return _items.empty(); }
    void reserve(size_t capacity) { return _items.reserve(capacity); }
    void push_back(const PCParseExpression& expr) { _items.push_back(expr); }

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    items_type _items;
};
//----------------------------------------------------------------------------
inline Parser::FArrayExpr *MakeArrayExpr(const Lexer::FLocation& site) {
    return new Parser::FArrayExpr(site);
}
//----------------------------------------------------------------------------
inline Parser::FArrayExpr *MakeArrayExpr(FArrayExpr::items_type&& ritems, const Lexer::FLocation& site) {
    return new Parser::FArrayExpr(std::move(ritems), site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FDictionaryExpr : public FParseExpression {
public:
    using dico_type = ASSOCIATIVE_VECTORINSITU(Parser, PCParseExpression, PCParseExpression, 2);

    explicit FDictionaryExpr(const Lexer::FLocation& site);
    FDictionaryExpr(dico_type&& rdico, const Lexer::FLocation& site);
    virtual ~FDictionaryExpr();

    size_t size() const { return _dico.size(); }
    bool empty() const { return _dico.empty(); }
    void reserve(size_t capacity) { return _dico.reserve(capacity); }
    void insert(const PCParseExpression& key, const PCParseExpression& value) { _dico.Insert_AssertUnique(key, value); }

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    dico_type _dico;
};
//----------------------------------------------------------------------------
inline Parser::FDictionaryExpr *MakeDictionaryExpr(const Lexer::FLocation& site) {
    return new Parser::FDictionaryExpr(site);
}
//----------------------------------------------------------------------------
inline Parser::FDictionaryExpr *MakeDictionaryExpr(FDictionaryExpr::dico_type&& ritems, const Lexer::FLocation& site) {
    return new Parser::FDictionaryExpr(std::move(ritems), site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FCastExpr : public FParseExpression {
public:
    FCastExpr(RTTI::ENativeType typeId, const FParseExpression* expr, const Lexer::FLocation& site);
    virtual ~FCastExpr();

    virtual RTTI::FAtom Eval(FParseContext *context) const override;
    virtual FString ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    RTTI::ENativeType _typeId;
    PCParseExpression _expr;
};
//----------------------------------------------------------------------------
inline FCastExpr *MakeCastExpr(RTTI::ENativeType typeId, const FParseExpression* expr, const Lexer::FLocation& site) {
    return new FCastExpr(typeId, expr, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE

#include "Parser/ParseExpression-inl.h"