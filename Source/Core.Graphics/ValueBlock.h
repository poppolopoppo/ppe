#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Value.h"

#include "Core/Container/Vector.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ValueBlock {
public:
    class Field {
    public:
        Field() : _type(0), _index(0), _offset(0), _inUse(0) {}
        Field(Graphics::Name name, ValueType type, size_t offset, size_t index = 0, bool inUse = true)
            : _name(name), _index(u32(index)), _type(u32(type)), _offset(u32(offset)), _inUse(inUse?1:0) {
            Assert(ValueType::Void != type);
            Assert(index == _index);
            Assert(u32(type) == _type);
            Assert(offset == _offset);
        }

        const Graphics::Name& Name() const { return _name; }
        size_t Index() const { return _index; }
        ValueType Type() const { return ValueType(_type); }
        size_t Offset() const { return _offset; }
        size_t SizeInBytes() const { return ValueSizeInBytes(ValueType(_type)); }
        bool InUse() const { return (_inUse ? 1 : 0); }

        void Copy(const MemoryView<u8>& dst, const MemoryView<const u8>& src) const {
            ValueCopy(Type(), dst.CutStartingAt(_offset), src.CutStartingAt(_offset));
        }

        bool Equals(const MemoryView<const u8>& lhs, const MemoryView<const u8>& rhs) const {
            return ValueEquals(Type(), lhs.CutStartingAt(_offset), rhs.CutStartingAt(_offset));
        }

        void Lerp(const MemoryView<u8>& dst, const MemoryView<const u8>& a, const MemoryView<const u8>& b, float f) const {
            ValueLerp(Type(), dst.CutStartingAt(_offset), a.CutStartingAt(_offset), b.CutStartingAt(_offset), f);
        }

        bool IsPromotable(ValueType other) const {
            return (ValueIsPromotable(Type(), other) &&
                    ValueIsPromotable(other, Type()) );
        }

        bool IsPromotableFrom(ValueType src) const {
            return ValueIsPromotable(Type(), src);
        }

        bool IsPromotableTo(ValueType dst) const {
            return ValueIsPromotable(dst, Type());
        }

        bool Promote(const MemoryView<u8>& dst, const Value& src) const {
            return ValuePromote(Type(), dst.CutStartingAt(_offset), src.Type(), src.MakeView());
        }

        bool Promote(const MemoryView<u8>& dst, ValueType input, const MemoryView<const u8>& src) const {
            return ValuePromote(Type(), dst.CutStartingAt(_offset), input, src);
        }

        bool PromoteArray(const MemoryView<u8>& dst, size_t dstStride, ValueType input, const MemoryView<const u8>& src, size_t srcStride, size_t count) const {
            return ValuePromoteArray(Type(), dst.CutStartingAt(_offset), dstStride, input, src, srcStride, count);
        }

        template <typename T>
        bool Read(T& dst, const MemoryView<const u8>& src) const {
            return ValuePromote(ValueTraits<T>::Type, MakeRawView(dst), Type(), src.CutStartingAt(_offset));
        }

        template <typename T>
        bool Write(const MemoryView<u8>& dst, const T& src) const {
            return ValuePromote(Type(), dst.CutStartingAt(_offset), ValueTraits<T>::Type, MakeRawView(src));
        }

        void Clear(const MemoryView<u8>& dst) const {
            ValueDefault(Type(), dst);
        }

        friend bool operator ==(const Field& lhs, const Field& rhs) {
            return (lhs._name == rhs._name &&
                    lhs._type == rhs._type &&
                    lhs._offset == rhs._offset &&
                    lhs._index == rhs._index ); }
        friend bool operator !=(const Field& lhs, const Field& rhs) { return not operator ==(lhs, rhs); }

    private:
        Graphics::Name _name;
        u32 _index  : 8;
        u32 _type   : 7;
        u32 _inUse  : 1;
        u32 _offset : 16;
    };

    typedef VECTORINSITU(Value, Field, 6) fields_type;

    ValueBlock() {}
    ~ValueBlock() {}

    ValueBlock(std::initializer_list<Field> fields) : _fields(fields) {}
    ValueBlock(const MemoryView<const Field>& fields) : _fields(fields) {}

    size_t size() const { return _fields.size(); }
    bool empty() const { return _fields.empty(); }

    const Field& operator [](size_t index) const { return _fields[index]; }

    const Field& front() const { return _fields.front(); }
    const Field& back() const { return _fields.back(); }

    fields_type::const_iterator begin() const { return _fields.begin(); }
    fields_type::const_iterator end() const { return _fields.end(); }

    size_t SizeInBytes() const;

    void Add(const Graphics::Name& name, ValueType type) { Add(name, type, SizeInBytes()); }
    void Add(const Graphics::Name& name, ValueType type, size_t offset, size_t index = 0, bool inUse = true);
    void Clear();

    void Copy(const MemoryView<u8>& dst, const MemoryView<const u8>& src) const;
    void Defaults(const MemoryView<u8>& dst) const;
    bool Equals(const MemoryView<const u8>& lhs, const MemoryView<const u8>& rhs) const;

    const Field& FindByName(const Graphics::Name& name) const;
    const Field* FindByNameIFP(const Graphics::Name& name) const;

    const Field& FindByNameAndIndex(const Graphics::Name& name, size_t index) const;
    const Field* FindByNameAndIndexIFP(const Graphics::Name& name, size_t index) const;

    MemoryView<const Field> MakeView() const { return _fields.MakeConstView(); }

    hash_t Hash(const MemoryView<const u8>& data) const;

    friend hash_t hash_value(const ValueBlock& block);

    friend bool operator ==(const ValueBlock& lhs, const ValueBlock& rhs);
    friend bool operator !=(const ValueBlock& lhs, const ValueBlock& rhs) { return not operator ==(lhs, rhs); }

private:
    fields_type _fields;
};
//----------------------------------------------------------------------------
using ValueField = ValueBlock::Field;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
