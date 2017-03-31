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
#include "Core/IO/StringView.h"
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
#include "Core.RTTI/RTTI_Macros-impl.h"
#include "Core.RTTI/RTTI_Macros.h"

#include "Core.Serialize/Lexer/Lexer.h"
#include "Core.Serialize/Lexer/Symbols.h"
#include "Core.Serialize/Parser/Parser.h"

#include "Core/Container/Tuple.h"

#include "Core.Serialize/Text/Grammar.h"

#include "Core/Color/Color.h"

#include "Core/Time/Timeline.h"

#include "Core/Diagnostic/Logger.h"

#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarBoundingBoxHelpers.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/PackedVectors.h"
#include "Core/Maths/PackingHelpers.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarMatrixHelpers.h"
#include "Core/Maths/Units.h"

#include "Core.Graphics/Device/VertexDeclaration.h"
#include "Core.Graphics/Device/State/BlendState.h"
#include "Core.Graphics/Device/State/DepthStencilState.h"
#include "Core.Graphics/Device/State/RasterizerState.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"
#include "Core.Graphics/Graphics.h"

#include <iostream>
#include <vector>

#ifdef PLATFORM_WINDOWS
#   include <Windows.h>
#else
#   error "no support"
#endif

static void Print(const Core::FCrtMemoryStats& memoryStats) {
    std::cerr << "Memory statistics :" << eol
        << " - Total free size          = " << memoryStats.TotalFreeSize << eol
        << " - Largest free block       = " << memoryStats.LargestFreeBlockSize << eol
        << " - Total used size          = " << memoryStats.TotalUsedSize << eol
        << " - Largest used block       = " << memoryStats.LargestUsedBlockSize << eol
        << " - Total overhead size      = " << memoryStats.TotalOverheadSize << eol
        << " - Total comitted size      = " << Core::SizeInBytes{ memoryStats.TotalOverheadSize.Value + memoryStats.TotalFreeSize.Value + memoryStats.TotalUsedSize.Value } << eol
        << " - External fragmentation   = " << (memoryStats.ExternalFragmentation() * 100) << "%" << eol;
}

void TestCallstack_() {
    using namespace Core;

    FCallstack callstack{ 2, 32 };
    FDecodedCallstack decoded{ callstack };

    std::cout << "FCallstack[" << decoded.Depth() << "] #"
        << std::hex << decoded.Hash() << std::dec
        << eol;

    std::cout << decoded << eol;
}

void TestStrings_() {
    using namespace Core;

    wchar_t buffer[1024];
    Format(buffer, L"string = {2}, decimal = {0}, float = {1}\n", "test", 42, 0.123456f);
    std::cout << buffer;

    FWString wstr = StringFormat(L"num={0} alphabool={0:a}", true);

    const char* test = "titi,toto,,tata,,";

    const char* s = test;
    FStringView p;
    while (Split(&s, ',', p))
        std::cout << p << eol;

    Format(std::cout, "string = {0:10U} {0:-10U}, decimal = {1:8X} {1:#8x}, float = {2:f3} {2:10f4}\n", "test", 0xBADCAFE, -0.123456f);

    Format(std::cout, "{0*16}\n", '-');

    Format(std::cout, "{0:#4*4}\n", 42);
}

