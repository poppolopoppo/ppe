// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ConstantBufferLayout.h"

#include "Allocator/PoolAllocator-impl.h"
#include "Container/Hash.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FConstantBufferLayout, );
//----------------------------------------------------------------------------
FConstantBufferLayout::FConstantBufferLayout() : _frozen(false) {}
//----------------------------------------------------------------------------
FConstantBufferLayout::~FConstantBufferLayout() {}
//----------------------------------------------------------------------------
FConstantBufferLayout& FConstantBufferLayout::operator =(const FConstantBufferLayout& other) {
    Assert(not _frozen);
    _block = other._block;
    return *this;
}
//----------------------------------------------------------------------------
void FConstantBufferLayout::Freeze() {
    Assert(!_frozen);
    _frozen = true;
}
//----------------------------------------------------------------------------
void FConstantBufferLayout::AddField(const FName& name, EValueType type) {
    Assert(!_frozen);
    Assert(!name.empty());

    size_t offset = 0;
    if (_block.size()) {
        const FValueField& pred = _block.MakeView().back();
        offset = pred.Offset();
        offset += pred.SizeInBytes();
    }

    const size_t sizeInBytes = ValueSizeInBytes(type);
    if ((offset & 15) + sizeInBytes > 16)
        offset = ROUND_TO_NEXT_16(offset);

    _block.Add(name, type, offset);
}
//----------------------------------------------------------------------------
void FConstantBufferLayout::AddField(const FName& name, EValueType type, size_t offset, size_t size, bool inUse/* = true */) {
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
} //!namespace PPE
