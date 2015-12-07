#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Stack.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ConstantBufferLayout);
class ConstantBufferLayout : public RefCountable {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxFieldCount, 16);

    explicit ConstantBufferLayout(size_t sizeInBytes = 0);
    ~ConstantBufferLayout();

    ConstantBufferLayout(const ConstantBufferLayout& other) { operator =(other); }
    ConstantBufferLayout& operator =(const ConstantBufferLayout& other);

    bool Frozen() const { return bitfrozen_type::Get(_data); }
    size_t Count() const { return bitcount_type::Get(_data); }
    size_t SizeInBytes() const { return bitsizeinbytes_type::Get(_data); }

    MemoryView<const BindName> Names() const { return MakeView(_names, _names + Count()); }
    MemoryView<const ConstantField> Fields() const { return MakeView(_fields, _fields + Count()); }

    void AddField(const BindName& name, ConstantFieldType type);
    void AddField(const BindName& name, ConstantFieldType type, size_t offset, size_t size, bool inUse);

    template <typename T>
    void AddField(const BindName& name) {
        STATIC_ASSERT(ConstantFieldTraits<T>::Enabled);
        AddField(name, ConstantFieldTraits<T>::Type);
    }

    size_t HashValue() const;
    bool Equals(const ConstantBufferLayout& other) const;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    typedef Meta::Bit<size_t>::First<1>::type bitfrozen_type;
    typedef Meta::Bit<size_t>::After<bitfrozen_type>::Field<4>::type bitcount_type;
    typedef Meta::Bit<size_t>::After<bitcount_type>::Remain::type bitsizeinbytes_type;

    size_t _data;
    BindName _names[MaxFieldCount];
    ConstantField _fields[MaxFieldCount];
};
//----------------------------------------------------------------------------
inline hash_t hash_value(const ConstantBufferLayout& layout) {
    return layout.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
