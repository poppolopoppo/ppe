#pragma once

#include "Core/Core.h"

#include "Core/RTTI/Atom/MetaAtom.h"
#include "Core/RTTI/Class/MetaClassName.h"
#include "Core/RTTI/Object/MetaObjectName.h"
#include "Core/RTTI/Property/MetaPropertyName.h"
#include "Core/RTTI/Type/MetaType.h"

#include "Core/Parser/ParseItem.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParseContext;
FWD_REFPTR(ParseExpression);
FWD_REFPTR(ParseStatement);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParseExpression : public ParseItem {
public:
    ParseExpression(const Lexer::Location& site);
    virtual ~ParseExpression();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const = 0;
    virtual void Invoke(ParseContext *context) const override { Eval(context); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class Literal : public ParseExpression {
public:
    typedef RTTI::MetaWrappedAtom< typename std::decay<T>::type > atom_type;

    explicit Literal(T&& rvalue, const Lexer::Location& site);
    virtual ~Literal();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(Literal)

private:
    RefPtr< atom_type > _literal;
};
//----------------------------------------------------------------------------
template <typename T>
Literal<T> *MakeLiteral(T&& rvalue, const Lexer::Location& site) {
    return new Literal<T>(std::move(rvalue), site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VariableExport : public ParseExpression {
public:
    enum Flags {
        Public,
        Private,
        Global,
    };

    explicit VariableExport(const RTTI::MetaObjectName& name, const PCParseExpression& value, const Flags scope, const Lexer::Location& site);
    virtual ~VariableExport();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(VariableExport)

private:
    RTTI::MetaObjectName _name;
    PCParseExpression _value;
    Flags _scope;
};
//----------------------------------------------------------------------------
inline VariableExport *MakeVariableExport(const RTTI::MetaObjectName& name, const PCParseExpression& value, const VariableExport::Flags scope, const Lexer::Location& site) {
    return new VariableExport(name, value, scope, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VariableReference : public ParseExpression {
public:
    explicit VariableReference(const RTTI::MetaObjectName& name, const Lexer::Location& site);
    virtual ~VariableReference();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(VariableReference)

private:
    RTTI::MetaObjectName _name;
};
//----------------------------------------------------------------------------
inline VariableReference *MakeVariableReference(const RTTI::MetaObjectName& name, const Lexer::Location& site) {
    return new VariableReference(name, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class UnaryFunction : public ParseExpression {
public:
    explicit UnaryFunction(_Functor&& functor, const ParseExpression *expr, const Lexer::Location& site);
    virtual ~UnaryFunction();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;

    SINGLETON_POOL_ALLOCATED_DECL(UnaryFunction)

private:
    _Functor _functor;
    PCParseExpression _expr;
};
//----------------------------------------------------------------------------
template <typename T>
UnaryFunction<T> *MakeUnaryFunction(T&& functor, const ParseExpression *expr, const Lexer::Location& site) {
    return new UnaryFunction<T>(std::move(functor), expr, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class BinaryFunction : public ParseExpression {
public:
    explicit BinaryFunction(_Functor&& functor, const ParseExpression *lhs, const ParseExpression *rhs, const Lexer::Location& site);
    virtual ~BinaryFunction();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;

    SINGLETON_POOL_ALLOCATED_DECL(BinaryFunction)

private:
    _Functor _functor;
    PCParseExpression _lhs;
    PCParseExpression _rhs;
};
//----------------------------------------------------------------------------
template <typename T>
BinaryFunction<T> *MakeBinaryFunction(T&& functor, const ParseExpression *lhs, const ParseExpression *rhs, const Lexer::Location& site) {
    return new BinaryFunction<T>(std::move(functor), lhs, rhs, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Test>
class Ternary : public ParseExpression {
public:
    explicit Ternary(_Test&& test, const ParseExpression *pif, const ParseExpression *ptrue, const ParseExpression *pfalse, const Lexer::Location& site);
    virtual ~Ternary();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;

    SINGLETON_POOL_ALLOCATED_DECL(Ternary)

private:
    _Test _test;
    PCParseExpression _if;
    PCParseExpression _true;
    PCParseExpression _false;
};
//----------------------------------------------------------------------------
template <typename _Test>
Ternary<_Test> *MakeTernary(_Test&& test, const ParseExpression *pif, const ParseExpression *ptrue, const ParseExpression *pfalse, const Lexer::Location& site) {
    return new Ternary<_Test>(std::move(test), pif, ptrue, pfalse, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ObjectDefinition : public ParseExpression {
public:
    explicit ObjectDefinition(const RTTI::MetaClassName& name, const Lexer::Location& site);
    virtual ~ObjectDefinition();

    void AddStatement(const ParseStatement *statement);

    template <typename _It>
    void AddStatements(_It&& begin, _It&& end) {
        _statements.insert(_statements.end(), begin, end);
    }

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(ObjectDefinition)

private:
    RTTI::MetaClassName _name;
    VECTOR(Parser, PCParseStatement) _statements;
};
//----------------------------------------------------------------------------
template <typename _It>
inline ObjectDefinition *MakeObjectDefinition(
    const RTTI::MetaClassName& name,
    const Lexer::Location& site,
    _It&& statementsBegin, _It&& statementsEnd) {
    ObjectDefinition *const def = new ObjectDefinition(name, site);
    def->AddStatements(statementsBegin, statementsEnd);
    return def;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PropertyReference : public ParseExpression {
public:
    explicit PropertyReference(const PCParseExpression& object, const RTTI::MetaPropertyName& member, const Lexer::Location& site);
    virtual ~PropertyReference();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(PropertyReference)

private:
    PCParseExpression _object;
    RTTI::MetaPropertyName _member;
};
//----------------------------------------------------------------------------
inline PropertyReference *MakePropertyReference(
    const PCParseExpression& object,
    const RTTI::MetaPropertyName& member,
    const Lexer::Location& site) {
    return new PropertyReference(object, member, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Pair : public ParseExpression {
public:
    explicit Pair(const PCParseExpression& lhs, const PCParseExpression& rhs, const Lexer::Location& site);
    virtual ~Pair();

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(Pair)

private:
    PCParseExpression _lhs;
    PCParseExpression _rhs;
};
//----------------------------------------------------------------------------
inline Parser::Pair *MakePair(
    const PCParseExpression& lhs,
    const PCParseExpression& rhs,
    const Lexer::Location& site) {
    return new Parser::Pair(lhs, rhs, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Array : public ParseExpression {
public:
    explicit Array(const Lexer::Location& site);
    Array(const MemoryView<const PCParseExpression>& items, const Lexer::Location& site);
    virtual ~Array();

    size_t size() const { return _items.size(); }
    bool empty() const { return _items.empty(); }
    void reserve(size_t capacity) { return _items.reserve(capacity); }
    void push_back(const PCParseExpression& expr) { _items.push_back(expr); }

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(Array)

private:
    VECTOR_THREAD_LOCAL(Parser, PCParseExpression) _items;
};
//----------------------------------------------------------------------------
inline Parser::Array *MakeArray(
    const MemoryView<const PCParseExpression>& items,
    const Lexer::Location& site) {
    return new Parser::Array(items, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Dictionary : public ParseExpression {
public:
    explicit Dictionary(const Lexer::Location& site);
    Dictionary(const MemoryView<const Core::Pair<PCParseExpression, PCParseExpression>>& items, const Lexer::Location& site);
    virtual ~Dictionary();

    size_t size() const { return _items.size(); }
    bool empty() const { return _items.empty(); }
    void reserve(size_t capacity) { return _items.reserve(capacity); }
    void insert(const PCParseExpression& key, const PCParseExpression& value) { _items.Insert_AssertUnique(key, value); }

    virtual RTTI::MetaAtom *Eval(ParseContext *context) const override;
    virtual String ToString() const override;

    SINGLETON_POOL_ALLOCATED_DECL(Dictionary)

private:
    ASSOCIATIVE_VECTOR_THREAD_LOCAL(Parser, PCParseExpression, PCParseExpression) _items;
};
//----------------------------------------------------------------------------
inline Parser::Dictionary *MakeDictionary(
    const MemoryView<const Core::Pair<PCParseExpression, PCParseExpression>>& items,
    const Lexer::Location& site) {
    return new Parser::Dictionary(items, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core

#include "Core/Parser/ParseExpression-inl.h"
