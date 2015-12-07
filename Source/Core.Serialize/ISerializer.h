#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/MemoryView.h"
#include "Core.RTTI/RTTI_fwd.h"

namespace Core {
class IStreamReader;
class IStreamWriter;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISerializer {
public:
    ISerializer() {}
    virtual ~ISerializer() {}

    ISerializer(const ISerializer& ) = delete;
    ISerializer& operator =(const ISerializer& ) = delete;

    virtual RTTI::MetaTransaction *Transaction() const = 0;

    virtual void Deserialize(VECTOR(Transaction, RTTI::PMetaObject)& objects, const MemoryView<const u8>& input, const wchar_t *sourceName = nullptr) = 0;
    virtual void Serialize(IStreamWriter* output, const MemoryView<const RTTI::PMetaObject>& objects) = 0;

    void Deserialize(VECTOR(Transaction, RTTI::PMetaObject)& objects, IStreamReader* iss, const wchar_t *sourceName = nullptr);

    template <typename T, typename _Allocator>
    void Deserialize(Vector<RefPtr<T>, _Allocator>& objects, const MemoryView<const u8>& input, const wchar_t *sourceName = nullptr) {
        VECTOR(Transaction, RTTI::PMetaObject) tmp;
        Deserialize(tmp, input, sourceName);
        objects.reserve(tmp.size());
        for (const RTTI::PMetaObject& o : tmp)
            objects.emplace_back(checked_cast<T*>(o.get()));
    }

    template <typename T, typename _Allocator>
    void Deserialize(Vector<RefPtr<T>, _Allocator>& objects, IStreamReader* iss, const wchar_t *sourceName = nullptr) {
        VECTOR(Transaction, RTTI::PMetaObject) tmp;
        Deserialize(tmp, iss, sourceName);
        objects.reserve(tmp.size());
        for (const RTTI::PMetaObject& o : tmp)
            objects.emplace_back(checked_cast<T*>(o.get()));
    }

    template <typename T, typename _Allocator>
    void Serialize(IStreamWriter* output, const Vector<RefPtr<T>, _Allocator>& objects) {
        STACKLOCAL_POD_ARRAY(RTTI::PMetaObject, tmp, objects.size());
        Assert(tmp.SizeInBytes() == sizeof(RefPtr<T>)*objects.size()); // wont change the refcounting
        memcpy(tmp.Pointer(), objects.data(), tmp.SizeInBytes());
        Serialize(output, tmp);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
