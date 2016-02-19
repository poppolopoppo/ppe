#include "stdafx.h"

#include "ApplicationTest.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/Heap.h"
#include "Core/Container/Vector.h"
#include "Core/Diagnostic/Callstack.h"
#include "Core/Diagnostic/CrtDebug.h"
#include "Core/Diagnostic/DecodedCallstack.h"
#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/IO/String.h"
#include "Core/IO/StringSlice.h"
#include "Core/Memory/MemoryStack.h"
#include "Core/Memory/MemoryView.h"

#include "Core/Thread/ThreadContext.h"
#include "Core/Thread/ThreadLocalStorage.h"

#include "Core/Thread/LockFreeCircularArray.h"

#include "Core/Container/RawStorage.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Memory/UniqueView.h"

#include "Core/Thread/Task/Task.h"
#include "Core/Thread/Task/TaskContext.h"
#include "Core/Thread/Task/TaskWorker.h"
#include "Core/Thread/ThreadPool.h"

#include "Core/Diagnostic/CurrentProcess.h"

#include "Core/Container/Token.h"

#include "Core/IO/FileSystem.h"
#include "Core/IO/VFS/VirtualFileSystemNativeComponent.h"
#include "Core/IO/VirtualFileSystem.h"

#include "Core.RTTI/Class/MetaClassName.h"
#include "Core.RTTI/MetaTransaction.h"
#include "Core.RTTI/Object/MetaObject.h"
#include "Core.RTTI/Type/MetaType.h"
#include "Core.RTTI/Type/MetaTypeTraits.h"

#include "Core.RTTI/RTTI.h"
#include "Core.RTTI/RTTIMacros-impl.h"
#include "Core.RTTI/RTTIMacros.h"

#include "Core.Serialize/Lexer/Lexer.h"
#include "Core.Serialize/Lexer/Symbols.h"
#include "Core.Serialize/Parser/Parser.h"

#include "Core/Container/Tuple.h"

#include "Core.Serialize/Text/Grammar.h"

#include "Core/Color/Color.h"

#include "Core/Time/Timeline.h"

#include "Core/Diagnostic/Logger.h"

#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Maths/Geometry/ScalarBoundingBoxHelpers.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/Packing/PackedVectors.h"
#include "Core/Maths/Packing/PackingHelpers.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"
#include "Core/Maths/Units.h"

#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"
#include "Core.Graphics/Device/State/BlendState.h"
#include "Core.Graphics/Device/State/DepthStencilState.h"
#include "Core.Graphics/Device/State/RasterizerState.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"
#include "Core.Graphics/Graphics.h"

#include <iostream>
#include <vector>

#ifdef OS_WINDOWS
#   include <Windows.h>
#else
#   error "no support"
#endif

static void Print(const Core::CrtMemoryStats& memoryStats) {
    std::cerr << "Memory statistics :" << std::endl
        << " - Total free size          = " << memoryStats.TotalFreeSize << std::endl
        << " - Largest free block       = " << memoryStats.LargestFreeBlockSize << std::endl
        << " - Total used size          = " << memoryStats.TotalUsedSize << std::endl
        << " - Largest used block       = " << memoryStats.LargestUsedBlockSize << std::endl
        << " - Total overhead size      = " << memoryStats.TotalOverheadSize << std::endl
        << " - Total comitted size      = " << Core::SizeInBytes{ memoryStats.TotalOverheadSize.Value + memoryStats.TotalFreeSize.Value + memoryStats.TotalUsedSize.Value } << std::endl
        << " - External fragmentation   = " << (memoryStats.ExternalFragmentation() * 100) << "%" << std::endl;
}

void TestCallstack_() {
    using namespace Core;

    Callstack callstack{ 2, 32 };
    DecodedCallstack decoded{ callstack };

    std::cout << "Callstack[" << decoded.Depth() << "] #"
        << std::hex << decoded.Hash() << std::dec
        << std::endl;

    std::cout << decoded << std::endl;
}

void TestStrings_() {
    using namespace Core;

    wchar_t buffer[1024];
    Format(buffer, L"string = {2}, decimal = {0}, float = {1}\n", "test", 42, 0.123456f);
    std::cout << buffer;

    WString wstr = StringFormat(L"num={0} alphabool={0:a}", true);

    const char* test = "titi,toto,,tata,,";

    const char* s = test;
    StringSlice p;
    while (Split(&s, ',', p))
        std::cout << p << std::endl;

    Format(std::cout, "string = {0:10U} {0:-10U}, decimal = {1:8X} {1:#8x}, float = {2:f3} {2:10f4}\n", "test", 0xBADCAFE, -0.123456f);

    Format(std::cout, "{0*16}\n", '-');

    Format(std::cout, "{0:#4*4}\n", 42);
}

void TestHash_() {
    using namespace Core;

    static u32 numbers[6] = {9,18,27,36,45,54};

    size_t h0 = hash_value(numbers);
    size_t h1 = hash_value(numbers[0],numbers[1],numbers[2],numbers[3],numbers[4],numbers[5]);
    size_t h2 = hash_value_seq(&numbers[0],&numbers[6]);

    AssertRelease(h0 == h1);
    AssertRelease(h1 == h2);

    size_t h3 = hash_value(numbers, numbers, numbers);

    size_t h4 = hash_value(4.1234567f);
    size_t h5 = hash_value(4.1234568f);
    AssertRelease(h4 != h5);

    int toto = 42;
    size_t h6 = hash_value(&toto);
    size_t h7 = hash_value((const void *)&toto);
    AssertRelease(h6 == h7);

    size_t h8 = hash_value(&numbers);
    size_t h9 = hash_value((const void *)&numbers);
    AssertRelease(h8 == h9);
}

class A : public Core::RefCountable {
public:
    A() {}
};

class B : public Core::RefCountable {
public:
    B() {}
    virtual ~B() {}
    virtual int Value() const { return 0; }
};
class C : public B {
public:
    C() {}
    virtual ~C() {}
    virtual int Value() const override { return 42; }
};

void TestPointers_() {
    using namespace Core;

    RefPtr<A> a = new A();
    RefPtr<C> c = new C();

    RefPtr<B> b = c;
    AssertRelease(b == c);
    AssertRelease(a != b);

    std::cout << c->Value() << std::endl;
}

int fib(int x) {
    if (x == 1)
        return 1;
    else if (x <= 0)
        return 0;
    else
        return fib(x-1)+fib(x-2);
}

