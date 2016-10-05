#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(DynamicObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDynamicProperty : public FMetaProperty {
public:
    explicit FDynamicProperty(const FName& name);
    virtual ~FDynamicProperty();
    /*
    virtual FMetaTypeInfo TypeInfo() const override;
    virtual const IMetaTypeVirtualTraits *Traits() const override;

    virtual bool IsDefaultValue(const FMetaObject *object) const override;

    virtual FMetaAtom *WrapMove(FMetaObject *src) const override;
    virtual FMetaAtom *WrapCopy(const FMetaObject *src) const override;

    virtual bool UnwrapMove(FMetaObject *dst, FMetaAtom *src) const override;
    virtual bool UnwrapCopy(FMetaObject *dst, const FMetaAtom *src) const override;

    virtual void MoveTo(FMetaObject *object, FMetaAtom *atom) const override;
    virtual void CopyTo(const FMetaObject *object, FMetaAtom *atom) const override;

    virtual void MoveFrom(FMetaObject *object, FMetaAtom *atom) const override;
    virtual void CopyFrom(FMetaObject *object, const FMetaAtom *atom) const override;

    virtual void Move(FMetaObject *dst, FMetaObject *src) const override;
    virtual void Copy(FMetaObject *dst, const FMetaObject *src) const override;

    virtual void Swap(FMetaObject *lhs, FMetaObject *rhs) const override;

    virtual bool Equals(const FMetaObject *lhs, const FMetaObject *rhs) const override;
    virtual bool DeepEquals(const FMetaObject *lhs, const FMetaObject *rhs) const override;

    virtual void *RawPtr(FMetaObject *obj) const override;
    virtual const void *RawPtr(const FMetaObject *obj) const override;

    virtual size_t HashValue(const FMetaObject *object) const override;
    */
    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
class FDynamicObject : public FMetaObject {
public:
    FDynamicObject();
    virtual ~FDynamicObject();

    FMetaAtom* GetValue(const FName& name);
    const FMetaAtom* GetValue(const FName& name) const;

    FMetaAtom* TryGetValue(const FName& name);
    const FMetaAtom* TryGetValue(const FName& name) const;

    void SetValue(const FName& name, const PMetaAtom& value);

    void ClearValues();

    virtual const RTTI::FMetaClass *RTTI_MetaClass() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

    class FMetaClass : public RTTI::InScopeMetaClass {
    public:
        typedef FMetaObject object_type;
        typedef void parent_type;

        FMetaClass();
        virtual ~FMetaClass();

        static void Create();
        static void Destroy();

        static bool HasInstance();
        static const FMetaClass *Instance();

    protected:
        //virtual const FMetaProperty *VirtualPropertyIFP(const char *name, size_t attributes) const override;
        //virtual const FMetaProperty *VirtualPropertyIFP(const FName& name, size_t attributes) const override;

        virtual const RTTI::FMetaClass* VirtualParent() const override;
        virtual FMetaObject* VirtualCreateInstance() const override;
    };

private:
    FMetaClass _metaClass;
    ASSOCIATIVE_VECTOR(RTTI, FName, PMetaAtom) _values;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
