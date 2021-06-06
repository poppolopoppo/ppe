#pragma once

#include "Serialize.h"

#include "RTTI/Atom.h"
#include "RTTI/NativeTypes.h"
#include "MetaObject.h"

#include "Parser/Parser.h"
#include "Parser/ParseItem.h"

#include "Allocator/SlabAllocator.h"
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
class PPE_SERIALIZE_API FParseExpression : public FParseItem, Meta::FNonCopyableNorMovable {
public:
    FParseExpression(const Lexer::FSpan& site);
    virtual ~FParseExpression();

    virtual FStringView Alias() const = 0;
    virtual RTTI::FAtom Eval(FParseContext *context) const = 0;
    virtual void Invoke(FParseContext *context) const override { Eval(context); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TLiteral : public FParseExpression {
public:
    TLiteral(T&& rvalue, const Lexer::FSpan& site);
    TLiteral(const T& value, const Lexer::FSpan& site) : TLiteral(T(value), site) {}
    virtual ~TLiteral();

    virtual FStringView Alias() const override final { return "Literal"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    T _literal;
};
//----------------------------------------------------------------------------
template <typename T>
auto MakeLiteral(T&& rvalue, const Lexer::FSpan& site) {
    return NEW_REF(Parser, TLiteral< Meta::TDecay<T> >, std::forward<T>(rvalue), site);
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

    explicit FVariableExport(const RTTI::FName& name, const PCParseExpression& value, const EFlags scope, const Lexer::FSpan& site);
    virtual ~FVariableExport();

    virtual FStringView Alias() const override final { return "VariableExport"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    RTTI::FName _name;
    PCParseExpression _value;
    EFlags _scope;
};
//----------------------------------------------------------------------------
inline TRefPtr<FVariableExport> MakeVariableExport(const RTTI::FName& name, const PCParseExpression& value, const FVariableExport::EFlags scope, const Lexer::FSpan& site) {
    return NEW_REF(Parser, FVariableExport, name, value, scope, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FVariableReference : public FParseExpression {
public:
    FVariableReference(const RTTI::FPathName& pathName, const Lexer::FSpan& site);
    virtual ~FVariableReference();

    virtual FStringView Alias() const override final { return "VariableReference"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    RTTI::FPathName _pathName;

};
//----------------------------------------------------------------------------
inline TRefPtr<FVariableReference> MakeVariableReference(const RTTI::FPathName& pathName, const Lexer::FSpan& site) {
    return NEW_REF(Parser, FVariableReference, pathName, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class TUnaryFunction : public FParseExpression {
public:
    explicit TUnaryFunction(FString&& symbol, _Functor&& functor, const FParseExpression *expr, const Lexer::FSpan& site);
    virtual ~TUnaryFunction();

    virtual FStringView Alias() const override final { return "UnaryFunction"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    FString _symbol;
    _Functor _functor;
    PCParseExpression _expr;
};
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<TUnaryFunction<T>> MakeUnaryFunction(const FStringView& symbol, T&& functor, const FParseExpression* expr, const Lexer::FSpan& site) {
    return NEW_REF(Parser, TUnaryFunction<T>, FString(symbol), std::move(functor), expr, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Functor>
class TBinaryFunction : public FParseExpression {
public:
    explicit TBinaryFunction(FString&& symbol, _Functor&& functor, const FParseExpression *lhs, const FParseExpression *rhs, const Lexer::FSpan& site);
    virtual ~TBinaryFunction();

    virtual FStringView Alias() const override final { return "BinaryFunction"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    FString _symbol;
    _Functor _functor;
    PCParseExpression _lhs;
    PCParseExpression _rhs;
};
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<TBinaryFunction<T>> MakeBinaryFunction(const FStringView& symbol, T&& functor, const FParseExpression *lhs, const FParseExpression *rhs, const Lexer::FSpan& site) {
    return NEW_REF(Parser, TBinaryFunction<T>, FString(symbol), std::move(functor), lhs, rhs, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Test>
class TTernary : public FParseExpression {
public:
    explicit TTernary(_Test&& test, const FParseExpression *pif, const FParseExpression *ptrue, const FParseExpression *pfalse, const Lexer::FSpan& site);
    virtual ~TTernary();

    virtual FStringView Alias() const override final { return "Ternary"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    _Test _test;
    PCParseExpression _if;
    PCParseExpression _true;
    PCParseExpression _false;
};
//----------------------------------------------------------------------------
template <typename _Test>
TRefPtr<TTernary<_Test>> MakeTernary(_Test&& test, const FParseExpression *pif, const FParseExpression *ptrue, const FParseExpression *pfalse, const Lexer::FSpan& site) {
    return NEW_REF(Parser, TTernary<_Test>, std::move(test), pif, ptrue, pfalse, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FObjectDefinition : public FParseExpression {
public:
    FObjectDefinition(const RTTI::FName& name, const Lexer::FSpan& site);
    virtual ~FObjectDefinition();

    void AddStatement(const FParseStatement *statement);

    template <typename _It>
    void AddStatements(_It&& begin, _It&& end) {
        _statements.insert(_statements.end(), begin, end);
    }

    virtual FStringView Alias() const override final { return "ObjectDefinition"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    RTTI::FName _name;
    VECTORINSITU(Parser, PCParseStatement, 3) _statements;
};
//----------------------------------------------------------------------------
template <typename _It>
inline TRefPtr<FObjectDefinition> MakeObjectDefinition(
    const RTTI::FName& name,
    const Lexer::FSpan& site,
    _It&& statementsBegin, _It&& statementsEnd) {
    TRefPtr<FObjectDefinition> def = NEW_REF(Parser, FObjectDefinition, name, site);
    def->AddStatements(statementsBegin, statementsEnd);
    return def;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FPropertyReference : public FParseExpression {
public:
    FPropertyReference(PCParseExpression&& object, const RTTI::FName& member, const Lexer::FSpan& site);
    virtual ~FPropertyReference();

    virtual FStringView Alias() const override final { return "PropertyReference"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    PCParseExpression _object;
    RTTI::FName _member;
};
//----------------------------------------------------------------------------
inline TRefPtr<FPropertyReference> MakePropertyReference(
    PCParseExpression&& object,
    const RTTI::FName& member,
    const Lexer::FSpan& site) {
    return NEW_REF(Parser, FPropertyReference, std::move(object), member, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTupleExpr : public FParseExpression {
public:
    using elements_type = VECTORINSITU(Parser, PCParseExpression, 4);

    explicit FTupleExpr(const Lexer::FSpan& site);
    FTupleExpr(const TMemoryView<PCParseExpression>& elts, const Lexer::FSpan& site);
    virtual ~FTupleExpr();

    virtual FStringView Alias() const override final { return "TupleExpr"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    elements_type _elements;
};
//----------------------------------------------------------------------------
inline TRefPtr<FTupleExpr> MakeTupleExpr(const TMemoryView<PCParseExpression>& elts, const Lexer::FSpan& site) {
    return NEW_REF(Parser, FTupleExpr, elts, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FArrayExpr : public FParseExpression {
public:
    using items_type = VECTORINSITU(Parser, PCParseExpression, 4);

    explicit FArrayExpr(const Lexer::FSpan& site);
    FArrayExpr(const TMemoryView<PCParseExpression>& elts, const Lexer::FSpan& site);
    virtual ~FArrayExpr();

    size_t size() const { return _items.size(); }
    bool empty() const { return _items.empty(); }
    void reserve(size_t capacity) { return _items.reserve(capacity); }
    void push_back(const PCParseExpression& expr) { _items.push_back(expr); }

    virtual FStringView Alias() const override final { return "ArrayExpr"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    items_type _items;
};
//----------------------------------------------------------------------------
inline TRefPtr<FArrayExpr> MakeArrayExpr(const Lexer::FSpan& site) {
    return NEW_REF(Parser, FArrayExpr, site);
}
//----------------------------------------------------------------------------
inline TRefPtr<FArrayExpr> MakeArrayExpr(const TMemoryView<PCParseExpression>& elts, const Lexer::FSpan& site) {
    return NEW_REF(Parser, FArrayExpr, elts, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FDictionaryExpr : public FParseExpression {
public:
    using dico_type = ASSOCIATIVE_VECTORINSITU(Parser, PCParseExpression, PCParseExpression, 2);

    explicit FDictionaryExpr(const Lexer::FSpan& site);
    FDictionaryExpr(dico_type&& rdico, const Lexer::FSpan& site);
    virtual ~FDictionaryExpr();

    size_t size() const { return _dico.size(); }
    bool empty() const { return _dico.empty(); }
    void reserve(size_t capacity) { return _dico.reserve(capacity); }
    void insert(const PCParseExpression& key, const PCParseExpression& value) { _dico.Insert_AssertUnique(key, value); }

    virtual FStringView Alias() const override final { return "DictionaryExpr"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    dico_type _dico;
};
//----------------------------------------------------------------------------
inline TRefPtr<FDictionaryExpr> MakeDictionaryExpr(const Lexer::FSpan& site) {
    return NEW_REF(Parser, FDictionaryExpr, site);
}
//----------------------------------------------------------------------------
inline TRefPtr<FDictionaryExpr> MakeDictionaryExpr(FDictionaryExpr::dico_type&& ritems, const Lexer::FSpan& site) {
    return NEW_REF(Parser, FDictionaryExpr, std::move(ritems), site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FCastExpr : public FParseExpression {
public:
    FCastExpr(const RTTI::PTypeTraits& traits, const FParseExpression* expr, const Lexer::FSpan& site);
    virtual ~FCastExpr();

    virtual FStringView Alias() const override final { return "CastExpr"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    RTTI::PTypeTraits _traits;
    PCParseExpression _expr;
};
//----------------------------------------------------------------------------
inline TRefPtr<FCastExpr> MakeCastExpr(const RTTI::PTypeTraits& traits, const FParseExpression* expr, const Lexer::FSpan& site) {
    return NEW_REF(Parser, FCastExpr, traits, expr, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FFunctionCall : public FParseExpression {
public:
    using args_type = VECTORINSITU(Parser, PCParseExpression, 4);

    FFunctionCall(PCParseExpression&& obj, const RTTI::FName& funcname, const TMemoryView<PCParseExpression>& args, const Lexer::FSpan& site);
    virtual ~FFunctionCall();

    virtual FStringView Alias() const override final { return "FunctionCall"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    PCParseExpression _obj;
    RTTI::FName _funcname;
    args_type _args;
};
//----------------------------------------------------------------------------
inline TRefPtr<FFunctionCall> MakeFunctionCall(PCParseExpression&& obj, const RTTI::FName& funcname, const TMemoryView<PCParseExpression>& args, const Lexer::FSpan& site) {
    return NEW_REF(Parser, FFunctionCall, std::move(obj), funcname, args, site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FSubscriptOperator : public FParseExpression {
public:
    FSubscriptOperator(PCParseExpression&& lvalue, PCParseExpression&& subscript, const Lexer::FSpan& site);
    virtual ~FSubscriptOperator();

    virtual FStringView Alias() const override final { return "SubscriptOperator"; }
    virtual RTTI::FAtom Eval(FParseContext *context) const override final;
    virtual FString ToString() const override final;

private:
    PCParseExpression _lvalue;
    PCParseExpression _subscript;

    size_t WrapIndexAround_(RTTI::FAtom subscript, size_t count) const;

    RTTI::FAtom Subscript_Scalar_(RTTI::FAtom lvalue, RTTI::FAtom subscript) const;
    RTTI::FAtom Subscript_Object_(RTTI::FAtom lvalue, RTTI::FAtom subscript) const;
    RTTI::FAtom Subscript_Tuple_(RTTI::FAtom lvalue, RTTI::FAtom subscript) const;
    RTTI::FAtom Subscript_List_(RTTI::FAtom lvalue, RTTI::FAtom subscript) const;
    RTTI::FAtom Subscript_Dico_(RTTI::FAtom lvalue, RTTI::FAtom subscript) const;
};
//----------------------------------------------------------------------------
inline TRefPtr<FSubscriptOperator> MakeSubscriptOperator(PCParseExpression&& lvalue, PCParseExpression&& subscript, const Lexer::FSpan& site) {
    return NEW_REF(Parser, FSubscriptOperator, std::move(lvalue), std::move(subscript), site);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE

#include "Parser/ParseExpression-inl.h"
