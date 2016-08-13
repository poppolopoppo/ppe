#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Name.h"
#include "Core.Graphics/ValueBlock.h"

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
    ConstantBufferLayout();
    ~ConstantBufferLayout();

    ConstantBufferLayout(const ConstantBufferLayout& other) { operator =(other); }
    ConstantBufferLayout& operator =(const ConstantBufferLayout& other);

    bool Frozen() const { return _frozen; }
    void Freeze();

    size_t size() const { return _block.size(); }
    bool empty() const { return _block.empty(); }

    size_t SizeInBytes() const { return _block.SizeInBytes(); }

    MemoryView<const ValueBlock::Field> Fields() const { return _block.MakeView(); }

    const ValueBlock& Block() const { return _block; }

    void AddField(const Name& name, ValueType type);
    void AddField(const Name& name, ValueType type, size_t offset, size_t size, bool inUse = true);

    template <typename T>
    void AddField(const Name& name) {
        STATIC_ASSERT(ValueType::Void != ValueTraits<T>::Type);
        AddField(name, ValueTraits<T>::Type);
    }

    bool Equals(const ConstantBufferLayout& other) const { return (other._block == _block); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    ValueBlock _block;
    bool _frozen;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
