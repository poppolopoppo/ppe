#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Value.h"

#include "Core/Container/Vector.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FValueBlock {
public:
    class TField {
    public:
        TField() : _type(0), _index(0), _offset(0), _inUse(0) {}
        TField(Graphics::FName name, EValueType type, size_t offset, size_t index = 0, bool inUse = false)
            : _name(name), _index(u32(index)), _type(u32(type)), _offset(u32(offset)), _inUse(inUse?1:0) {
            Assert(EValueType::Void != type);
            Assert(index == _index);
            Assert(u32(type) == _type);
            Assert(offset == _offset);
        }

        const Graphics::FName& Name() const { return _name; }
        size_t Index() const { return _index; }
        EValueType Type() const { return EValueType(_type); }
        size_t Offset() const { return _offset; }
        size_t SizeInBytes() const { return ValueSizeInBytes(EValueType(_type)); }
        bool InUse() const { return bool(_inUse); }

        void SetInUse(bool value = true) { _inUse = (value?1:0); }

        void Copy(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src) const { ValueCopy(Type(), dst.CutStartingAt(_offset), src.CutStartingAt(_offset)); }
        bool Equals(const TMemoryView<const u8>& lhs, const TMemoryView<const u8>& rhs) const { return ValueEquals(Type(), lhs.CutStartingAt(_offset), rhs.CutStartingAt(_offset)); }

        void Lerp(const TMemoryView<u8>& dst, const TMemoryView<const u8>& a, const TMemoryView<const u8>& b, float f) const {
            ValueLerp(Type(), dst.CutStartingAt(_offset), a.CutStartingAt(_offset), b.CutStartingAt(_offset), f);
        }

        bool IsPromotable(EValueType other) const { return (ValueIsPromotable(Type(), other) && ValueIsPromotable(other, Type()) ); }
        bool IsPromotableFrom(EValueType src) const { return ValueIsPromotable(Type(), src); }
        bool IsPromotableTo(EValueType dst) const { return ValueIsPromotable(dst, Type()); }

        bool Promote(const TMemoryView<u8>& dst, const FValue& src) const { return ValuePromote(Type(), dst.CutStartingAt(_offset), src.Type(), src.MakeView()); }
        bool Promote(const TMemoryView<u8>& dst, EValueType input, const TMemoryView<const u8>& src) const { return ValuePromote(Type(), dst.CutStartingAt(_offset), input, src); }

        bool PromoteArray(const TMemoryView<u8>& dst, size_t dstStride, EValueType input, const TMemoryView<const u8>& src, size_t srcStride, size_t count) const {
            return ValuePromoteArray(Type(), dst.CutStartingAt(_offset), dstStride, input, src, srcStride, count);
        }

        template <typename T>
        bool Read(T& dst, const TMemoryView<const u8>& src) const {
            return ValuePromote(TValueTraits<T>::Type, MakeRawView(dst), Type(), src.CutStartingAt(_offset));
        }

        template <typename T>
        bool Write(const TMemoryView<u8>& dst, const T& src) const {
            return ValuePromote(Type(), dst.CutStartingAt(_offset), TValueTraits<T>::Type, MakeRawView(src));
        }

        void Clear(const TMemoryView<u8>& dst) const {
            ValueDefault(Type(), dst);
        }

        friend bool operator ==(const TField& lhs, const TField& rhs) { return (lhs._name == rhs._name && lhs._type == rhs._type && lhs._offset == rhs._offset && lhs._index == rhs._index ); }
        friend bool operator !=(const TField& lhs, const TField& rhs) { return not operator ==(lhs, rhs); }

    private:
        Graphics::FName _name;
        u32 _index  : 8;
        u32 _type   : 7;
        u32 _inUse  : 1;
        u32 _offset : 16;
    };

    typedef VECTORINSITU(Value, TField, 6) fields_type;

    FValueBlock() {}
    ~FValueBlock() {}

    FValueBlock(std::initializer_list<TField> fields) : _fields(fields) {}
    FValueBlock(const TMemoryView<const TField>& fields) : _fields(fields) {}

    size_t size() const { return _fields.size(); }
    bool empty() const { return _fields.empty(); }

    const TField& operator [](size_t index) const { return _fields[index]; }

    const TField& front() const { return _fields.front(); }
    const TField& back() const { return _fields.back(); }

    fields_type::const_iterator begin() const { return _fields.begin(); }
    fields_type::const_iterator end() const { return _fields.end(); }

    size_t SizeInBytes() const;

    void Add(const Graphics::FName& name, EValueType type) { Add(name, type, SizeInBytes()); }
    void Add(const Graphics::FName& name, EValueType type, size_t offset, size_t index = 0, bool inUse = true);
    void Clear();

    void Copy(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src) const;
    void Defaults(const TMemoryView<u8>& dst) const;
    bool Equals(const TMemoryView<const u8>& lhs, const TMemoryView<const u8>& rhs) const;

    TField& FindByName(const Graphics::FName& name);
    TField* FindByNameIFP(const Graphics::FName& name);

    TField& FindByNameAndIndex(const Graphics::FName& name, size_t index);
    TField* FindByNameAndIndexIFP(const Graphics::FName& name, size_t index);

    const TField& FindByName(const Graphics::FName& name) const { return remove_const(this)->FindByName(name); }
    const TField* FindByNameIFP(const Graphics::FName& name) const { return remove_const(this)->FindByNameIFP(name); }

    const TField& FindByNameAndIndex(const Graphics::FName& name, size_t index) const { return remove_const(this)->FindByNameAndIndex(name, index); }
    const TField* FindByNameAndIndexIFP(const Graphics::FName& name, size_t index) const { return remove_const(this)->FindByNameAndIndexIFP(name, index); }

    TMemoryView<const TField> MakeView() const { return _fields.MakeConstView(); }

    hash_t THash(const TMemoryView<const u8>& data) const;

    friend hash_t hash_value(const FValueBlock& block);

    friend bool operator ==(const FValueBlock& lhs, const FValueBlock& rhs);
    friend bool operator !=(const FValueBlock& lhs, const FValueBlock& rhs) { return not operator ==(lhs, rhs); }

private:
    fields_type _fields;
};
//----------------------------------------------------------------------------
using FValueField = FValueBlock::TField;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