void TestHash_() {
    using namespace Core;

    static u32 numbers[6] = {9,18,27,36,45,54};

    size_t h0 = hash_value(numbers);
    size_t h1 = hash_value(numbers[0],numbers[1],numbers[2],numbers[3],numbers[4],numbers[5]);
    size_t h2 = hash_range(&numbers[0],&numbers[6]);

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

class A : public Core::FRefCountable {
public:
    A() {}
};

class B : public Core::FRefCountable {
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

    TRefPtr<A> a = new A();
    TRefPtr<C> c = new C();

    TRefPtr<B> b = c;
    AssertRelease(b == c);
    AssertRelease(a != b);

    std::cout << c->Value() << eol;
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

    FThreadContext& context = FThreadLocalContext_::Instance();

    std::cout << context.Name() << ", " << context.Tag() << " = " << context.Id() << eol;

    LockFreeCircularQueue_SingleProducer<const wchar_t*> queue(128);

    queue.Produce(L"FToto");

    const wchar_t* str = nullptr;
    if (!queue.Consume(str))
        AssertRelease(0);

    std::cout << str << eol;

    {
        TaskEvaluator evaluator("GlobalEvaluator", std::thread::hardware_concurrency(), 90);
        evaluator.Start();

        TaskCompletionPort completionPort("Test", &evaluator);

        const PTask task = new LambdaTask([](const TaskContext& ctx) {
            std::cout   << "[" << ctx.Worker()->ThreadIndex() << "] "
                        << fib(15) << "!" << eol;
            return TaskResult::Succeed;
        });

        for (size_t i = 0; i < 100; ++i)
            completionPort.Produce(task);

        completionPort.WaitAll();
        std::cout << "done!" << eol;

        for (size_t i = 0; i < 100; ++i)
            completionPort.Produce(task);

        while (completionPort.WaitOne())
            std::cout << "done!" << eol;

        evaluator.Shutdown();
    }
}

struct FTestToken {};

class FTestToken2 : public Core::TToken<FTestToken2, char, Core::CaseSensitive::False> {
};

void TestTokens_() {
    using namespace Core;

    typedef TToken<FTestToken, char, CaseSensitive::False> token_type;
    token_type::FStartup tokenStartup(32);

    token_type t("toto");
    token_type v("FToto");
    AssertRelease(t == v);

    v = "tata";
    AssertRelease(t != v);
    v = t;
    AssertRelease(t == v);

    token_type e;
    AssertRelease(e != v);
    AssertRelease(e != t);

    token_type q = "";

    THash<token_type> h;
    std::cout << h(t) << eol;
    std::cout << h(v) << eol;
    std::cout << h(e) << eol;
    std::cout << h(q) << eol;
}

void TestFileSystem_() {
    using namespace Core;

    FExtname ext = L".jpg";

    std::cout << ext << " (" << hash_value(ext) << ")" << eol;
    std::cout << FExtname(L".jPG") << eol;

    FExtname e{ FWString(L".Jpg") };
    AssertRelease(e == ext);
    AssertRelease(e.c_str() == ext.c_str());
    std::cout << e << " (" << hash_value(e) << ")" << eol;

    std::cout
        << "Size of filesystem classes :" << eol
        << " * FMountingPoint    = " << sizeof(FMountingPoint)    << eol
        << " * FDirpath          = " << sizeof(FDirpath)          << eol
        << " * FBasename         = " << sizeof(FBasename)         << eol
        << " * FExtname          = " << sizeof(FExtname)          << eol
        << " * FFilename         = " << sizeof(FFilename)         << eol
        << " * TVector<int>      = " << sizeof(std::vector<int>) << eol
        << " * TVector<FDirname>  = " << sizeof(std::vector<FDirname>) << eol
        << " * TRawStorage<int>  = " << sizeof(RAWSTORAGE(FTask, int)) << eol
        << eol;

    FMountingPoint mount(L"Data:");
    FDirpath path = { L"Data:", L"Assets", L"3D", L"Models" };
    FBasename basename(L"Test", L".JPG");
    FFilename filename{ path, basename };

    std::cout << filename << " (" << hash_value(filename) << ")" << eol;

    FFilename f = L"data:/assets/3d/models/test.jpg";
    std::cout << f << " (" << hash_value(f) << ")" << eol;
    f = L"test.jpg";
    std::cout << f << " (" << hash_value(f) << ")" << " (basename: " << hash_value(f.Basename()) << ")" << eol;
    f = L"models/test.jpg";
    std::cout << f << " (" << hash_value(f) << ")" << eol;
    f = L"data:/assets/3d/models/";
    std::cout << f << " (" << hash_value(f) << ")" << eol;
    f = L"DATA:/ASSETS/3D/MODELS/TEST.JPG";
    std::cout << f << " (" << hash_value(f) << ")" << eol;

    std::cout << f.Basename() << hash_value(f.Basename()) << eol;
    std::cout << basename << hash_value(basename) << eol;

    std::cout << f.BasenameNoExt() << hash_value(f.BasenameNoExt()) << eol;
    std::cout << basename.BasenameNoExt() << hash_value(basename.BasenameNoExt()) << eol;

    std::cout << f.Extname() << hash_value(f.Extname()) << eol;
    std::cout << basename.Extname() << hash_value(basename.Extname()) << eol;

    AssertRelease(hash_value(f) == hash_value(filename));
    AssertRelease(hash_value(e) == hash_value(filename.Extname()));
    AssertRelease(hash_value(e) == hash_value(basename.Extname()));

    FDirpath p = L"data:/assets/3d";
    std::cout << p << " (" << hash_value(p) << ")" << eol;
    p = L"data:";
    std::cout << p << " (" << hash_value(p) << ")" << eol;
    p = L"data:/assets";
    std::cout << p << " (" << hash_value(p) << ")" << eol;
}

void TestVirtualFileSystem_() {
    using namespace Core;

    VirtualFileSystemRoot& vfs = FVirtualFileSystem::Instance();

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
        std::cout << read.Pointer() << eol;
    }

    if (!vfs.FileExists(L"procesS:/tEsT.tXT"))
        AssertRelease(false);

    vfs.EnumerateFiles(L"process:/", true, [](const Core::FFilename& filename) {
        std::cout << filename << eol;
    });
}

FWD_REFPTR(Titi);
class FTiti : public Core::RTTI::FMetaObject {
public:
    FTiti() {}
    virtual ~FTiti() {}
    RTTI_CLASS_HEADER(FTiti, void);
private:
    int _count;
    Core::FString _name;
    VECTOR(Internal, PTiti) _tities;
};
RTTI_CLASS_BEGIN(FTiti, Concrete)
RTTI_PROPERTY_FIELD(_count)
RTTI_PROPERTY_FIELD_ALIAS(_name, FName)
RTTI_PROPERTY_FIELD(_tities)
RTTI_CLASS_END()

