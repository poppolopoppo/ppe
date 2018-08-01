#include "stdafx.h"

#include "ValueBlock.h"

#include "Memory/HashFunctions.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t FValueBlock::SizeInBytes() const {
#ifdef WITH_PPE_ASSERT
    size_t sizeInBytes = 0;
    for (const FField& field : _fields) {
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
        const FField& last = _fields.back();
        return last.Offset() + ValueSizeInBytes(last.Type());
    }
#endif
}
//----------------------------------------------------------------------------
void FValueBlock::Add(const Graphics::FName& name, EValueType type, size_t offset, size_t index/* = 0 */, bool inUse/* = true */) {
    Assert(!name.empty());
    Assert(EValueType::Void != type);
    Assert(SizeInBytes() <= offset);
    Assert(nullptr == FindByNameAndIndexIFP(name, index));

    _fields.emplace_back(name, type, offset, index, inUse);
}
//----------------------------------------------------------------------------
void FValueBlock::Clear() {
    _fields.clear();
}
//----------------------------------------------------------------------------
void FValueBlock::Copy(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src) const {
    for (const FField& field : _fields)
        ValueCopy(field.Type(), dst.CutStartingAt(field.Offset()), src.CutStartingAt(field.Offset()));
}
//----------------------------------------------------------------------------
void FValueBlock::Defaults(const TMemoryView<u8>& dst) const {
    for (const FField& field : _fields)
        ValueDefault(field.Type(), dst.CutStartingAt(field.Offset()));
}
//----------------------------------------------------------------------------
bool FValueBlock::Equals(const TMemoryView<const u8>& lhs, const TMemoryView<const u8>& rhs) const {
    for (const FField& field : _fields)
        if (not ValueEquals(field.Type(), lhs.CutStartingAt(field.Offset()), rhs.CutStartingAt(field.Offset())))
            return false;

    return true;
}
//----------------------------------------------------------------------------
FValueBlock::FField& FValueBlock::FindByName(const Graphics::FName& name) {
    FField* const field = FindByNameIFP(name);
    Assert(field);
    return *field;
}
//----------------------------------------------------------------------------
FValueBlock::FField* FValueBlock::FindByNameIFP(const Graphics::FName& name) {
    const auto it = std::find_if(_fields.begin(), _fields.end(), [=](const FValueBlock::FField& field) {
        return (field.Name() == name);
    });
    return (it != _fields.end() ? std::addressof(*it) : nullptr);
}
//----------------------------------------------------------------------------
FValueBlock::FField& FValueBlock::FindByNameAndIndex(const Graphics::FName& name, size_t index) {
    FField* const field = FindByNameAndIndexIFP(name, index);
    Assert(field);
    return *field;
}
//----------------------------------------------------------------------------
FValueBlock::FField* FValueBlock::FindByNameAndIndexIFP(const Graphics::FName& name, size_t index) {
    const auto it = std::find_if(_fields.begin(), _fields.end(), [=](const FValueBlock::FField& field) {
        return (field.Name() == name && field.Index() == index);
    });
    return (it != _fields.end() ? std::addressof(*it) : nullptr);
}
//----------------------------------------------------------------------------
hash_t FValueBlock::HashValue(const TMemoryView<const u8>& data) const {
    hash_t h(0);
    for (const FField& field : _fields)
        hash_combine(h, ValueHash(field.Type(), data.CutStartingAt(field.Offset())));

    return h;
}
//----------------------------------------------------------------------------
hash_t hash_value(const FValueBlock& block) {
    return hash_mem(block._fields.MakeView());
}
//----------------------------------------------------------------------------
bool operator ==(const FValueBlock& lhs, const FValueBlock& rhs) {
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
} //!namespace PPE