void TestThreads_() {
    using namespace Core;

    ThreadContext& context = CurrentThreadContext::Instance();

    std::cout << context.Name() << ", " << context.Tag() << " = " << context.Id() << std::endl;

    LockFreeCircularQueue_SingleProducer<const wchar_t*> queue(128);

    queue.Produce(L"Toto");

    const wchar_t* str = nullptr;
    if (!queue.Consume(str))
        AssertRelease(0);

    std::cout << str << std::endl;

    {
        TaskEvaluator evaluator("GlobalEvaluator", std::thread::hardware_concurrency(), 90);
        evaluator.Start();

        TaskCompletionPort completionPort("Test", &evaluator);

        const PTask task = new LambdaTask([](const TaskContext& ctx) {
            std::cout   << "[" << ctx.Worker()->ThreadIndex() << "] "
                        << fib(15) << "!" << std::endl;
            return TaskResult::Succeed;
        });

        for (size_t i = 0; i < 100; ++i)
            completionPort.Produce(task);

        completionPort.WaitAll();
        std::cout << "done!" << std::endl;

        for (size_t i = 0; i < 100; ++i)
            completionPort.Produce(task);

        while (completionPort.WaitOne())
            std::cout << "done!" << std::endl;

        evaluator.Shutdown();
    }
}

struct TestToken {};

class TestToken2 : public Core::Token<TestToken2, char, Core::CaseSensitive::False> {
};

void TestTokens_() {
    using namespace Core;

    typedef Token<TestToken, char, CaseSensitive::False> token_type;
    token_type::Startup tokenStartup(32);

    token_type t("toto");
    token_type v("Toto");
    AssertRelease(t == v);

    v = "tata";
    AssertRelease(t != v);
    v = t;
    AssertRelease(t == v);

    token_type e;
    AssertRelease(e != v);
    AssertRelease(e != t);

    token_type q = "";

    Hash<token_type> h;
    std::cout << h(t) << std::endl;
    std::cout << h(v) << std::endl;
    std::cout << h(e) << std::endl;
    std::cout << h(q) << std::endl;
}

void TestFileSystem_() {
    using namespace Core;

    Extname ext = L".jpg";

    std::cout << ext << " (" << hash_value(ext) << ")" << std::endl;
    std::cout << Extname(L".jPG") << std::endl;

    Extname e{ WString(L".Jpg") };
    AssertRelease(e == ext);
    AssertRelease(e.c_str() == ext.c_str());
    std::cout << e << " (" << hash_value(e) << ")" << std::endl;

    std::cout
        << "Size of filesystem classes :" << std::endl
        << " * MountingPoint    = " << sizeof(MountingPoint)    << std::endl
        << " * Dirpath          = " << sizeof(Dirpath)          << std::endl
        << " * Basename         = " << sizeof(Basename)         << std::endl
        << " * Extname          = " << sizeof(Extname)          << std::endl
        << " * Filename         = " << sizeof(Filename)         << std::endl
        << " * Vector<int>      = " << sizeof(std::vector<int>) << std::endl
        << " * Vector<Dirname>  = " << sizeof(std::vector<Dirname>) << std::endl
        << " * RawStorage<int>  = " << sizeof(RAWSTORAGE(Task, int)) << std::endl
        << std::endl;

    MountingPoint mount(L"Data:");
    Dirpath path = { L"Data:", L"Assets", L"3D", L"Models" };
    Basename basename(L"Test", L".JPG");
    Filename filename{ path, basename };

    std::cout << filename << " (" << hash_value(filename) << ")" << std::endl;

    Filename f = L"data:/assets/3d/models/test.jpg";
    std::cout << f << " (" << hash_value(f) << ")" << std::endl;
    f = L"test.jpg";
    std::cout << f << " (" << hash_value(f) << ")" << " (basename: " << hash_value(f.Basename()) << ")" << std::endl;
    f = L"models/test.jpg";
    std::cout << f << " (" << hash_value(f) << ")" << std::endl;
    f = L"data:/assets/3d/models/";
    std::cout << f << " (" << hash_value(f) << ")" << std::endl;
    f = L"DATA:/ASSETS/3D/MODELS/TEST.JPG";
    std::cout << f << " (" << hash_value(f) << ")" << std::endl;

    std::cout << f.Basename() << hash_value(f.Basename()) << std::endl;
    std::cout << basename << hash_value(basename) << std::endl;

    std::cout << f.BasenameNoExt() << hash_value(f.BasenameNoExt()) << std::endl;
    std::cout << basename.BasenameNoExt() << hash_value(basename.BasenameNoExt()) << std::endl;

    std::cout << f.Extname() << hash_value(f.Extname()) << std::endl;
    std::cout << basename.Extname() << hash_value(basename.Extname()) << std::endl;

    AssertRelease(hash_value(f) == hash_value(filename));
    AssertRelease(hash_value(e) == hash_value(filename.Extname()));
    AssertRelease(hash_value(e) == hash_value(basename.Extname()));

    Dirpath p = L"data:/assets/3d";
    std::cout << p << " (" << hash_value(p) << ")" << std::endl;
    p = L"data:";
    std::cout << p << " (" << hash_value(p) << ")" << std::endl;
    p = L"data:/assets";
    std::cout << p << " (" << hash_value(p) << ")" << std::endl;
}

void TestVirtualFileSystem_() {
    using namespace Core;

    VirtualFileSystemRoot& vfs = VirtualFileSystem::Instance();

    vfs.MountNativePath(L"C:", L"C:");

    vfs.TryCreateDirectory(L"process:/tmp/1/2/3");

    {
        auto oss = vfs.OpenWritable(L"Process:/test.txt", AccessPolicy::Truncate);
        oss->WriteArray(L"ceci est un test, c'est très süper.");
    }

    {
        auto oss = vfs.OpenWritable(L"Process:/tmp/1/2/test.txt", AccessPolicy::Truncate);
        oss->WriteArray(L"ceci est un test, c'est très süper.");
    }

    {
        auto oss = vfs.OpenReadable(L"Process:/tmp/1/2/test.txt");
        RAWSTORAGE(FileSystem, wchar_t) read;
        oss->ReadAll(read);
        std::cout << read.Pointer() << std::endl;
    }

    if (!vfs.FileExists(L"procesS:/tEsT.tXT"))
        AssertRelease(false);

    vfs.EnumerateFiles(L"process:/", true, [](const Core::Filename& filename) {
        std::cout << filename << std::endl;
    });
}