FWD_REFPTR(Toto);
class FToto : public Core::RTTI::FMetaObject {
public:
    FToto() {}
    virtual ~FToto() {}
    RTTI_CLASS_HEADER(FToto, void);
private:
    int _count;
    Core::FString _name;
    VECTOR(Internal, PTiti) _tities;
    VECTOR_THREAD_LOCAL(Internal, Core::FString) _titles;
    Core::TRefPtr<FToto> _parent;
    Core::TPair<int, int> _pair;
    Core::TPair<float, float> _fpair;
    Core::TPair<Core::TPair<int, int>, Core::TPair<int, int> > _vpair;
    Core::TPair<Core::TPair<int, int>, Core::TPair<int, float> > _vpair2;
    HASHMAP_THREAD_LOCAL(Internal, Core::FString, float) _dict;
    HASHMAP_THREAD_LOCAL(Internal, Core::FString, Core::TPair<int COMMA Core::TPair<float COMMA Core::FString>>) _dict2;
};
RTTI_CLASS_BEGIN(FToto, Concrete)
RTTI_PROPERTY_FIELD_ALIAS(_count, Count)
RTTI_PROPERTY_FIELD_ALIAS(_name, FName)
RTTI_PROPERTY_FIELD_ALIAS(_tities, Tities)
RTTI_PROPERTY_FIELD_ALIAS(_titles, Titles)
RTTI_PROPERTY_FIELD_ALIAS(_parent, Parent)
RTTI_PROPERTY_FIELD_ALIAS(_pair, TPair)
RTTI_PROPERTY_FIELD_ALIAS(_fpair, FPair)
RTTI_PROPERTY_FIELD_ALIAS(_vpair, VPair)
RTTI_PROPERTY_FIELD_ALIAS(_vpair2, VPair2)
RTTI_PROPERTY_FIELD_ALIAS(_dict, Dict)
RTTI_PROPERTY_FIELD_ALIAS(_dict2, Dict2)
RTTI_CLASS_END()

template <typename T>
static void TestRTTIWrap_() {
    typedef Core::RTTI::TMetaTypeTraits< T > type_traits;
    T wrapped;
    typename type_traits::wrapper_type wrapper;
    type_traits::WrapCopy(wrapper, wrapped);
    type_traits::WrapMove(wrapper, std::move(wrapped));
    type_traits::UnwrapCopy(wrapped, wrapper);
    type_traits::UnwrapMove(wrapped, std::move(wrapper));
}

