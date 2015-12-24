#include "stdafx.h"

#include "RobotApp.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FileSystem.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Color/Color.h"
#include "Core/Maths/Maths.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Memory/Compression.h"
#include "Core/Memory/MemoryStream.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core.RTTI/RTTI.h"
#include "Core.RTTI/RTTIMacros.h"
#include "Core.RTTI/RTTIMacros-impl.h"
#include "Core.RTTI/MetaAtomVisitor.h"
#include "Core.RTTI/MetaTransaction.h"
#include "Core.Serialize/Binary/BinarySerializer.h"

#include "Core.RTTI/MetaType.Definitions-inl.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
FWD_REFPTR(ContentIdentity);
class ContentIdentity : public RTTI::MetaObject {
public:
    RTTI_CLASS_HEADER(ContentIdentity, RTTI::MetaObject);

    ContentIdentity() {}
    ContentIdentity(const Filename& sourceFile, const DateTime& lastModified)
        : _sourceFile(sourceFile), _lastModified(lastModified) {}

    const Filename& SourceFile() const { return _sourceFile; }
    const DateTime& LastModified() const { return _lastModified; }

private:
    Filename _sourceFile;
    DateTime _lastModified;
};
RTTI_CLASS_BEGIN(ContentIdentity, Concrete)
RTTI_PROPERTY_PRIVATE_FIELD(_sourceFile)
RTTI_PROPERTY_PRIVATE_FIELD(_lastModified)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FWD_REFPTR(RTTITest_);
class RTTITest_ : public RTTI::MetaObject {
public:
    RTTI_CLASS_HEADER(RTTITest_, RTTI::MetaObject);

    RTTITest_() : _dummy(0)
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    ,   _ ## _Name(RTTI::MetaTypeTraits<T>::meta_type::DefaultValue()) \
    ,   _ ## _Name ## Pair(RTTI::MetaTypeTraits<T>::meta_type::DefaultValue(), RTTI::MetaTypeTraits<T>::meta_type::DefaultValue())
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_
    {}

    bool _dummy;

    template <typename T>
    using yolo_pair_type = Pair<T, T>;
    template <typename T>
    using yolo_dict_type = HASHMAP_THREAD_LOCAL(Container, T, T);
    template <typename T>
    using yolo_vect_type = VECTOR_THREAD_LOCAL(Container, yolo_dict_type<T>);
    template <typename T>
    using yolo_type = HASHMAP_THREAD_LOCAL(Container, yolo_pair_type<T>, yolo_vect_type<T>);

#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    T _ ## _Name; \
    VECTOR_THREAD_LOCAL(Container, T) _ ## _Name ## Vec; \
    Pair<T, T> _ ## _Name ## Pair; \
    HASHMAP_THREAD_LOCAL(Container, T, T) _ ## _Name ## Dico; \
    //yolo_type<T> _ ## _Name ## Yolo;
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_
};
RTTI_CLASS_BEGIN(RTTITest_, Concrete)
#define DEF_METATYPE_SCALAR_IMPL_(_Name, T, _TypeId, _Unused) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Vec) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Pair) \
    RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Dico) \
    //RTTI_PROPERTY_PRIVATE_FIELD(_ ## _Name ## Yolo)
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR_IMPL_)
#undef DEF_METATYPE_SCALAR_IMPL_

