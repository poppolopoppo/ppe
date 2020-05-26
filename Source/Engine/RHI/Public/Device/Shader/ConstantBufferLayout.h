#pragma once

#include "Graphics.h"

#include "Name.h"
#include "ValueBlock.h"

#include "Allocator/PoolAllocator.h"
#include "Container/Stack.h"
#include "Memory/RefPtr.h"
#include "Meta/BitField.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ConstantBufferLayout);
class FConstantBufferLayout : public FRefCountable {
public:
    FConstantBufferLayout();
    ~FConstantBufferLayout();

    FConstantBufferLayout(const FConstantBufferLayout& other) { operator =(other); }
    FConstantBufferLayout& operator =(const FConstantBufferLayout& other);

    bool Frozen() const { return _frozen; }
    void Freeze();

    size_t size() const { return _block.size(); }
    bool empty() const { return _block.empty(); }

    size_t SizeInBytes() const { return _block.SizeInBytes(); }

    TMemoryView<const FValueField> Fields() const { return _block.MakeView(); }

    const FValueBlock& Block() const { return _block; }

    void AddField(const FName& name, EValueType type);
    void AddField(const FName& name, EValueType type, size_t offset, size_t size, bool inUse = true);

    template <typename T>
    void AddField(const FName& name) {
        STATIC_ASSERT(EValueType::Void != TValueTraits<T>::EType);
        AddField(name, TValueTraits<T>::EType);
    }

    bool Equals(const FConstantBufferLayout& other) const { return (other._block == _block); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FValueBlock _block;
    bool _frozen;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
