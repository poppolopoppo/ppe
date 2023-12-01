// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MetaObjectHelpers.h"

#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/ReferenceCollector.h"
#include "RTTI/TypeTraits.h"
#include "RTTI/Exceptions.h"

#include "MetaClass.h"
#include "MetaModule.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"

#include "Container/FixedSizeHashTable.h"
#include "Container/Hash.h"
#include "Container/Stack.h"
#include "Container/Vector.h"
#include "Memory/HashFunctions.h"
#include "Meta/PointerWFlags.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Equals(const FMetaObject& lhs, const FMetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);

    if (rhs.RTTI_Class() != metaClass)
        return false;

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        if (not lhsValue.Equals(rhsValue))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool DeepEquals(const FMetaObject& lhs, const FMetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);

    if (rhs.RTTI_Class() != metaClass)
        return false;

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        if (not lhsValue.DeepEquals(rhsValue))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Move(dstValue);
    }
}
//----------------------------------------------------------------------------
void Copy(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Copy(dstValue);
    }
}
//----------------------------------------------------------------------------
void Swap(FMetaObject& lhs, FMetaObject& rhs) {
    if (&lhs == &rhs)
        return;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);
    Assert(rhs.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        lhsValue.SwapValue(rhsValue);
    }
}
//----------------------------------------------------------------------------
void Clone(const FMetaObject& src, PMetaObject& pdst) {
    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);

    if (not metaClass->CreateInstance(pdst))
        AssertNotReached();

    FMetaObject& dst = *pdst;
    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Copy(dstValue);
    }
}
//----------------------------------------------------------------------------
void DeepCopy(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        Verify( srcValue.DeepCopy(dstValue) );
    }
}
//----------------------------------------------------------------------------
void DeepClone(const FMetaObject& src, PMetaObject& pdst) {
    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);

    if (not metaClass->CreateInstance(pdst))
        AssertNotReached();

    FMetaObject& dst = *pdst;
    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        Verify( srcValue.DeepCopy(dstValue) );
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ResetToDefaultValue(FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties())
        prop->ResetToDefaultValue(obj);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    hash_t h(PPE_HASH_VALUE_SEED);
    for (const FMetaProperty* prop : metaClass->AllProperties())
        hash_combine(h, prop->Get(obj));

    return h;
}
//----------------------------------------------------------------------------
u128 Fingerprint128(const FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    STACKLOCAL_POD_STACK(hash_t, hashValues, metaClass->NumProperties(true));
    for (const FMetaProperty* prop : metaClass->AllProperties())
        hashValues.Push(prop->Get(obj).HashValue());

    return PPE::Fingerprint128(hashValues.MakeConstView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RTTI_CHECKS
void CheckMetaClassAllocation(const FMetaClass* metaClass) {
    if (nullptr == metaClass) {
        PPE_LOG(RTTI, Error, "trying to allocate with a null metaclass");
        PPE_THROW_IT(FClassException("null metaclass", metaClass));
    }

    if (metaClass->IsAbstract()) {
        PPE_LOG(RTTI, Error, "can't allocate a new object with abstract metaclass \"{0}\" ({1})",
            metaClass->Name(),
            metaClass->Flags());
        PPE_THROW_IT(FClassException("abstract metaclass", metaClass));
    }

    if (metaClass->IsDeprecated()) {
        PPE_LOG(RTTI, Warning, "allocate a new object using deprecated metaclass \"{0}\" ({1})",
            metaClass->Name(),
            metaClass->Flags() );
    }
}
#endif //!USE_PPE_RTTI_CHECKS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t CollectReferences(
    const FMetaObject& root,
    TFunction<bool(const ITypeTraits&, FMetaObject&)>&& prefix,
    TFunction<bool(const ITypeTraits&, FMetaObject&)>&& postfix,
    EVisitorFlags flags) {
    FLambdaReferenceCollector collector{ flags };
    collector.Collect(root, std::move(prefix), std::move(postfix));
    return collector.NumReferences();
}
//----------------------------------------------------------------------------
size_t CollectReferences(
    const TMemoryView<const PMetaObject>& roots,
    TFunction<bool(const ITypeTraits&, FMetaObject&)>&& prefix,
    TFunction<bool(const ITypeTraits&, FMetaObject&)>&& postfix,
    EVisitorFlags flags) {
    FLambdaReferenceCollector collector{ flags };
    collector.Collect(roots, std::move(prefix), std::move(postfix));
    return collector.NumReferences();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FCheckCircularReferences_ : private FBaseReferenceCollector {
public:
    FCheckCircularReferences_() NOEXCEPT
    :   FBaseReferenceCollector(
            EVisitorFlags::KeepDeprecated +
            EVisitorFlags::KeepTransient )
    ,   _circular(false)
    {}

    bool Check(const FMetaObject& root) {
        _circular = false;
        FBaseReferenceCollector::Collect(root);
        Assert_NoAssume(_chain.empty());
        return (not _circular);
    }

    bool Check(const TMemoryView<const PMetaObject>& roots) {
        _circular = false;
        FBaseReferenceCollector::Collect(roots);
        Assert_NoAssume(_chain.empty());
        return (not _circular);
    }

protected:
    virtual bool Visit(const ITupleTraits* tuple, void* data) override final {
        if (ShouldSkipTraits(this, *tuple))
            return true;

        size_t index = 0;
        return tuple->ForEach(data, [this, &index](const FAtom& elt) {
            static const FName GTuple{ "Tuple" };

            const FMarker_ markerIt{
                MARK_Struct,
                elt.Traits(),
                _chain.back()->Name,
                GTuple,
                index, elt.Data() };
            _chain.emplace_back(&markerIt);

            const bool result = elt.Accept(this);

            ++index;
            Verify(_chain.pop_back_ReturnBack() == &markerIt);

            return result;
        });
    }

    virtual bool Visit(const IListTraits* list, void* data) override final {
        if (ShouldSkipTraits(this, *list))
            return true;

        size_t index = 0;
        return list->ForEach(data, [this, &index](const FAtom& item) {
            static const FName GList{ "List" };

            const FMarker_ markerIt{
                MARK_Container,
                item.Traits(),
                _chain.back()->Name,
                GList,
                index, item.Data() };
            _chain.emplace_back(&markerIt);

            const bool result = item.Accept(this);

            ++index;
            Verify(_chain.pop_back_ReturnBack() == &markerIt);

            return result;
        });
    }

    virtual bool Visit(const IDicoTraits* dico, void* data) override final {
        if (ShouldSkipTraits(this, *dico))
            return true;

        size_t index = 0;
        return dico->ForEach(data, [this, &index](const FAtom& key, const FAtom& value) {
            bool result = true;

            if (not ShouldSkipTraits(this, *key.Traits())) {
                static const FName GKey{ "Key" };

                const FMarker_ markerIt{
                    MARK_Container,
                    key.Traits(),
                    _chain.back()->Name,
                    GKey,
                    index, key.Data() };
                _chain.emplace_back(&markerIt);

                result &= key.Accept(this);

                Verify(_chain.pop_back_ReturnBack() == &markerIt);
            }

            if (not ShouldSkipTraits(this, *value.Traits())) {
                static const FName GValue{ "Value" };

                const FMarker_ markerIt{
                    MARK_Container,
                    value.Traits(),
                    _chain.back()->Name,
                    GValue,
                    index, value.Data() };
                _chain.emplace_back(&markerIt);

                result &= value.Accept(this);

                Verify(_chain.pop_back_ReturnBack() == &markerIt);
            }

            ++index;
            return result;
        });
    }

    virtual bool Visit(const IScalarTraits*, PMetaObject& pobj) override final {
        bool result = true;
        if (const FMetaObject * const ref = pobj.get()) {
            const FMarker_ markerObj{
                MARK_Object,
                ref->RTTI_Traits(),
                ref->RTTI_Outer() ? ref->RTTI_Outer()->Namespace() : ref->RTTI_Class()->Module()->Name(),
                ref->RTTI_Name(),
                size_t(0), ref };
            _chain.emplace_back(&markerObj);

            if (not _visiteds.Emplace_KeepExisting(ref)) {
                OnCircularReference_(ref);
            }
            else {
                const FMetaClass* const klass = ref->RTTI_Class();

                size_t index = 0;
                for (const FMetaProperty* prop : klass->AllProperties()) {
                    if (not ShouldSkipTraits(this, *prop->Traits())) {
                        const FAtom p = prop->Get(*ref);

                        const FMarker_ markerProp{
                            MARK_Property,
                            p.Traits(),
                            klass->Name(),
                            prop->Name(),
                            index, p.Data() };
                        _chain.emplace_back(&markerProp);

                        result &= p.Accept(this);

                        Verify(_chain.pop_back_ReturnBack() == &markerProp);
                    }
                    ++index;
                }

                _visiteds.Remove_AssertExists(ref);
            }

            Verify(_chain.pop_back_ReturnBack() == &markerObj);
        }
        return result;
    }

private:
    enum EMarker {
        MARK_Object = 0,
        MARK_Property,
        MARK_Container,
        MARK_Struct,
    };

    struct FMarker_ {
        PTypeTraits Traits;
        FName Outer;
        FName Name;
        size_t Index;
        Meta::FPointerWFlags Ref;

        FMarker_(EMarker type, const PTypeTraits& traits, const FName& outer, const FName& name, size_t index, const void* ref)
        :   Traits(traits)
        ,   Outer(outer)
        ,   Name(name)
        ,   Index(index) {
            Ref.Reset(ref, checked_cast<uintptr_t>(type));
        }

    };

    bool _circular;

    VECTORINSITU(MetaObject, const FMarker_*, 16) _chain;
    TFixedSizeHashSet<const FMetaObject*, 32> _visiteds;

    void OnCircularReference_(const void* ref) {
        Unused(ref);
        _circular = true;

        PPE_LOG_DIRECT(RTTI, Warning, [this, ref](FTextWriter& oss) {
            oss << "found a circular reference !" << Eol;

            Fmt::FIndent indent = Fmt::FIndent::TwoSpaces();

            forrange(i, 0, _chain.size()) {
                const FMarker_& mark = (*_chain[i]);

                Format(oss, "[{0:#2}]", i);
                oss << indent;

                if (mark.Ref.Get() == ref)
                    oss << " -=> ";
                else
                    oss << " --- ";

                CONSTEXPR const FStringView typenames[] = {
                    "OBJECT   ",
                    "PROPERTY ",
                    "CONTAINER",
                    "STRUCT   "
                };

                oss << Fmt::Pointer(mark.Ref.Get())
                    << ' ' << typenames[mark.Ref.Flag01()]
                    << " >>> " << mark.Outer
                    << " . " << mark.Name
                    << " [ " << mark.Index
                    << " ]  (" << mark.Traits->NamedTypeInfos()
                    << ")"
                    << Eol;

                indent.Inc();
            }
        });
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool CheckCircularReferences(const FMetaObject& root) {
    FCheckCircularReferences_ circular;
    return circular.Check(root);
}
//----------------------------------------------------------------------------
bool CheckCircularReferences(const TMemoryView<const PMetaObject>& roots) {
    FCheckCircularReferences_ circular;
    return circular.Check(roots);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FMetaObject& obj) {
    RTTI::PMetaObject pobj(const_cast<RTTI::FMetaObject*>(&obj));
    PrettyPrint(oss, RTTI::FAtom::FromObj(pobj));
    RemoveRef_AssertAlive(pobj);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaObject& obj) {
    RTTI::PMetaObject pobj(const_cast<RTTI::FMetaObject*>(&obj));
    PrettyPrint(oss, RTTI::FAtom::FromObj(pobj));
    RemoveRef_AssertAlive(pobj);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