RTTI_CLASS_END()
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class RTTIAtomRandomizer_ : protected RTTI::MetaAtomWrapMoveVisitor {
    typedef RTTI::MetaAtomWrapMoveVisitor parent_type;
public:
    RTTIAtomRandomizer_(size_t maxDim, u64 seed) : _maxDim(maxDim), _rand(seed) {
        AssertRelease(IS_POW2(_maxDim));
    }

    void Randomize(RTTI::MetaObject* pobject);

protected:
    using parent_type::Inspect;
    using parent_type::Visit;

    virtual void Inspect(RTTI::IMetaAtomPair* ppair, RTTI::Pair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair) override {
        pair.first = ppair->FirstTraits()->CreateDefaultValue();
        pair.second = ppair->SecondTraits()->CreateDefaultValue();

        parent_type::Inspect(ppair, pair);
    }

    virtual void Inspect(RTTI::IMetaAtomVector* pvector, RTTI::Vector<RTTI::PMetaAtom>& vector) override {
        const RTTI::IMetaTypeVirtualTraits* traits = pvector->ValueTraits();
        const size_t count = NextRandomDim_();
        vector.reserve(count);
        forrange(i, 0, count)
            vector.emplace_back(traits->CreateDefaultValue());

        parent_type::Inspect(pvector, vector);
    }

    virtual void Inspect(RTTI::IMetaAtomDictionary* pdictionary, RTTI::Dictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
        const RTTI::IMetaTypeVirtualTraits* keyTraits = pdictionary->KeyTraits();
        const RTTI::IMetaTypeVirtualTraits* valueTraits = pdictionary->ValueTraits();
        const size_t count = NextRandomDim_();
        dictionary.reserve(count);
        forrange(i, 0, count)
            dictionary.Vector().emplace_back(
                keyTraits->CreateDefaultValue(),
                valueTraits->CreateDefaultValue() );

        parent_type::Inspect(pdictionary, dictionary);
    }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    virtual void Visit(RTTI::MetaTypedAtom<T>* scalar) override { \
        Assert(_TypeId == scalar->TypeInfo().Id); \
        Randomize_(scalar->Wrapper()); \
        /*parent_type::Visit(scalar);*/ \
    }
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

private:
    const size_t _maxDim;
    RandomGenerator _rand;

    HASHMAP_THREAD_LOCAL(RTTI, RTTI::MetaTypeId, RTTI::PMetaAtom) _atomCache;

    size_t NextRandomDim_() { return (_rand.Next() % _maxDim); }

    template <typename T>
    void Randomize_(T& value) {
        _rand.Randomize(value);
    }

    template <typename T, size_t _Width, size_t _Height>
    void Randomize_(ScalarMatrix<T, _Width, _Height>& value) {
        ScalarMatrixData<T, _Width, _Height>& data = value.data_();
        const size_t dim = _Width*_Height;
        for (size_t i = 0; i < dim; ++i)
            _rand.Randomize(data.raw[i]);
    }

    void Randomize_(String& str) {
        const size_t count = NextRandomDim_();
        str.resize(count);
        forrange(i, 0, count)
            str[i] = (char)(_rand.Next('a'+0, 'z'+1));
    }

    void Randomize_(WString& wstr) {
        const size_t count = NextRandomDim_();
        wstr.resize(count);
        forrange(i, 0, count)
            wstr[i] = (wchar_t)(_rand.Next(L'a'+0, L'z'+1));
    }

    void Randomize_(Core::RTTI::PMetaAtom& atom) {
        Assert(atom);
        AssertNotImplemented();
    }

    void Randomize_(Core::RTTI::PMetaObject& object) {
        if (object)
            Randomize(object.get());
    }
};
//----------------------------------------------------------------------------
void RTTIAtomRandomizer_::Randomize(RTTI::MetaObject* pobject) {
    Assert(pobject);
    for (const auto& it : pobject->RTTI_MetaClass()->Properties()) {
        RTTI::PMetaAtom& atom = _atomCache[it.second->TypeInfo().Id];
        if (atom)
            it.second->MoveTo(pobject, atom.get());
        else
            atom = it.second->WrapMove(pobject);
        parent_type::Append(atom.get());
        it.second->UnwrapMove(pobject, atom.get());
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RobotApp::RobotApp()
: parent_type(
    L"RobotApp", 100, 100,
    Graphics::PresentationParameters(
        640, 480,
        Graphics::SurfaceFormatType::B8G8R8A8_SRGB,
        Graphics::SurfaceFormatType::D24S8,
        false,
        false,
        0,
        Graphics::PresentInterval::Default ),
    Graphics::DeviceAPI::DirectX11,
    true ) {
}
//----------------------------------------------------------------------------
void RobotApp::Start() {
    parent_type::Start();

    typedef RTTITest_ test_type;
    static const size_t test_count = 32;

    ContentIdentity::MetaClass::Create();
    RTTITest_::MetaClass::Create();
    {
        const wchar_t* filename = L"Temp:/robotapp.bin";

        VECTOR_THREAD_LOCAL(RTTI, RefPtr<test_type>) input;
        {
            RTTIAtomRandomizer_ rand(8, 0xabadcafeull);
            forrange(i, 0, test_count) {
                RefPtr<test_type> t = new test_type();
                rand.Randomize(t.get());
                input.emplace_back(t);
            }
        }

        RTTI::MetaTransaction transaction;
        Serialize::BinarySerializer serializer(&transaction);

        MEMORYSTREAM_THREAD_LOCAL(Serialize) uncompressed;
        {
            serializer.Serialize(&uncompressed, input);
#if 0
            auto compressed = VFS_OpenBinaryWritable(filename, AccessPolicy::Truncate);
            LZJB::CompressMemory(compressed.get(), uncompressed.MakeView());
#else
            RAWSTORAGE_THREAD_LOCAL(Serialize, u8) compressed;
            Compression::CompressMemory(compressed, uncompressed.MakeView(), Compression::HighCompression);

            RAWSTORAGE_THREAD_LOCAL(Stream, u8) decompressed;
            Compression::DecompressMemory(decompressed, compressed.MakeView());

            Assert(uncompressed.SizeInBytes() == decompressed.SizeInBytes());
            const size_t k = decompressed.SizeInBytes();
            for (size_t i = 0; i < k; ++i)
                Assert(uncompressed.Pointer()[i] == decompressed.Pointer()[i]);

            VFS_WriteAll(filename, compressed.MakeView(), AccessPolicy::Truncate_Binary);

            RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) stored;
            VFS_ReadAll(&stored, filename, AccessPolicy::Binary);

            Assert(stored.SizeInBytes() == compressed.SizeInBytes());
            const size_t n = compressed.SizeInBytes();
            for (size_t i = 0; i < n; ++i)
                Assert(stored.Pointer()[i] == compressed.Pointer()[i]);
#endif
        }

        VECTOR_THREAD_LOCAL(RTTI, RefPtr<test_type>) output;

        {
            RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) compressed;
            VFS_ReadAll(&compressed, filename, AccessPolicy::Binary);
            RAWSTORAGE_THREAD_LOCAL(Stream, u8) decompressed;
            Compression::DecompressMemory(decompressed, compressed.MakeConstView());

            Assert(uncompressed.SizeInBytes() == decompressed.SizeInBytes());
            const size_t k = decompressed.SizeInBytes();
            for (size_t i = 0; i < k; ++i)
                Assert(uncompressed.Pointer()[i] == decompressed.Pointer()[i]);

            serializer.Deserialize(output, decompressed.MakeConstView());
        }

        AssertRelease(input.size() == output.size());
        for (size_t i = 0; i < output.size(); ++i)
            AssertRelease(RTTI::Equals(*input[i], *output[i]));
    }
    RTTITest_::MetaClass::Destroy();
    ContentIdentity::MetaClass::Destroy();

    RenderLoop();
}
//----------------------------------------------------------------------------
void RobotApp::Draw(const Timeline& time) {
    parent_type::Draw(time);

    const double totalSeconds = Units::Time::Seconds(time.Total()).Value();
    const float t = float(Frac(totalSeconds*0.1));
    const float3 hsv(t, 1.0f, 0.5f);
    const float3 rgb = HSV_to_RGB(hsv);
    const ColorRGBAF clearColor(rgb, 1.0f);

    Graphics::IDeviceAPIEncapsulator* const device = DeviceEncapsulator().Device();
    device->Clear(device->BackBufferRenderTarget(), clearColor);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core