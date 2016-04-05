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
ConstantBufferLayout::ConstantBufferLayout(size_t sizeInBytes /* = 0 */)
:   _data(0) {
    bitsizeinbytes_type::InplaceSet(_data, sizeInBytes);
}
//----------------------------------------------------------------------------
ConstantBufferLayout::~ConstantBufferLayout() {}
//----------------------------------------------------------------------------
ConstantBufferLayout& ConstantBufferLayout::operator =(const ConstantBufferLayout& other) {
    _data = 0;
    const size_t count = other.Count();
    bitcount_type::InplaceSet(_data, count);
    bitsizeinbytes_type::InplaceSet(_data, other.SizeInBytes());

    Assert(count <= MaxFieldCount);
    for (size_t i = 0; i < count; ++i) {
        _names[i] = other._names[i];
        _fields[i] = other._fields[i];
    }

    return *this;
}
//----------------------------------------------------------------------------
void ConstantBufferLayout::AddField(const BindName& name, ConstantFieldType type) {
    Assert(!name.empty());

    const size_t count = Count();
    Assert(count < MaxFieldCount);

    size_t offset = 0;
    if (count) {
        const ConstantField& pred = _fields[count - 1];
        offset = pred.Offset();
        offset += static_cast<size_t>(ConstantFieldTypeSizeInBytes(pred.Type()));
    }

    const size_t sizeInBytes = static_cast<size_t>(ConstantFieldTypeSizeInBytes(type));
    if ((offset & 15) + sizeInBytes > 16)
        offset = ROUND_TO_NEXT_16(offset);

    _names[count] = name;
    _fields[count].Reset(true/* always used */, offset, type);

    bitcount_type::InplaceSet(_data, count + 1);
    bitsizeinbytes_type::InplaceSet(_data, ROUND_TO_NEXT_16(offset + sizeInBytes));
}
//----------------------------------------------------------------------------
void ConstantBufferLayout::AddField(const BindName& name, ConstantFieldType type, size_t offset, size_t size, bool inUse) {
    UNUSED(size);
    Assert(!name.empty());
    Assert(size == ConstantFieldTypeSizeInBytes(type));

    const size_t count = Count();
    Assert(count < MaxFieldCount);
    Assert(0 == count ||
        (_fields[count - 1].Offset() + ConstantFieldTypeSizeInBytes(_fields[count - 1].Type()) <= offset) );

    _names[count] = name;
    _fields[count].Reset(inUse, offset, type);

    Assert(_fields[count].Offset() + ConstantFieldTypeSizeInBytes(type) <= SizeInBytes());

    bitcount_type::InplaceSet(_data, count + 1);
}
//----------------------------------------------------------------------------
size_t ConstantBufferLayout::HashValue() const {
    const size_t count = Count();

    const size_t h0 = hash_range(&_names[0], count);
    const size_t h1 = hash_range(&_fields[0], count);

    return hash_tuple(_data, h0, h1);
}
//----------------------------------------------------------------------------
bool ConstantBufferLayout::Equals(const ConstantBufferLayout& other) const {
    if (this == &other)
        return true;

    if (other._data != _data)
        return false;

    const size_t count = Count();

    bool result = true;
    for (size_t i = 0; result && i < count; ++i) {
        result &= (other._names[i] == _names[i]);
        result &= (other._fields[i] == _fields[i]);
    }

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