FWD_REFPTR(Titi);
class Titi : public Core::RTTI::MetaObject {
public:
    Titi() {}
    virtual ~Titi() {}
    RTTI_CLASS_HEADER(Titi, void);
private:
    int _count;
    Core::String _name;
    VECTOR(Internal, PTiti) _tities;
};
RTTI_CLASS_BEGIN(Titi, Concrete)
RTTI_PROPERTY_FIELD(_count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_FIELD(_tities)
RTTI_CLASS_END()

FWD_REFPTR(Toto);
class Toto : public Core::RTTI::MetaObject {
public:
    Toto() {}
    virtual ~Toto() {}
    RTTI_CLASS_HEADER(Toto, void);
private:
    int _count;
    Core::String _name;
    VECTOR(Internal, PTiti) _tities;
    VECTOR_THREAD_LOCAL(Internal, Core::String) _titles;
    Core::RefPtr<Toto> _parent;
    Core::Pair<int, int> _pair;
    Core::Pair<float, float> _fpair;
    Core::Pair<Core::Pair<int, int>, Core::Pair<int, int> > _vpair;
    Core::Pair<Core::Pair<int, int>, Core::Pair<int, float> > _vpair2;
    HASHMAP_THREAD_LOCAL(Internal, Core::String, float) _dict;
    HASHMAP_THREAD_LOCAL(Internal, Core::String, Core::Pair<int COMMA Core::Pair<float COMMA Core::String>>) _dict2;
};
RTTI_CLASS_BEGIN(Toto, Concrete)
RTTI_PROPERTY_FIELD_ALIAS(_count, Count)
RTTI_PROPERTY_FIELD_ALIAS(_name, Name)
RTTI_PROPERTY_FIELD_ALIAS(_tities, Tities)
RTTI_PROPERTY_FIELD_ALIAS(_titles, Titles)
RTTI_PROPERTY_FIELD_ALIAS(_parent, Parent)
RTTI_PROPERTY_FIELD_ALIAS(_pair, Pair)
RTTI_PROPERTY_FIELD_ALIAS(_fpair, FPair)
RTTI_PROPERTY_FIELD_ALIAS(_vpair, VPair)
RTTI_PROPERTY_FIELD_ALIAS(_vpair2, VPair2)
RTTI_PROPERTY_FIELD_ALIAS(_dict, Dict)
RTTI_PROPERTY_FIELD_ALIAS(_dict2, Dict2)
RTTI_CLASS_END()

template <typename T>
static void TestRTTIWrap_() {
    typedef Core::RTTI::MetaTypeTraits< T > type_traits;
    T wrapped;
    typename type_traits::wrapper_type wrapper;
    type_traits::WrapCopy(wrapper, wrapped);
    type_traits::WrapMove(wrapper, std::move(wrapped));
    type_traits::UnwrapCopy(wrapped, wrapper);
    type_traits::UnwrapMove(wrapped, std::move(wrapper));
}

static void TestRTTI_() {
    using namespace Core;

    Format(std::cout, "Id = {0}, Name = {1}, Default = {2}, Flags = {3}\n",
        RTTI::MetaType< int32_t >::Id(),
        RTTI::MetaType< int32_t >::Name(),
        RTTI::MetaType< int32_t >::DefaultValue(),
        (size_t)RTTI::MetaType< int32_t >::Flags()
        );

    Format(std::cout, "Id = {0}, Name = {1}, Default = {2}, Flags = {3}\n",
        RTTI::MetaType< int >::Id(),
        RTTI::MetaType< int >::Name(),
        RTTI::MetaType< int >::DefaultValue(),
        (size_t)RTTI::MetaType< int >::Flags()
        );

    Format(std::cout, "Id = {0}, Name = {1}, Default = {2}, Flags = {3}\n",
        RTTI::MetaType< size_t >::Id(),
        RTTI::MetaType< size_t >::Name(),
        RTTI::MetaType< size_t >::DefaultValue(),
        (size_t)RTTI::MetaType< size_t >::Flags()
        );

    Format(std::cout, "Id = {0}, Name = {1}, Default = {2}, Flags = {3}\n",
        RTTI::MetaType< String >::Id(),
        RTTI::MetaType< String >::Name(),
        RTTI::MetaType< String >::DefaultValue(),
        (size_t)RTTI::MetaType< String >::Flags()
        );

    Format(std::cout, "Id = {0}, Name = {1}, Default = {2}, Flags = {3}\n",
        RTTI::MetaType< VECTOR(RTTI, int) >::Id(),
        RTTI::MetaType< VECTOR(RTTI, int) >::Name(),
        RTTI::MetaType< VECTOR(RTTI, int) >::DefaultValue(),
        (size_t)RTTI::MetaType< VECTOR(RTTI, int) >::Flags()
        );

    Format(std::cout, "Id = {0}, Name = {1}, Default = {2}, Flags = {3}\n",
        RTTI::MetaType< Pair<float, int> >::Id(),
        RTTI::MetaType< Pair<float, int> >::Name(),
        RTTI::MetaType< Pair<float, int> >::DefaultValue(),
        (size_t)RTTI::MetaType< Pair<float, int> >::Flags()
        );

    RTTI::MetaTypeTraits< int >::meta_type::Name();

    int i = 0;
    RTTI::MetaTypeTraits< int >::UnwrapCopy(i, 42);

    RTTI::PMetaObject o;
    RTTI::MetaTypeTraits< RTTI::PMetaObject >::UnwrapCopy(o, nullptr);

    PTiti t;
    RTTI::MetaTypeTraits< PTiti >::UnwrapCopy(t, nullptr);
    RTTI::MetaTypeTraits< PTiti >::WrapMove(o, std::move(t));

    TestRTTIWrap_< PTiti >();
    TestRTTIWrap_< Pair<WString, PTiti> >();
    TestRTTIWrap_< VECTOR_THREAD_LOCAL(RTTI, PTiti) >();
    TestRTTIWrap_< HASHMAP(RTTI, int, int) >();
    TestRTTIWrap_< HASHMAP(RTTI, String, PTiti) >();
    TestRTTIWrap_< ASSOCIATIVE_VECTOR(RTTI, String, PTiti) >();
    //TestRTTIWrap_< HASHSET(RTTI, String) >();

    RTTI::MetaClassSingleton<Titi>::Create();

    t = new Titi();
    const RTTI::MetaClass *metaClass = t->RTTI_MetaClass();

    Format(std::cout, "MetaClass<{0}> : {1}\n", metaClass->Name(), metaClass->Attributes());
    for (const auto& it : metaClass->Properties())
        Format(std::cout, "  - {0} : {1} -> {2}\n", it.first, it.second->Attributes(), it.second->TypeInfo());

    const RTTI::MetaProperty *prop = metaClass->PropertyIFP("_count");

    int value;
    prop->Cast<int>()->GetCopy(t, value);
    prop->Cast<int>()->SetMove(t, 42);

    RTTI::PMetaAtom atom = prop->WrapCopy(t);
    std::cout << *atom->Cast<int>()
        << " (" << atom->HashValue() << ")"
        << std::endl;

    prop = metaClass->PropertyIFP("_tities");
    atom = prop->WrapCopy(t);

    auto typedAtom = atom->Cast< VECTOR(Internal, PTiti) >();
    typedAtom->Wrapper().push_back(new Titi());

    prop->MoveFrom(t, typedAtom);

    //auto wrongAtom = atom->Cast< int >();
    auto wrongAtom2 = atom->As< int >();
    AssertRelease(!wrongAtom2);

    RTTI::MetaClassSingleton<Titi>::Destroy();
}

static void TestLexerParser_() {
    using namespace Core;

    Lexer::LexerStartup lexerStartup;
    Parser::ParserStartup parserStartup;

    Parser::Production< Parser::Enumerable<
        Tuple<  const Lexer::Match *,
        const Lexer::Match *,
        const Lexer::Match *>
    > > p =
    Parser::Expect(Lexer::Symbol::Int)
    .And(Parser::Expect(Lexer::Symbol::Add)
    .Or(Parser::Expect(Lexer::Symbol::Sub))
    .Or(Parser::Expect(Lexer::Symbol::Mul))
    .Or(Parser::Expect(Lexer::Symbol::Div)))
    .And(Parser::Expect(Lexer::Symbol::Int))
    .Many();

    VirtualFileSystemRoot& vfs = VirtualFileSystem::Instance();

    VirtualFileSystemComponentStartup processStartup(
        new VirtualFileSystemNativeComponent(L"Process:/", CurrentProcess::Instance().Directory())
        );

    const Filename inputFilename = L"Process:/parser.txt";
    const String nativeFilename = StringFormat("{0}", inputFilename);

    {
        auto oss = vfs.OpenWritable(inputFilename, AccessPolicy::Truncate);
        oss->WriteArray("(2 + 3 * 2) + 3 + 0xFF 3 * 8 055 / -7");
    }

    {
        RAWSTORAGE(Serializer, char) storage;
        vfs.ReadAll(inputFilename, storage);

        Lexer::Lexer lexer(storage.MakeConstView(), nativeFilename.c_str());
        Parser::ParseList input(&lexer);

        Parser::ParseContext globalContext(nullptr);

        try {
            Parser::PCParseStatement statement = Serialize::Grammar_Parse(input);

            Parser::ParseContext localContext(globalContext.Transaction(), &globalContext);
            statement->Invoke(&localContext);
        }
        catch (const Parser::ParserException& e) {
            if (e.Item())
                Format(std::cerr, "parser error : <{0}> {1}, {2}.\n", e.Item()->ToString(), e.what(), e.Site());
            else
                Format(std::cerr, "parser error : {0}, {1}.\n", e.what(), e.Site());
        }
        catch (const Lexer::LexerException& e) {
            Format(std::cerr, "lexer error : <{0}>: {1}, {2}.\n", e.Match().Symbol()->CStr(), e.what(), e.Match().Site());
        }
    }

    RTTI::MetaClassSingleton<Toto>::Create();
    {
        const RTTI::MetaClass& metaClass = RTTI::MetaClassSingleton<Toto>::Instance();
        Format(std::cout, "MetaClass<{0}> : {1}\n", metaClass.Name(), metaClass.Attributes());
        for (const auto& it : metaClass.Properties())
            Format(std::cout, "  - {0} : {1}, {2} -> {4} [{3}]\n",
            it.first,
            it.second->Attributes(),
            (uint64_t)it.second->TypeInfo().Flags,
            (uint64_t)it.second->TypeInfo().Id,
            it.second->TypeInfo());
    }

    Parser::ParseContext globalContext(new RTTI::MetaTransaction());

    do
    {
        std::cout << "$ ";

        char line[1024];
        std::cin.getline(line, lengthof(line));

        if (0 == CompareNI("exit", line, 5))
            break;

        try {
            Lexer::Lexer lexer(StringSlice(&line[0], Length(line)), "@in_memory");
            Parser::ParseList input(&lexer);

            Parser::PCParseItem item = Serialize::Grammar_Parse(input);
            AssertRelease(item);

            item->Invoke(&globalContext);
        }
        catch (const Parser::ParserException& e) {
            if (e.Item())
                Format(std::cerr, "parser error : <{0}> {1}, {2}.\n", e.Item()->ToString(), e.what(), e.Site());
            else
                Format(std::cerr, "parser error : {0}, {1}.\n", e.what(), e.Site());
        }
        catch (const Lexer::LexerException& e) {
            Format(std::cerr, "lexer error : <{0}>: {1}, {2}.\n", e.Match().Symbol()->CStr(), e.what(), e.Match().Site());
        }
    } while (true);

    RTTI::MetaClassSingleton<Toto>::Destroy();
}

static void TestTuple_() {
    using namespace Core;

    int i = 42;
    float f = 4.0f;

    Tuple<int, float> t = MergeTuple(i, f);

    auto k1 = MakeTuple(i);
    auto k2 = MakeTuple(f);
    auto k3 = MakeTuple(t);

    Tuple<int, float, float> t2 = MergeTuple(t, f);

    //Tuple<int, float, float> t2 = std::tuple_cat(MakeTuple(t), MakeTuple(f));
}

static void TestMaths_() {
    using namespace Core;

    float2 f;
    float2 u(1);
    float2 v = { 0, 1 };
    float3 w(0, 1, 2);

    w.x() = 41;

    w = (u + v).OneExtend();

    w = w.xzy();

    std::cout << w.OneExtend().Dehomogenize().yxyz() << std::endl;

    w = 1.0f + w;

    float3 x = Lerp(w, 2 * w, 0.5f);

    std::cout << (w == x) << std::endl;

    x = Saturate(x) + 1.0f;
    x = Rcp(x);

    RandomGenerator rand;
    x = Lerp(float2(-13), float2(42), rand.NextFloat01()).ZeroExtend();

    u32 i = rand.NextU32(10);
    u32 j = rand.NextU32(10, 42);

    float t = rand.NextFloat01();
    float s = rand.NextFloatM11();

    float4 z = Lerp(float4(-42), float4(69), NextRandFloat01<4>(rand));

    RandomGenerator rand2(RandomGenerator::RandomSeedTag{});
    t = rand2.NextFloat01();
    s = rand2.NextFloatM11();

    const AABB3f aabb = AABB3f::MinusOneOne();

    for (size_t c = 0; c < 10; ++c) {
        float3 n = NextRandFloatM11<3>(rand2);

        ScalarVector<u16, 3> q0 = QuantizeCeil<u16>(aabb, n);
        ScalarVector<u16, 3> q1 = QuantizeFloor<u16>(aabb, n);
        ScalarVector<u16, 3> q2 = QuantizeRound<u16>(aabb, n);

        float3 r0 = Unquantize(aabb, q0);
        float3 r1 = Unquantize(aabb, q1);
        float3 r2 = Unquantize(aabb, q2);

        AssertRelease(r0.AllGreaterOrEqual(n));
        AssertRelease(n.AllGreaterOrEqual(r1));
        AssertRelease(r0.AllGreaterOrEqual(r1));
    }

    float3x3 m = float3x3::Identity();
    float3 mv = m.Multiply(float3::One());

    float f11 = m.m11();
    float f20 = m.m20();

    float3x3 m2 = 1 + m;

    float4x4 mz = Make3DRotationMatrixAroundZ<float>(F_PIOver3);
    float4x4 mt = Make3DTransformMatrix(42 * float3::One(), 2.0f, float3::Y(), F_PIOver3);

    float4x4 mzi = Invert(mz);
    float4x4 mzii = Invert(mzi);

    float4x4 mti = Invert(mt);
    float4x4 mtii = Invert(mti);

    HalfFloat h = 1.234f;
    float hf = h;
    HalfFloat hc = hf;
    AssertRelease(hc == h);

    const float half_epsilon = HalfFloat::Epsilon.Unpack();

    half4 h4(half(1.0f), half(0.0f), half(-1.0f), half(0.234f));
    float4 h4f = HalfUnpack(h4);
    AssertRelease(h4f.x() == 1.0f);
    AssertRelease(h4f.y() == 0.0f);
    AssertRelease(h4f.z() == -1.0f);
    AssertRelease(std::abs(1 - h4f.w() / 0.234f) <= 0.001f);

    half2 hmin = half2::MinValue();
    half2 hmax = half2::MaxValue();
    float2 hminf = HalfUnpack(hmin);
    float2 hmaxf = HalfUnpack(hmax);

    float fg = 0.2345f;
    u8 ufg = Float01_to_UByte0255(fg);
    float fg2 = UByte0255_to_Float01(ufg);
    u8 ufg2 = Float01_to_UByte0255(fg2);
    AssertRelease(ufg == ufg2);

    fg = -0.2345f;
    ufg = FloatM11_to_UByte0255(fg);
    fg2 = UByte0255_to_FloatM11(ufg);
    ufg2 = FloatM11_to_UByte0255(fg2);
    AssertRelease(ufg == ufg2);

    float fi = 0.234567f;
    u16 ufi = Float01_to_UShort065535(fi);
    float fi2 = UShort065535_to_Float01(ufi);
    u16 ufi2 = Float01_to_UShort065535(fi2);
    AssertRelease(ufi == ufi2);

    fi = -0.234567f;
    ufi = FloatM11_to_UShort065535(fi);
    fi2 = UShort065535_to_FloatM11(ufi);
    ufi2 = FloatM11_to_UShort065535(fi2);
    AssertRelease(ufi == ufi2);

    float3 p0n = Normalize3(float3(-1,2,0.2f));
    half4 hp0n = HalfPack(p0n.OneExtend());
    float4 fhp0n = HalfUnpack(hp0n);

    UX10Y10Z10W2N p0;
    p0.Pack_FloatM11(p0n, 1);

    float3 p0_xyz;
    u8 p0_w;
    p0.Unpack_FloatM11(p0_xyz, p0_w);

    UX10Y10Z10W2N p1;
    p1.Pack_FloatM11(p0_xyz, p0_w);
    AssertRelease(p1 == p0);

    p0.Pack_FloatM11(float3(0,-1,1), 1);
    p0.Unpack_FloatM11(p0_xyz, p0_w);
    AssertRelease(p0_xyz.x() == 0);
    AssertRelease(p0_xyz.y() == -1);
    AssertRelease(p0_xyz.z() == 1);
    AssertRelease(p0_w == 1);

    byte4n sbn0 = float4(0,-1,1, 0.5f);

    float4 sbn0_xyzw = NormUnpack(sbn0);
    AssertRelease(sbn0_xyzw.x() == 0);
    AssertRelease(sbn0_xyzw.y() == -1);
    AssertRelease(sbn0_xyzw.z() == 1);

    byte4n sbn1 = SNormPack<u8>(sbn0_xyzw);
    AssertRelease(sbn1 == sbn0);

    ColorRGBAF rgb0 = Color::AliceBlue;
    ColorRGBAF hsl0 = RGB_to_HSL(rgb0.Data());
    ColorRGBAF rgb1 = HSL_to_RGB(hsl0.Data());
    AssertRelease(Length3(rgb0.Data() - rgb1.Data()) < 0.01f);

    rgb0 = Color::AliceBlue;
    ColorRGBAF hsv0 = RGB_to_HSV(rgb0.Data());
    rgb1 = HSV_to_RGB(hsv0.Data());
    AssertRelease(Length3(rgb0.Data() - rgb1.Data()) < 0.01f);
}

static void TestMatrices_() {
    using namespace Core;

#define DBG_MAT(_MAT) \
    Format(std::cout, "Matrix: \"{0}\" =\n" \
    "  {1:3f1}, {2:3f1}, {3:3f1}, {4:3f1}\n" \
    "  {5:3f1}, {6:3f1}, {7:3f1}, {8:3f1}\n" \
    "  {9:3f1}, {10:3f1}, {11:3f1}, {12:3f1}\n" \
    "  {13:3f1}, {14:3f1}, {15:3f1}, {16:3f1}\n" \
    "\n{17}\n", \
    STRINGIZE(_MAT), \
    (_MAT).data_()[0], (_MAT).data_()[1], (_MAT).data_()[2], (_MAT).data_()[3], \
    (_MAT).data_()[4], (_MAT).data_()[5], (_MAT).data_()[6], (_MAT).data_()[7], \
    (_MAT).data_()[8], (_MAT).data_()[9], (_MAT).data_()[10], (_MAT).data_()[11], \
    (_MAT).data_()[12], (_MAT).data_()[13], (_MAT).data_()[14], (_MAT).data_()[15], \
    (_MAT) )

    const float4x4 id = float4x4::Identity();
    DBG_MAT(id);

    float4x4 _1234 = id;
    _1234.SetDiagonal(float4(1,2,3,4));
    DBG_MAT(_1234);

    float4x4 c1c2c3c4;
    c1c2c3c4.SetColumn_x(float4(1));
    c1c2c3c4.SetColumn_y(float4(2));
    c1c2c3c4.SetColumn_z(float4(3));
    c1c2c3c4.SetColumn_w(float4(4));
    DBG_MAT(c1c2c3c4);

    float4x4 r1r2r3r4;
    r1r2r3r4.SetRow_x(float4(1));
    r1r2r3r4.SetRow_y(float4(2));
    r1r2r3r4.SetRow_z(float4(3));
    r1r2r3r4.SetRow_w(float4(4));
    DBG_MAT(r1r2r3r4);

    float4x4 a1a2a3a4 = id;
    a1a2a3a4.SetAxisX(float3(1));
    a1a2a3a4.SetAxisY(float3(2));
    a1a2a3a4.SetAxisZ(float3(3));
    a1a2a3a4.SetAxisT(float3(4));
    DBG_MAT(a1a2a3a4);

    float4x4 _016;
    _016._11() = 0;
    _016._12() = 1;
    _016._13() = 2;
    _016._14() = 3;
    _016._21() = 4;
    _016._22() = 5;
    _016._23() = 6;
    _016._24() = 7;
    _016._31() = 8;
    _016._32() = 9;
    _016._33() = 10;
    _016._34() = 11;
    _016._41() = 12;
    _016._42() = 13;
    _016._43() = 14;
    _016._44() = 15;
    DBG_MAT(_016);

    float4x4 _m016;
    _m016.m00() = 0;
    _m016.m01() = 1;
    _m016.m02() = 2;
    _m016.m03() = 3;
    _m016.m10() = 4;
    _m016.m11() = 5;
    _m016.m12() = 6;
    _m016.m13() = 7;
    _m016.m20() = 8;
    _m016.m21() = 9;
    _m016.m22() = 10;
    _m016.m23() = 11;
    _m016.m30() = 12;
    _m016.m31() = 13;
    _m016.m32() = 14;
    _m016.m33() = 15;
    DBG_MAT(_m016);

    float4x4 tr = MakeTranslationMatrix(float3(-1,-2,-3));
    DBG_MAT(tr);

    float4x4 rotX = Make3DRotationMatrixAroundX(F_PI);
    DBG_MAT(rotX);
    float4x4 rotY = Make3DRotationMatrixAroundY(F_PI);
    DBG_MAT(rotY);
    float4x4 rotZ = Make3DRotationMatrixAroundZ(F_PI);
    DBG_MAT(rotZ);

#undef DBG_MAT
}

static void TestTime_() {
    using namespace Core;

    Timeline time = Timeline::StartNow();

    const size_t seconds = 1;
    const size_t wantedTickCount = seconds*15;
    size_t actualTickCount = 0;

    do
    {
        Timespan elapased;
        if (time.Tick_Target15FPS(elapased)) {
            std::cout  << "Tick " << time.Total() << " !" << std::endl;
            ++actualTickCount;
        }
    }
    while (time.Total() < Units::Time::Seconds(double(seconds)) );

    std::cout << L"Total = " << Units::Time::Seconds(time.Total()) << " seconds -> "
        << actualTickCount << "/"
        << wantedTickCount << " ticks"
        << std::endl;
}

namespace Core {
    struct VertexPositionTexCoordNormalTangent {
        float3  Position;
        ushort2 TexCoord;
        ubyte4  Normal;
        ubyte4  Tangent;

        static void CreateVertexDeclaration(Graphics::VertexDeclaration *vertexDecl) {
            vertexDecl->AddTypedSubPart<Graphics::VertexSubPartSemantic::Position>(&VertexPositionTexCoordNormalTangent::Position, 0);
            vertexDecl->AddTypedSubPart<Graphics::VertexSubPartSemantic::TexCoord>(&VertexPositionTexCoordNormalTangent::TexCoord, 0);
            vertexDecl->AddTypedSubPart<Graphics::VertexSubPartSemantic::Normal>(&VertexPositionTexCoordNormalTangent::Normal, 0);
            vertexDecl->AddTypedSubPart<Graphics::VertexSubPartSemantic::Tangent>(&VertexPositionTexCoordNormalTangent::Tangent, 0);
            vertexDecl->Freeze();
        }
    };
}

void TestGraphics_() {
    using namespace Core;

    Graphics::VertexDeclaration vertexDecl;
    vertexDecl.AddSubPart<Graphics::VertexSubPartFormat::Float3, Graphics::VertexSubPartSemantic::Position>(0);
    vertexDecl.AddSubPart<Graphics::VertexSubPartFormat::UShort2, Graphics::VertexSubPartSemantic::TexCoord>(0);
    vertexDecl.AddSubPart<Graphics::VertexSubPartFormat::UByte4, Graphics::VertexSubPartSemantic::Normal>(0);
    vertexDecl.AddSubPart<Graphics::VertexSubPartFormat::UByte4, Graphics::VertexSubPartSemantic::Tangent>(0);
    vertexDecl.Freeze();

    String str = vertexDecl.ToString();
    size_t size = vertexDecl.SizeInBytes();

    Pair<const Graphics::VertexSubPartKey *, const Graphics::AbstractVertexSubPart *> it =
        vertexDecl.SubPartBySemantic(Graphics::VertexSubPartSemantic::Position, 0);
    AssertRelease(it.second);
    AssertRelease(it.first->Format() == Graphics::VertexSubPartFormat::Float3);

    it = vertexDecl.SubPartBySemantic(Graphics::VertexSubPartSemantic::Tangent, 0);
    AssertRelease(it.second);
    AssertRelease(it.first->Format() == Graphics::VertexSubPartFormat::UByte4);

    const auto *subpart = vertexDecl.TypedSubPart<Graphics::VertexSubPartFormat::UByte4>(Graphics::VertexSubPartSemantic::Normal, 0);

    Graphics::VertexDeclaration vertexDecl2;
    VertexPositionTexCoordNormalTangent::CreateVertexDeclaration(&vertexDecl2);

    String str2 = vertexDecl2.ToString();
    size_t size2 = vertexDecl2.SizeInBytes();

    VertexPositionTexCoordNormalTangent v;

    vertexDecl2.TypedSubPart<Graphics::VertexSubPartFormat::Float3>(Graphics::VertexSubPartSemantic::Position, 0)->TypedSet(&v, float3(1,2,3));
    vertexDecl2.TypedSubPart<Graphics::VertexSubPartFormat::UShort2>(Graphics::VertexSubPartSemantic::TexCoord, 0)->TypedSet(&v, ushort2(16,1024));
    vertexDecl2.TypedSubPart<Graphics::VertexSubPartFormat::UByte4>(Graphics::VertexSubPartSemantic::Normal, 0)->TypedSet(&v, ubyte4(0,0,0xFF,0));

    VertexPositionTexCoordNormalTangent v2;
    vertexDecl2.CopyVertex(&v2, &v, sizeof(VertexPositionTexCoordNormalTangent));
    AssertRelease(v.Position == v2.Position);
    AssertRelease(v.TexCoord == v2.TexCoord);
    AssertRelease(v.Normal == v2.Normal);

    const Graphics::VertexDeclaration *vdecl2 = Graphics::Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration;
    const auto autoName = vdecl2->ToString();

    Graphics::Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N vvv;
    vvv.Position0 = float3(1,2,3);
    vvv.TexCoord0 = HalfPack(float2(1100, -12345));
    vvv.Normal0.Pack_FloatM11(float3(-1,0,0), 0);
    vvv.Tangent0.Pack_FloatM11(float3(0,1,0), 0);

    float3 pos0 = vdecl2->TypedSubPart<Graphics::VertexSubPartFormat::Float3>(Graphics::VertexSubPartSemantic::Position, 0)->TypedGet(&vvv);
    half2 tex0 = vdecl2->TypedSubPart<Graphics::VertexSubPartFormat::Half2>(Graphics::VertexSubPartSemantic::TexCoord, 0)->TypedGet(&vvv);
    UX10Y10Z10W2N normal0 = vdecl2->TypedSubPart<Graphics::VertexSubPartFormat::UX10Y10Z10W2N>(Graphics::VertexSubPartSemantic::Normal, 0)->TypedGet(&vvv);
    UX10Y10Z10W2N tangent0 = vdecl2->TypedSubPart<Graphics::VertexSubPartFormat::UX10Y10Z10W2N>(Graphics::VertexSubPartSemantic::Tangent, 0)->TypedGet(&vvv);
    AssertRelease(pos0 == vvv.Position0);
    AssertRelease(tex0 == vvv.TexCoord0);
    AssertRelease(normal0 == vvv.Normal0);
    AssertRelease(tangent0 == vvv.Tangent0);

    const Graphics::SurfaceFormat *fmt0 = Graphics::SurfaceFormat::D24S8;
    AssertRelease(fmt0->IsDepth());
    AssertRelease(fmt0->IsStencil());
    AssertRelease(fmt0->BitsPerPixels() == 32);
    AssertRelease(fmt0->Pitch() == 4);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    fmt0 = Graphics::SurfaceFormat::R8G8B8A8_SRGB;
    AssertRelease(fmt0->IsRGB());
    AssertRelease(fmt0->IsAlpha());
    AssertRelease(fmt0->IsGammaSpace());
    AssertRelease(fmt0->BitsPerPixels() == 32);
    AssertRelease(fmt0->Pitch() == 4);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    /* TODO
    fmt0 = Graphics::SurfaceFormat::DXT5_SRGB;
    AssertRelease(fmt0->RGB());
    AssertRelease(fmt0->Alpha());
    AssertRelease(fmt0->GammaSpace());
    AssertRelease(fmt0->DXTC());
    AssertRelease(fmt0->BitsPerPixels() == 8);
    AssertRelease(fmt0->Pitch() == 4);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 4);
    */

    fmt0 = Graphics::SurfaceFormat::R32G32_F;
    AssertRelease(fmt0->IsRA());
    AssertRelease(fmt0->IsFloatingPoint());
    AssertRelease(fmt0->BitsPerPixels() == 64);
    AssertRelease(fmt0->Pitch() == 8);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    fmt0 = Graphics::SurfaceFormat::R16G16B16A16_F;
    AssertRelease(fmt0->IsRGB());
    AssertRelease(fmt0->IsAlpha());
    AssertRelease(fmt0->IsFloatingPoint());
    AssertRelease(fmt0->BitsPerPixels() == 64);
     AssertRelease(fmt0->Pitch() == 8);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    fmt0 = Graphics::SurfaceFormat::R32G32B32A32;
    AssertRelease(fmt0->IsRGB());
    AssertRelease(fmt0->IsAlpha());
    AssertRelease(!fmt0->IsFloatingPoint());
    AssertRelease(fmt0->BitsPerPixels() == 128);
    AssertRelease(fmt0->Pitch() == 16);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    const Graphics::BlendState *blend = Graphics::BlendState::AlphaBlend;
    AssertRelease(blend->ColorSourceBlend() == Graphics::Blend::One);
    AssertRelease(blend->AlphaDestinationBlend() == Graphics::Blend::InverseSourceAlpha);
    AssertRelease(blend->Frozen());

    const Graphics::DepthStencilState *depth = Graphics::DepthStencilState::Default;
    AssertRelease(depth->DepthBufferEnabled());
    AssertRelease(depth->DepthBufferFunction() == Graphics::CompareFunction::LessEqual);

    const Graphics::RasterizerState *rasterizer = Graphics::RasterizerState::CullCounterClockwise;
    AssertRelease(rasterizer->CullMode() == Graphics::CullMode::CullCounterClockwiseFace);
    AssertRelease(rasterizer->MultiSampleAntiAlias());
    AssertRelease(!rasterizer->ScissorTestEnabled());

    ColorRGBA col0 = Color::LimeGreen;
    col0 = Color::LightSeaGreen;
    ColorRGBA16 col16 = col0;
    ColorRGBAF colF = col0;
    ColorBGRA col2 = col16;
    colF = colF.ToSRGB();
    colF = colF.ToLinear();
}

#include "Core.Engine/Input/State/KeyboardKey.h"
#include "Core.Engine/Input/State/MouseButton.h"

void TestInputs_() {
    using namespace Core;
    using namespace Core::Engine;

    std::cout << "static const u8 gVirtualKey_to_KeyboardKey[0xFF] = {" << std::endl;

    u8 virtualKeyToKeyboardKey[0xFF];
    memset(virtualKeyToKeyboardKey, 0xFF, sizeof(virtualKeyToKeyboardKey));

    virtualKeyToKeyboardKey[VK_NUMPAD0] = u8(KeyboardKey::Numpad0);
    virtualKeyToKeyboardKey[VK_NUMPAD1] = u8(KeyboardKey::Numpad1);
    virtualKeyToKeyboardKey[VK_NUMPAD2] = u8(KeyboardKey::Numpad2);
    virtualKeyToKeyboardKey[VK_NUMPAD3] = u8(KeyboardKey::Numpad3);
    virtualKeyToKeyboardKey[VK_NUMPAD4] = u8(KeyboardKey::Numpad4);
    virtualKeyToKeyboardKey[VK_NUMPAD5] = u8(KeyboardKey::Numpad5);
    virtualKeyToKeyboardKey[VK_NUMPAD6] = u8(KeyboardKey::Numpad6);
    virtualKeyToKeyboardKey[VK_NUMPAD7] = u8(KeyboardKey::Numpad7);
    virtualKeyToKeyboardKey[VK_NUMPAD8] = u8(KeyboardKey::Numpad8);
    virtualKeyToKeyboardKey[VK_NUMPAD9] = u8(KeyboardKey::Numpad9);

    virtualKeyToKeyboardKey[VK_ADD] = u8(KeyboardKey::Add);
    virtualKeyToKeyboardKey[VK_SUBTRACT] = u8(KeyboardKey::Substract);
    virtualKeyToKeyboardKey[VK_MULTIPLY] = u8(KeyboardKey::Multiply);
    virtualKeyToKeyboardKey[VK_DIVIDE] = u8(KeyboardKey::Divide);

    virtualKeyToKeyboardKey[VK_F1] = u8(KeyboardKey::F1);
    virtualKeyToKeyboardKey[VK_F2] = u8(KeyboardKey::F2);
    virtualKeyToKeyboardKey[VK_F3] = u8(KeyboardKey::F3);
    virtualKeyToKeyboardKey[VK_F4] = u8(KeyboardKey::F4);
    virtualKeyToKeyboardKey[VK_F5] = u8(KeyboardKey::F5);
    virtualKeyToKeyboardKey[VK_F6] = u8(KeyboardKey::F6);
    virtualKeyToKeyboardKey[VK_F7] = u8(KeyboardKey::F7);
    virtualKeyToKeyboardKey[VK_F8] = u8(KeyboardKey::F8);
    virtualKeyToKeyboardKey[VK_F9] = u8(KeyboardKey::F9);
    virtualKeyToKeyboardKey[VK_F10] = u8(KeyboardKey::F10);
    virtualKeyToKeyboardKey[VK_F11] = u8(KeyboardKey::F11);
    virtualKeyToKeyboardKey[VK_F12] = u8(KeyboardKey::F12);

    virtualKeyToKeyboardKey[VK_UP] = u8(KeyboardKey::Up);
    virtualKeyToKeyboardKey[VK_DOWN] = u8(KeyboardKey::Down);
    virtualKeyToKeyboardKey[VK_LEFT] = u8(KeyboardKey::Left);
    virtualKeyToKeyboardKey[VK_RIGHT] = u8(KeyboardKey::Right);

    virtualKeyToKeyboardKey[VK_ESCAPE] = u8(KeyboardKey::Escape);
    virtualKeyToKeyboardKey[VK_SPACE] = u8(KeyboardKey::Space);

    virtualKeyToKeyboardKey[VK_PAUSE] = u8(KeyboardKey::Pause);
    virtualKeyToKeyboardKey[VK_PRINT] = u8(KeyboardKey::PrintScreen);
    virtualKeyToKeyboardKey[VK_SCROLL] = u8(KeyboardKey::ScrollLock);

    virtualKeyToKeyboardKey[VK_BACK] = u8(KeyboardKey::Backspace);
    virtualKeyToKeyboardKey[VK_RETURN] = u8(KeyboardKey::Enter);
    virtualKeyToKeyboardKey[VK_TAB] = u8(KeyboardKey::Tab);

    virtualKeyToKeyboardKey[VK_HOME] = u8(KeyboardKey::Home);
    virtualKeyToKeyboardKey[VK_END] = u8(KeyboardKey::End);
    virtualKeyToKeyboardKey[VK_INSERT] = u8(KeyboardKey::Insert);
    virtualKeyToKeyboardKey[VK_DELETE] = u8(KeyboardKey::Delete);
    virtualKeyToKeyboardKey[VK_PRIOR] = u8(KeyboardKey::PageUp);
    virtualKeyToKeyboardKey[VK_NEXT] = u8(KeyboardKey::PageDown);

    virtualKeyToKeyboardKey[VK_MENU] = u8(KeyboardKey::Alt);
    virtualKeyToKeyboardKey[VK_APPS] = u8(KeyboardKey::Menu);
    virtualKeyToKeyboardKey[VK_SHIFT] = u8(KeyboardKey::Control);
    virtualKeyToKeyboardKey[VK_CONTROL] = u8(KeyboardKey::Shift);

    for (u8 i = 'A'; i <= 'Z'; ++i) {
        AssertRelease(u8(KeyboardKey::A) + i - 'A' == i);
        virtualKeyToKeyboardKey[i] = i;
    }
    for (u8 i = '0'; i <= '9'; ++i) {
        AssertRelease(u8(KeyboardKey::_0) + i - '0' == i);
        virtualKeyToKeyboardKey[i] = i;
    }

    for (size_t i = 0; i < 0xFF; ++i)
        if (virtualKeyToKeyboardKey[i] == 0xFF)
            std::cout << "    0xFF," << std::endl;
        else
            std::cout << "    u8(KeyboardKey::" << KeyboardKeyToCStr(KeyboardKey(virtualKeyToKeyboardKey[i])) << ")," << std::endl;

    std::cout << "};" << std::endl;
}

void Tests() {
    using namespace Core;

    MemoryTrackingData scopeTrackingData;
    std::vector<int, TrackingAllocator< Mallocator<int>> > u{ TrackingAllocator<Mallocator<int>>(scopeTrackingData) };
    u.push_back(1);
    u.push_back(2);
    u.push_back(3);

    {
        //SKIP_MEMORY_LEAKS_IN_SCOPE();
        new int;
    }

    {
        VECTOR_THREAD_LOCAL(Container, int) z = { 1, 4, 5, 3, 2, 0 };
        std::sort(z.begin(), z.end());
        std::cout << MakeView(z) << std::endl;
    }

    {
        RAWSTORAGE_THREAD_LOCAL(Container, float3) q(32);
        q[0] = float3(1,2,3);
        q.Resize_DiscardData(16);
        q[0] = float3(1,2,3);
        q.Resize_KeepData(18);
        size_t s = q.SizeInBytes();
    }

    {
        std::vector<int, Allocator<int> > v{ 4, 5, 6 };
        v.push_back(2);
        v.insert(v.end(), u.begin(), u.end());
        v.reserve(9);

        //Print(MemoryTrackingData::Global());

        Vector<int> w(v.begin(), v.end());

        //Print(MemoryTrackingData::Global());

        STACKLOCAL_POD_STACK(uint8_t, memoryStack, 30000);
        {
            typedef DECORATE_ALLOCATOR(Internal, StackAllocator<int>) allocator_type;
            std::vector<int, allocator_type> x(w.begin(), w.end(), allocator_type(memoryStack));
            x.clear();
            x.reserve(42);
        }
        const auto memoryView1 = MALLOCA_VIEW(uint8_t, 300);
        auto memoryStack1 = MemoryStack<uint8_t>(memoryView1);
        {
            typedef DECORATE_ALLOCATOR(Internal, StackAllocator<int>) allocator_type;
            std::vector<int, allocator_type> x(w.begin(), w.end(), allocator_type(memoryStack1));
            x.clear();
            x.reserve(42);
        }
        auto memoryStack2 = StaticStack<uint8_t, 300>();
        {
            typedef DECORATE_ALLOCATOR(Internal, StackAllocator<int>) allocator_type;
            std::vector<int, allocator_type> x(w.begin(), w.end(), allocator_type(memoryStack2));
            x.clear();
            x.reserve(42);
        }

        //Print(MemoryTrackingData::Global());
    }

    {
        Heap& heap = Heaps::Process::Instance();
        void* p = heap.malloc<16>(100);

        CrtMemoryStats memStats;
        CrtDumpMemoryStats(&memStats, heap.Handle());
        Print(memStats);

        typedef DECORATE_ALLOCATOR(Internal, HeapAllocator<int>) allocator_type;
        std::vector<int, allocator_type> x(u.begin(), u.end());
        x.reserve(1024);

        std::cout << "Heap size = " << heap.Size() << std::endl;

        CrtDumpMemoryStats(&memStats, heap.Handle());
        Print(memStats);

        heap.free<16>(p);
    }

    {
        Heap heap{ "stack", false };
        void* p = heap.malloc<16>(100);

        std::cout << "Heap size = " << heap.Size() << std::endl;

        CrtMemoryStats memStats;
        CrtDumpMemoryStats(&memStats, heap.Handle());
        Print(memStats);

        heap.free<16>(p);
    }

    TestCallstack_();

    TestStrings_();

    TestHash_();

    TestPointers_();

    TestThreads_();

    TestTokens_();

    //TestRTTI_();

    TestLexerParser_();

    TestTuple_();

    TestMaths_();

    TestMatrices_();

    TestTime_();

    TestGraphics_();

    TestInputs_();

    TestFileSystem_();

    TestVirtualFileSystem_();
}

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ApplicationTest::ApplicationTest()
:   ApplicationConsole(L"ApplicationTest") {}
//----------------------------------------------------------------------------
ApplicationTest::~ApplicationTest() {}
//----------------------------------------------------------------------------
void ApplicationTest::Start() {
    ApplicationConsole::Start();
    Tests();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
