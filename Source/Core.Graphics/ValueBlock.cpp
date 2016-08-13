#include "stdafx.h"

#include "ValueBlock.h"

#include "Core/Memory/HashFunctions.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t ValueBlock::SizeInBytes() const {
#ifdef WITH_CORE_ASSERT
    size_t sizeInBytes = 0;
    for (const Field& field : _fields) {
        const size_t s = field.Offset() + ValueSizeInBytes(field.Type());
        Assert(s > sizeInBytes);
        Assert((s % sizeof(u32)) == 0);
        sizeInBytes = s;
    }
    return sizeInBytes;
#else
    if (_fields.empty())
    {
        return 0;
    }
    else
    {
        const Field& last = _fields.back();
        return last.Offset() + ValueSizeInBytes(last.Type());
    }
#endif
}
//----------------------------------------------------------------------------
void ValueBlock::Add(const Graphics::Name& name, ValueType type, size_t offset, size_t index/* = 0 */, bool inUse/* = true */) {
    Assert(!name.empty());
    Assert(ValueType::Void != type);
    Assert(SizeInBytes() <= offset);
    Assert(nullptr == FindByNameAndIndexIFP(name, index));

    _fields.emplace_back(name, type, offset, index, inUse);
}
//----------------------------------------------------------------------------
void ValueBlock::Clear() {
    _fields.clear();
}
//----------------------------------------------------------------------------
void ValueBlock::Copy(const MemoryView<u8>& dst, const MemoryView<const u8>& src) const {
    for (const Field& field : _fields)
        ValueCopy(field.Type(), dst.CutStartingAt(field.Offset()), src.CutStartingAt(field.Offset()));
}
//----------------------------------------------------------------------------
void ValueBlock::Defaults(const MemoryView<u8>& dst) const {
    for (const Field& field : _fields)
        ValueDefault(field.Type(), dst.CutStartingAt(field.Offset()));
}
//----------------------------------------------------------------------------
bool ValueBlock::Equals(const MemoryView<const u8>& lhs, const MemoryView<const u8>& rhs) const {
    for (const Field& field : _fields)
        if (not ValueEquals(field.Type(), lhs.CutStartingAt(field.Offset()), rhs.CutStartingAt(field.Offset())))
            return false;

    return true;
}
//----------------------------------------------------------------------------
const ValueBlock::Field& ValueBlock::FindByName(const Graphics::Name& name) const {
    const Field* field = FindByNameIFP(name);
    Assert(field);
    return *field;
}
//----------------------------------------------------------------------------
const ValueBlock::Field* ValueBlock::FindByNameIFP(const Graphics::Name& name) const {
    const auto it = std::find_if(_fields.begin(), _fields.end(), [=](const ValueBlock::Field& field) {
        return (field.Name() == name);
    });
    return (it != _fields.end() ? &*it : nullptr);
}
//----------------------------------------------------------------------------
const ValueBlock::Field& ValueBlock::FindByNameAndIndex(const Graphics::Name& name, size_t index) const {
    const Field* field = FindByNameAndIndexIFP(name, index);
    Assert(field);
    return *field;
}
//----------------------------------------------------------------------------
const ValueBlock::Field* ValueBlock::FindByNameAndIndexIFP(const Graphics::Name& name, size_t index) const {
    const auto it = std::find_if(_fields.begin(), _fields.end(), [=](const ValueBlock::Field& field) {
        return (field.Name() == name && field.Index() == index);
    });
    return (it != _fields.end() ? &*it : nullptr);
}
//----------------------------------------------------------------------------
hash_t ValueBlock::Hash(const MemoryView<const u8>& data) const {
    hash_t h(0);
    for (const Field& field : _fields)
        hash_combine(h, ValueHash(field.Type(), data.CutStartingAt(field.Offset())));

    return h;
}
//----------------------------------------------------------------------------
hash_t hash_value(const ValueBlock& block) {
    return hash_mem(block._fields.MakeView());
}
//----------------------------------------------------------------------------
bool operator ==(const ValueBlock& lhs, const ValueBlock& rhs) {
    if (lhs.size() != rhs.size())
        return false;

    forrange(i, 0, lhs._fields.size())
        if (lhs._fields[i] != rhs._fields[i])
            return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
