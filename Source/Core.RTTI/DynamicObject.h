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
class DynamicProperty : public MetaProperty {
public:
    explicit DynamicProperty(const MetaPropertyName& name);
    virtual ~DynamicProperty();
    /*
    virtual MetaTypeInfo TypeInfo() const override;
    virtual const IMetaTypeVirtualTraits *Traits() const override;

    virtual bool IsDefaultValue(const MetaObject *object) const override;

    virtual MetaAtom *WrapMove(MetaObject *src) const override;
    virtual MetaAtom *WrapCopy(const MetaObject *src) const override;

    virtual bool UnwrapMove(MetaObject *dst, MetaAtom *src) const override;
    virtual bool UnwrapCopy(MetaObject *dst, const MetaAtom *src) const override;

    virtual void MoveTo(MetaObject *object, MetaAtom *atom) const override;
    virtual void CopyTo(const MetaObject *object, MetaAtom *atom) const override;

    virtual void MoveFrom(MetaObject *object, MetaAtom *atom) const override;
    virtual void CopyFrom(MetaObject *object, const MetaAtom *atom) const override;

    virtual void Move(MetaObject *dst, MetaObject *src) const override;
    virtual void Copy(MetaObject *dst, const MetaObject *src) const override;

    virtual void Swap(MetaObject *lhs, MetaObject *rhs) const override;

    virtual bool Equals(const MetaObject *lhs, const MetaObject *rhs) const override;
    virtual bool DeepEquals(const MetaObject *lhs, const MetaObject *rhs) const override;

    virtual void *RawPtr(MetaObject *obj) const override;
    virtual const void *RawPtr(const MetaObject *obj) const override;

    virtual size_t HashValue(const MetaObject *object) const override;
    */
    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
class DynamicObject : public MetaObject {
public:
    DynamicObject();
    virtual ~DynamicObject();

    MetaAtom* GetValue(const MetaPropertyName& name);
    const MetaAtom* GetValue(const MetaPropertyName& name) const;

    MetaAtom* TryGetValue(const MetaPropertyName& name);
    const MetaAtom* TryGetValue(const MetaPropertyName& name) const;

    void SetValue(const MetaPropertyName& name, const PMetaAtom& value);

    void ClearValues();

    virtual const RTTI::MetaClass *RTTI_MetaClass() const override;

    SINGLETON_POOL_ALLOCATED_DECL();

    class MetaClass : public RTTI::InScopeMetaClass {
    public:
        typedef MetaObject object_type;
        typedef void parent_type;

        MetaClass();
        virtual ~MetaClass();

        static void Create();
        static void Destroy();

        static bool HasInstance();
        static const MetaClass *Instance();

    protected:
        //virtual const MetaProperty *VirtualPropertyIFP(const char *name, size_t attributes) const override;
        //virtual const MetaProperty *VirtualPropertyIFP(const MetaPropertyName& name, size_t attributes) const override;

        virtual const RTTI::MetaClass* VirtualParent() const override;
        virtual MetaObject* VirtualCreateInstance() const override;
    };

private:
    MetaClass _metaClass;
    ASSOCIATIVE_VECTOR(RTTI, MetaPropertyName, PMetaAtom) _values;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
