#pragma once

#include "RTTI_fwd.h"

#include "RTTI/AtomVisitor.h"

#include "Container/SparseArray.h"
#include "Misc/Function.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RTTI_API FBaseReferenceCollector : private FBaseAtomVisitor {
public:
    virtual ~FBaseReferenceCollector() = default;

    EObjectFlags ObjectFlags() const { return _objectFlags; }
    size_t NumReferences() const { return _numReferences; }

protected:
    FBaseReferenceCollector() NOEXCEPT;
    explicit FBaseReferenceCollector(EVisitorFlags flags) NOEXCEPT;

    void Collect(const FMetaObject& root);
    void Collect(const TMemoryView<const PMetaObject>& roots);

    virtual bool Visit(const IScalarTraits* scalar, PMetaObject& pobj) override;

private:
    EObjectFlags _objectFlags{ 0 };
    size_t _numReferences{ 0 };
};
//----------------------------------------------------------------------------
class PPE_RTTI_API FDirectReferenceCollector : public FBaseReferenceCollector {
public:
    using FReferences = SPARSEARRAY_INSITU(MetaObject, SMetaObject);

    FDirectReferenceCollector() NOEXCEPT;
    explicit FDirectReferenceCollector(EVisitorFlags flags) NOEXCEPT;

    void Collect(const FMetaObject& root, FReferences* refs);
    void Collect(const TMemoryView<const PMetaObject>& roots, FReferences* refs);

protected:
    virtual bool Visit(const IScalarTraits* scalar, PMetaObject& pobj) override final;

private:
    using FBaseReferenceCollector::Collect; // make parent collect private

    FReferences* _refs{ nullptr };
};
//----------------------------------------------------------------------------
class PPE_RTTI_API FLambdaReferenceCollector : public FBaseReferenceCollector {
public:
    using FOnReference = TFunction<bool(const IScalarTraits&, FMetaObject&)>;

    FLambdaReferenceCollector() = default;
    explicit FLambdaReferenceCollector(EVisitorFlags flags) NOEXCEPT
        : FBaseReferenceCollector(flags)
    {}

    void Collect(const FMetaObject& root, FOnReference&& prefix, FOnReference&& postfix = NoFunction);
    void Collect(const TMemoryView<const PMetaObject>& roots, FOnReference&& prefix, FOnReference&& postfix = NoFunction);

protected:
    virtual bool Visit(const IScalarTraits* scalar, PMetaObject& pobj) override final;

private:
    using FBaseReferenceCollector::Collect; // make parent collect private

    FOnReference _prefix;
    FOnReference _postfix;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
