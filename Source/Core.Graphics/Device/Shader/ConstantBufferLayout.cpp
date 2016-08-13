#include "stdafx.h"

#include "ConstantBufferLayout.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Container/Hash.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, ConstantBufferLayout, );
//----------------------------------------------------------------------------
ConstantBufferLayout::ConstantBufferLayout() : _frozen(false) {}
//----------------------------------------------------------------------------
ConstantBufferLayout::~ConstantBufferLayout() {}
//----------------------------------------------------------------------------
ConstantBufferLayout& ConstantBufferLayout::operator =(const ConstantBufferLayout& other) {
    Assert(not _frozen);
    _block = other._block;
    return *this;
}
//----------------------------------------------------------------------------
void ConstantBufferLayout::Freeze() {
    Assert(!_frozen);
    _frozen = true;
}
//----------------------------------------------------------------------------
void ConstantBufferLayout::AddField(const Name& name, ValueType type) {
    Assert(!_frozen);
    Assert(!name.empty());

    size_t offset = 0;
    if (_block.size()) {
        const ValueBlock::Field& pred = _block.MakeView().back();
        offset = pred.Offset();
        offset += pred.SizeInBytes();
    }

    const size_t sizeInBytes = ValueSizeInBytes(type);
    if ((offset & 15) + sizeInBytes > 16)
        offset = ROUND_TO_NEXT_16(offset);

    _block.Add(name, type, offset);
}
//----------------------------------------------------------------------------
void ConstantBufferLayout::AddField(const Name& name, ValueType type, size_t offset, size_t size, bool inUse/* = true */) {
    UNUSED(size);
    Assert(!_frozen);
    Assert(!name.empty());
    AssertRelease(size == ValueSizeInBytes(type));

    Assert(_block.empty() ||
        (_block.back().Offset() + ValueSizeInBytes(_block.back().Type()) <= offset) );

    _block.Add(name, type, offset, 0/* defaulted index */, inUse);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