static void TestRTTI_() {
    using namespace Core;

    Format(std::cout, "Id = {0}, FName = {1}, Default = {2}, EFlags = {3}\n",
        RTTI::TMetaType< int32_t >::Id(),
        RTTI::TMetaType< int32_t >::Name(),
        RTTI::TMetaType< int32_t >::DefaultValue(),
        (size_t)RTTI::TMetaType< int32_t >::EFlags()
        );

    Format(std::cout, "Id = {0}, FName = {1}, Default = {2}, EFlags = {3}\n",
        RTTI::TMetaType< int >::Id(),
        RTTI::TMetaType< int >::Name(),
        RTTI::TMetaType< int >::DefaultValue(),
        (size_t)RTTI::TMetaType< int >::EFlags()
        );

    Format(std::cout, "Id = {0}, FName = {1}, Default = {2}, EFlags = {3}\n",
        RTTI::TMetaType< size_t >::Id(),
        RTTI::TMetaType< size_t >::Name(),
        RTTI::TMetaType< size_t >::DefaultValue(),
        (size_t)RTTI::TMetaType< size_t >::EFlags()
        );

    Format(std::cout, "Id = {0}, FName = {1}, Default = {2}, EFlags = {3}\n",
        RTTI::TMetaType< FString >::Id(),
        RTTI::TMetaType< FString >::Name(),
        RTTI::TMetaType< FString >::DefaultValue(),
        (size_t)RTTI::TMetaType< FString >::EFlags()
        );

    Format(std::cout, "Id = {0}, FName = {1}, Default = {2}, EFlags = {3}\n",
        RTTI::TMetaType< VECTOR(RTTI, int) >::Id(),
        RTTI::TMetaType< VECTOR(RTTI, int) >::Name(),
        RTTI::TMetaType< VECTOR(RTTI, int) >::DefaultValue(),
        (size_t)RTTI::TMetaType< VECTOR(RTTI, int) >::EFlags()
        );

    Format(std::cout, "Id = {0}, FName = {1}, Default = {2}, EFlags = {3}\n",
        RTTI::TMetaType< TPair<float, int> >::Id(),
        RTTI::TMetaType< TPair<float, int> >::Name(),
        RTTI::TMetaType< TPair<float, int> >::DefaultValue(),
        (size_t)RTTI::TMetaType< TPair<float, int> >::EFlags()
        );

    RTTI::TMetaTypeTraits< int >::meta_type::Name();

    int i = 0;
    RTTI::TMetaTypeTraits< int >::UnwrapCopy(i, 42);

    RTTI::PMetaObject o;
    RTTI::TMetaTypeTraits< RTTI::PMetaObject >::UnwrapCopy(o, nullptr);

    PTiti t;
    RTTI::TMetaTypeTraits< PTiti >::UnwrapCopy(t, nullptr);
    RTTI::TMetaTypeTraits< PTiti >::WrapMove(o, std::move(t));

    TestRTTIWrap_< PTiti >();
    TestRTTIWrap_< TPair<FWString, PTiti> >();
    TestRTTIWrap_< VECTOR_THREAD_LOCAL(RTTI, PTiti) >();
    TestRTTIWrap_< HASHMAP(RTTI, int, int) >();
    TestRTTIWrap_< HASHMAP(RTTI, FString, PTiti) >();
    TestRTTIWrap_< ASSOCIATIVE_VECTOR(RTTI, FString, PTiti) >();
    //TestRTTIWrap_< HASHSET(RTTI, FString) >();

    RTTI::TMetaClassSingleton<FTiti>::Create();

    t = new FTiti();
    const RTTI::FMetaClass *metaClass = t->RTTI_MetaClass();

    Format(std::cout, "TMetaClass<{0}> : {1}\n", metaClass->Name(), metaClass->Attributes());
    RTTI::ForEachProperty(metaClass, [](const FMetaClass* metaClass, const FMetaProperty* prop) {
        Format(std::cout, "  - {0} : {1} -> {2}\n", prop->Name(), prop->Attributes(), prop->TypeInfo());
    });

    const RTTI::FMetaProperty *prop = metaClass->PropertyIFP("_count");

    int value;
    prop->Cast<int>()->GetCopy(t, value);
    prop->Cast<int>()->SetMove(t, 42);

    RTTI::PMetaAtom atom = prop->WrapCopy(t);
    std::cout << *atom->Cast<int>()
        << " (" << atom->HashValue() << ")"
        << eol;

    prop = metaClass->PropertyIFP("_tities");
    atom = prop->WrapCopy(t);

    auto typedAtom = atom->Cast< VECTOR(Internal, PTiti) >();
    typedAtom->Wrapper().push_back(new FTiti());

    prop->MoveFrom(t, typedAtom);

    //auto wrongAtom = atom->Cast< int >();
    auto wrongAtom2 = atom->As< int >();
    AssertRelease(!wrongAtom2);

    RTTI::TMetaClassSingleton<FTiti>::Destroy();
}

static void TestLexerParser_() {
    using namespace Core;

    FLexer::FLexerStartup lexerStartup;
    Parser::FParserStartup parserStartup;

    Parser::TProduction< Parser::TEnumerable<
        TTuple<  const Lexer::FMatch *,
        const Lexer::FMatch *,
        const Lexer::FMatch *>
    > > p =
    Parser::Expect(Lexer::FSymbol::Int)
    .And(Parser::Expect(Lexer::FSymbol::Add)
    .Or(Parser::Expect(Lexer::FSymbol::Sub))
    .Or(Parser::Expect(Lexer::FSymbol::Mul))
    .Or(Parser::Expect(Lexer::FSymbol::Div)))
    .And(Parser::Expect(Lexer::FSymbol::Int))
    .Many();

    VirtualFileSystemRoot& vfs = FVirtualFileSystem::Instance();

    FVirtualFileSystemComponentStartup processStartup(
        new FVirtualFileSystemNativeComponent(L"Process:/", FCurrentProcess::Instance().Directory())
        );

    const FFilename inputFilename = L"Process:/parser.txt";
    const FString nativeFilename = StringFormat("{0}", inputFilename);

    {
        auto oss = vfs.OpenWritable(inputFilename, AccessPolicy::Truncate);
        oss->WriteArray("(2 + 3 * 2) + 3 + 0xFF 3 * 8 055 / -7");
    }

    {
        RAWSTORAGE(Serializer, char) storage;
        vfs.ReadAll(inputFilename, storage);

        FLexer::FLexer lexer(storage.MakeConstView(), nativeFilename.c_str());
        Parser::FParseList input(&lexer);

        Parser::FParseContext globalContext(nullptr);

        try {
            Parser::PCParseStatement statement = Serialize::Grammar_Parse(input);

            Parser::FParseContext localContext(globalContext.Transaction(), &globalContext);
            statement->Invoke(&localContext);
        }
        catch (const Parser::FParserException& e) {
            if (e.Item())
                Format(std::cerr, "parser error : <{0}> {1}, {2}.\n", e.Item()->ToString(), e.what(), e.Site());
            else
                Format(std::cerr, "parser error : {0}, {1}.\n", e.what(), e.Site());
        }
        catch (const FLexer::FLexerException& e) {
            Format(std::cerr, "lexer error : <{0}>: {1}, {2}.\n", e.Match().Symbol()->CStr(), e.what(), e.Match().Site());
        }
    }

    RTTI::TMetaClassSingleton<FToto>::Create();
    {
        const RTTI::FMetaClass& metaClass = RTTI::TMetaClassSingleton<FToto>::Instance();
        Format(std::cout, "TMetaClass<{0}> : {1}\n", metaClass.Name(), metaClass.Attributes());
        for (const auto& it : metaClass.Properties())
            Format(std::cout, "  - {0} : {1}, {2} -> {4} [{3}]\n",
            it.first,
            it.second->Attributes(),
            (uint64_t)it.second->TypeInfo().Flags,
            (uint64_t)it.second->TypeInfo().Id,
            it.second->TypeInfo());
    }

    Parser::FParseContext globalContext(new RTTI::FMetaTransaction());

    do
    {
        std::cout << "$ ";

        char line[1024];
        std::cin.getline(line, lengthof(line));

        if (0 == CompareNI("exit", line, 5))
            break;

        try {
            FLexer::FLexer lexer(FStringView(&line[0], Length(line)), "@in_memory");
            Parser::FParseList input(&lexer);

            Parser::PCParseItem item = Serialize::Grammar_Parse(input);
            AssertRelease(item);

            item->Invoke(&globalContext);
        }
        catch (const Parser::FParserException& e) {
            if (e.Item())
                Format(std::cerr, "parser error : <{0}> {1}, {2}.\n", e.Item()->ToString(), e.what(), e.Site());
            else
                Format(std::cerr, "parser error : {0}, {1}.\n", e.what(), e.Site());
        }
        catch (const FLexer::FLexerException& e) {
            Format(std::cerr, "lexer error : <{0}>: {1}, {2}.\n", e.Match().Symbol()->CStr(), e.what(), e.Match().Site());
        }
    } while (true);

    RTTI::TMetaClassSingleton<FToto>::Destroy();
}

static void TestTuple_() {
    using namespace Core;

    int i = 42;
    float f = 4.0f;

    TTuple<int, float> t = MergeTuple(i, f);

    auto k1 = MakeTuple(i);
    auto k2 = MakeTuple(f);
    auto k3 = MakeTuple(t);

    TTuple<int, float, float> t2 = MergeTuple(t, f);

    //TTuple<int, float, float> t2 = std::tuple_cat(MakeTuple(t), MakeTuple(f));
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

    std::cout << w.OneExtend().Dehomogenize().yxyz() << eol;

    w = 1.0f + w;

    float3 x = TLerp(w, 2 * w, 0.5f);

    std::cout << (w == x) << eol;

    x = Saturate(x) + 1.0f;
    x = TRcp(x);

    FRandomGenerator rand;
    x = TLerp(float2(-13), float2(42), rand.NextFloat01()).ZeroExtend();

    u32 i = rand.NextU32(10);
    u32 j = rand.NextU32(10, 42);

    float t = rand.NextFloat01();
    float s = rand.NextFloatM11();

    float4 z = TLerp(float4(-42), float4(69), NextRandFloat01<4>(rand));

    FRandomGenerator rand2(FRandomGenerator::RandomSeedTag{});
    t = rand2.NextFloat01();
    s = rand2.NextFloatM11();

    const AABB3f aabb = AABB3f::MinusOneOne();

    for (size_t c = 0; c < 10; ++c) {
        float3 n = NextRandFloatM11<3>(rand2);

        TScalarVector<u16, 3> q0 = QuantizeCeil<u16>(aabb, n);
        TScalarVector<u16, 3> q1 = QuantizeFloor<u16>(aabb, n);
        TScalarVector<u16, 3> q2 = QuantizeRound<u16>(aabb, n);

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

    FHalfFloat h = 1.234f;
    float hf = h;
    FHalfFloat hc = hf;
    AssertRelease(hc == h);

    const float half_epsilon = FHalfFloat::Epsilon.Unpack();

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

    FTimeline time = FTimeline::StartNow();

    const size_t seconds = 1;
    const size_t wantedTickCount = seconds*15;
    size_t actualTickCount = 0;

    do
    {
        Timespan elapased;
        if (time.Tick_Target15FPS(elapased)) {
            std::cout  << "Tick " << time.Total() << " !" << eol;
            ++actualTickCount;
        }
    }
    while (time.Total() < Units::Time::Seconds(double(seconds)) );

    std::cout << L"Total = " << Units::Time::Seconds(time.Total()) << " seconds -> "
        << actualTickCount << "/"
        << wantedTickCount << " ticks"
        << eol;
}

namespace Core {
    struct FVertexPositionTexCoordNormalTangent {
        float3  Position;
        ushort2 TexCoord;
        ubyte4  Normal;
        ubyte4  Tangent;

        static void CreateVertexDeclaration(Graphics::FVertexDeclaration *vertexDecl) {
            vertexDecl->AddTypedSubPart<Graphics::VertexSubPartSemantic::Position>(&FVertexPositionTexCoordNormalTangent::Position, 0);
            vertexDecl->AddTypedSubPart<Graphics::VertexSubPartSemantic::TexCoord>(&FVertexPositionTexCoordNormalTangent::TexCoord, 0);
            vertexDecl->AddTypedSubPart<Graphics::VertexSubPartSemantic::Normal>(&FVertexPositionTexCoordNormalTangent::Normal, 0);
            vertexDecl->AddTypedSubPart<Graphics::VertexSubPartSemantic::Tangent>(&FVertexPositionTexCoordNormalTangent::Tangent, 0);
            vertexDecl->Freeze();
        }
    };
}

void TestGraphics_() {
    using namespace Core;

    Graphics::FVertexDeclaration vertexDecl;
    vertexDecl.AddSubPart<Graphics::VertexSubPartFormat::Float3, Graphics::VertexSubPartSemantic::Position>(0);
    vertexDecl.AddSubPart<Graphics::VertexSubPartFormat::UShort2, Graphics::VertexSubPartSemantic::TexCoord>(0);
    vertexDecl.AddSubPart<Graphics::VertexSubPartFormat::UByte4, Graphics::VertexSubPartSemantic::Normal>(0);
    vertexDecl.AddSubPart<Graphics::VertexSubPartFormat::UByte4, Graphics::VertexSubPartSemantic::Tangent>(0);
    vertexDecl.Freeze();

    FString str = vertexDecl.ToString();
    size_t size = vertexDecl.SizeInBytes();

    TPair<const Graphics::FVertexSubPartKey *, const Graphics::FAbstractVertexSubPart *> it =
        vertexDecl.SubPartBySemantic(Graphics::VertexSubPartSemantic::Position, 0);
    AssertRelease(it.second);
    AssertRelease(it.first->Format() == Graphics::VertexSubPartFormat::Float3);

    it = vertexDecl.SubPartBySemantic(Graphics::VertexSubPartSemantic::Tangent, 0);
    AssertRelease(it.second);
    AssertRelease(it.first->Format() == Graphics::VertexSubPartFormat::UByte4);

    const auto *subpart = vertexDecl.TypedSubPart<Graphics::VertexSubPartFormat::UByte4>(Graphics::VertexSubPartSemantic::Normal, 0);

    Graphics::FVertexDeclaration vertexDecl2;
    FVertexPositionTexCoordNormalTangent::CreateVertexDeclaration(&vertexDecl2);

    FString str2 = vertexDecl2.ToString();
    size_t size2 = vertexDecl2.SizeInBytes();

    FVertexPositionTexCoordNormalTangent v;

    vertexDecl2.TypedSubPart<Graphics::VertexSubPartFormat::Float3>(Graphics::VertexSubPartSemantic::Position, 0)->TypedSet(&v, float3(1,2,3));
    vertexDecl2.TypedSubPart<Graphics::VertexSubPartFormat::UShort2>(Graphics::VertexSubPartSemantic::TexCoord, 0)->TypedSet(&v, ushort2(16,1024));
    vertexDecl2.TypedSubPart<Graphics::VertexSubPartFormat::UByte4>(Graphics::VertexSubPartSemantic::Normal, 0)->TypedSet(&v, ubyte4(0,0,0xFF,0));

    FVertexPositionTexCoordNormalTangent v2;
    vertexDecl2.CopyVertex(&v2, &v, sizeof(FVertexPositionTexCoordNormalTangent));
    AssertRelease(v.Position == v2.Position);
    AssertRelease(v.TexCoord == v2.TexCoord);
    AssertRelease(v.Normal == v2.Normal);

    const Graphics::FVertexDeclaration *vdecl2 = Graphics::Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration;
    const auto autoName = vdecl2->ToString();

    Graphics::Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N vvv;
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

    const Graphics::FSurfaceFormat *fmt0 = Graphics::FSurfaceFormat::D24S8;
    AssertRelease(fmt0->IsDepth());
    AssertRelease(fmt0->IsStencil());
    AssertRelease(fmt0->BitsPerPixels() == 32);
    AssertRelease(fmt0->Pitch() == 4);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    fmt0 = Graphics::FSurfaceFormat::R8G8B8A8_SRGB;
    AssertRelease(fmt0->IsRGB());
    AssertRelease(fmt0->IsAlpha());
    AssertRelease(fmt0->IsGammaSpace());
    AssertRelease(fmt0->BitsPerPixels() == 32);
    AssertRelease(fmt0->Pitch() == 4);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    /* TODO
    fmt0 = Graphics::FSurfaceFormat::DXT5_SRGB;
    AssertRelease(fmt0->RGB());
    AssertRelease(fmt0->Alpha());
    AssertRelease(fmt0->GammaSpace());
    AssertRelease(fmt0->DXTC());
    AssertRelease(fmt0->BitsPerPixels() == 8);
    AssertRelease(fmt0->Pitch() == 4);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 4);
    */

    fmt0 = Graphics::FSurfaceFormat::R32G32_F;
    AssertRelease(fmt0->IsRA());
    AssertRelease(fmt0->IsFloatingPoint());
    AssertRelease(fmt0->BitsPerPixels() == 64);
    AssertRelease(fmt0->Pitch() == 8);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    fmt0 = Graphics::FSurfaceFormat::R16G16B16A16_F;
    AssertRelease(fmt0->IsRGB());
    AssertRelease(fmt0->IsAlpha());
    AssertRelease(fmt0->IsFloatingPoint());
    AssertRelease(fmt0->BitsPerPixels() == 64);
     AssertRelease(fmt0->Pitch() == 8);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    fmt0 = Graphics::FSurfaceFormat::R32G32B32A32;
    AssertRelease(fmt0->IsRGB());
    AssertRelease(fmt0->IsAlpha());
    AssertRelease(!fmt0->IsFloatingPoint());
    AssertRelease(fmt0->BitsPerPixels() == 128);
    AssertRelease(fmt0->Pitch() == 16);
    AssertRelease(fmt0->MacroBlockSizeInPixels() == 1);

    const Graphics::FBlendState *blend = Graphics::FBlendState::AlphaBlend;
    AssertRelease(blend->ColorSourceBlend() == Graphics::EBlend::One);
    AssertRelease(blend->AlphaDestinationBlend() == Graphics::EBlend::InverseSourceAlpha);
    AssertRelease(blend->Frozen());

    const Graphics::FDepthStencilState *depth = Graphics::FDepthStencilState::Default;
    AssertRelease(depth->DepthBufferEnabled());
    AssertRelease(depth->DepthBufferFunction() == Graphics::ECompareFunction::TLessEqual);

    const Graphics::FRasterizerState *rasterizer = Graphics::FRasterizerState::CullCounterClockwise;
    AssertRelease(rasterizer->CullMode() == Graphics::ECullMode::CullCounterClockwiseFace);
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

    std::cout << "static const u8 gVirtualKey_to_KeyboardKey[0xFF] = {" << eol;

    u8 virtualKeyToKeyboardKey[0xFF];
    memset(virtualKeyToKeyboardKey, 0xFF, sizeof(virtualKeyToKeyboardKey));

    virtualKeyToKeyboardKey[VK_NUMPAD0] = u8(EKeyboardKey::Numpad0);
    virtualKeyToKeyboardKey[VK_NUMPAD1] = u8(EKeyboardKey::Numpad1);
    virtualKeyToKeyboardKey[VK_NUMPAD2] = u8(EKeyboardKey::Numpad2);
    virtualKeyToKeyboardKey[VK_NUMPAD3] = u8(EKeyboardKey::Numpad3);
    virtualKeyToKeyboardKey[VK_NUMPAD4] = u8(EKeyboardKey::Numpad4);
    virtualKeyToKeyboardKey[VK_NUMPAD5] = u8(EKeyboardKey::Numpad5);
    virtualKeyToKeyboardKey[VK_NUMPAD6] = u8(EKeyboardKey::Numpad6);
    virtualKeyToKeyboardKey[VK_NUMPAD7] = u8(EKeyboardKey::Numpad7);
    virtualKeyToKeyboardKey[VK_NUMPAD8] = u8(EKeyboardKey::Numpad8);
    virtualKeyToKeyboardKey[VK_NUMPAD9] = u8(EKeyboardKey::Numpad9);

    virtualKeyToKeyboardKey[VK_ADD] = u8(EKeyboardKey::Add);
    virtualKeyToKeyboardKey[VK_SUBTRACT] = u8(EKeyboardKey::Subtract);
    virtualKeyToKeyboardKey[VK_MULTIPLY] = u8(EKeyboardKey::Multiply);
    virtualKeyToKeyboardKey[VK_DIVIDE] = u8(EKeyboardKey::Divide);

    virtualKeyToKeyboardKey[VK_F1] = u8(EKeyboardKey::F1);
    virtualKeyToKeyboardKey[VK_F2] = u8(EKeyboardKey::F2);
    virtualKeyToKeyboardKey[VK_F3] = u8(EKeyboardKey::F3);
    virtualKeyToKeyboardKey[VK_F4] = u8(EKeyboardKey::F4);
    virtualKeyToKeyboardKey[VK_F5] = u8(EKeyboardKey::F5);
    virtualKeyToKeyboardKey[VK_F6] = u8(EKeyboardKey::F6);
    virtualKeyToKeyboardKey[VK_F7] = u8(EKeyboardKey::F7);
    virtualKeyToKeyboardKey[VK_F8] = u8(EKeyboardKey::F8);
    virtualKeyToKeyboardKey[VK_F9] = u8(EKeyboardKey::F9);
    virtualKeyToKeyboardKey[VK_F10] = u8(EKeyboardKey::F10);
    virtualKeyToKeyboardKey[VK_F11] = u8(EKeyboardKey::F11);
    virtualKeyToKeyboardKey[VK_F12] = u8(EKeyboardKey::F12);

    virtualKeyToKeyboardKey[VK_UP] = u8(EKeyboardKey::Up);
    virtualKeyToKeyboardKey[VK_DOWN] = u8(EKeyboardKey::Down);
    virtualKeyToKeyboardKey[VK_LEFT] = u8(EKeyboardKey::Left);
    virtualKeyToKeyboardKey[VK_RIGHT] = u8(EKeyboardKey::Right);

    virtualKeyToKeyboardKey[VK_ESCAPE] = u8(EKeyboardKey::Escape);
    virtualKeyToKeyboardKey[VK_SPACE] = u8(EKeyboardKey::Space);

    virtualKeyToKeyboardKey[VK_PAUSE] = u8(EKeyboardKey::Pause);
    virtualKeyToKeyboardKey[VK_PRINT] = u8(EKeyboardKey::PrintScreen);
    virtualKeyToKeyboardKey[VK_SCROLL] = u8(EKeyboardKey::ScrollLock);

    virtualKeyToKeyboardKey[VK_BACK] = u8(EKeyboardKey::Backspace);
    virtualKeyToKeyboardKey[VK_RETURN] = u8(EKeyboardKey::Enter);
    virtualKeyToKeyboardKey[VK_TAB] = u8(EKeyboardKey::Tab);

    virtualKeyToKeyboardKey[VK_HOME] = u8(EKeyboardKey::Home);
    virtualKeyToKeyboardKey[VK_END] = u8(EKeyboardKey::End);
    virtualKeyToKeyboardKey[VK_INSERT] = u8(EKeyboardKey::Insert);
    virtualKeyToKeyboardKey[VK_DELETE] = u8(EKeyboardKey::Delete);
    virtualKeyToKeyboardKey[VK_PRIOR] = u8(EKeyboardKey::PageUp);
    virtualKeyToKeyboardKey[VK_NEXT] = u8(EKeyboardKey::PageDown);

    virtualKeyToKeyboardKey[VK_MENU] = u8(EKeyboardKey::Alt);
    virtualKeyToKeyboardKey[VK_APPS] = u8(EKeyboardKey::Menu);
    virtualKeyToKeyboardKey[VK_SHIFT] = u8(EKeyboardKey::Control);
    virtualKeyToKeyboardKey[VK_CONTROL] = u8(EKeyboardKey::Shift);

    for (u8 i = 'A'; i <= 'Z'; ++i) {
        AssertRelease(u8(EKeyboardKey::A) + i - 'A' == i);
        virtualKeyToKeyboardKey[i] = i;
    }
    for (u8 i = '0'; i <= '9'; ++i) {
        AssertRelease(u8(EKeyboardKey::_0) + i - '0' == i);
        virtualKeyToKeyboardKey[i] = i;
    }

    for (size_t i = 0; i < 0xFF; ++i)
        if (virtualKeyToKeyboardKey[i] == 0xFF)
            std::cout << "    0xFF," << eol;
        else
            std::cout << "    u8(EKeyboardKey::" << KeyboardKeyToCStr(EKeyboardKey(virtualKeyToKeyboardKey[i])) << ")," << eol;

    std::cout << "};" << eol;
}

void Tests() {
    using namespace Core;

    FMemoryTrackingData scopeTrackingData;
    std::vector<int, TTrackingAllocator< TMallocator<int>> > u{ TTrackingAllocator<TMallocator<int>>(scopeTrackingData) };
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
        std::cout << MakeView(z) << eol;
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
        std::vector<int, TAllocator<int> > v{ 4, 5, 6 };
        v.push_back(2);
        v.insert(v.end(), u.begin(), u.end());
        v.reserve(9);

        //Print(FMemoryTrackingData::Global());

        TVector<int> w(v.begin(), v.end());

        //Print(FMemoryTrackingData::Global());

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

        //Print(FMemoryTrackingData::Global());
    }

    {
        FHeap& heap = Heaps::Process::Instance();
        void* p = heap.malloc<16>(100);

        FCrtMemoryStats memStats;
        CrtDumpMemoryStats(&memStats, heap.Handle());
        Print(memStats);

        typedef DECORATE_ALLOCATOR(Internal, THeapAllocator<int>) allocator_type;
        std::vector<int, allocator_type> x(u.begin(), u.end());
        x.reserve(1024);

        std::cout << "FHeap size = " << heap.Size() << eol;

        CrtDumpMemoryStats(&memStats, heap.Handle());
        Print(memStats);

        heap.free<16>(p);
    }

    {
        FHeap heap{ "stack", false };
        void* p = heap.malloc<16>(100);

        std::cout << "FHeap size = " << heap.Size() << eol;

        FCrtMemoryStats memStats;
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
FApplicationTest::FApplicationTest()
:   FApplicationConsole(L"FApplicationTest") {}
//----------------------------------------------------------------------------
FApplicationTest::~FApplicationTest() {}
//----------------------------------------------------------------------------
void FApplicationTest::Start() {
    FApplicationConsole::Start();
    Tests();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
