#include "stdafx.h"

#include "TransactionSerializer.h"

#include "Serialize.h"
#include "ISerializer.h"
#include "TransactionLinker.h"
#include "TransactionSaver.h"

#include "MetaTransaction.h"
#include "RTTI/Macros-impl.h"
#include "RTTI/Typedefs.h"

#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "Memory/Compression.h"
#include "Memory/MemoryStream.h"
#include "Thread/DeferredStream.h"
#include "Thread/Task/TaskHelpers.h"

namespace PPE {
namespace Serialize {
EXTERN_LOG_CATEGORY(PPE_SERIALIZE_API, Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_ENUM_FLAGS_BEGIN(Serialize, ETransactionFlags)
RTTI_ENUM_VALUE(None)
RTTI_ENUM_VALUE(AutoBuild)
RTTI_ENUM_VALUE(AutoMount)
RTTI_ENUM_VALUE(AutoImport)
RTTI_ENUM_VALUE(Compressed)
RTTI_ENUM_VALUE(Merged) // #TODO
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_ENUM_BEGIN(Serialize, ESerializeFormat)
RTTI_ENUM_VALUE(Binary)
RTTI_ENUM_VALUE(Json)
RTTI_ENUM_VALUE(Markup)
RTTI_ENUM_VALUE(Script)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Serialize, FTransactionSerializer, Public)
RTTI_PROPERTY_PRIVATE_FIELD(_id)
RTTI_PROPERTY_PRIVATE_FIELD(_namespace)
RTTI_PROPERTY_PRIVATE_FIELD(_flags)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Serialize, FDirectoryTransaction, Public)
RTTI_PROPERTY_PRIVATE_FIELD(_inputPattern)
RTTI_PROPERTY_PRIVATE_FIELD(_inputPaths)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTransactionSerializer::FTransactionSerializer(RTTI::FConstructorTag)
:   _flags(ETransactionFlags::Default)
{}
//----------------------------------------------------------------------------
FTransactionSerializer::FTransactionSerializer(
    const RTTI::FName& id,
    const RTTI::FName& namespace_,
    ETransactionFlags flags /* = ETransactionFlags::Default */ )
:   _id(id)
,   _namespace(namespace_)
,   _flags(flags) {
    Assert_NoAssume(not _id.empty());
    Assert_NoAssume(not _namespace.empty());
}
//----------------------------------------------------------------------------
FTransactionSerializer::~FTransactionSerializer() {
    Assert(not _transaction);
}
//----------------------------------------------------------------------------
void FTransactionSerializer::BuildTransaction(FSources& sources) {
    if (_transaction)
        UnloadTransaction();

    FetchSources(sources);
    if (sources.empty())
        return;

    LOG(Serialize, Emphasis, L"building transaction '{0}' with namespace <{1}> from {2} sources ...",
        _id, _namespace, sources.size() );

    _transaction = NEW_REF(MetaSerialize, RTTI::FMetaTransaction, _namespace);

    VECTOR(Transient, FTransactionLinker) linkers;
    linkers.resize(sources.size());

    ParallelFor(0, sources.size(), [&linkers, &sources](size_t i) {
        FTransactionLinker linker(sources[i]);
        const PSerializer serializer = ISerializer::FromExtname(linker.Filename().Extname());

        for (;;) {
            // using deferred IO to avoid blocking workers on IO
            const UStreamReader reader{ VFS_OpenBinaryReadable(linker.Filename()) };
            const bool succeed = UsingDeferredStream(reader.get(), [&](IBufferedStreamReader* async) {
                return ISerializer::InteractiveDeserialize(*serializer, *async, &linker);
            });

            // interactive de-serialization allows for retrying when serializer failed
            // that way the user can correct the source file and iterate until it's valid
            if (succeed)
                break;
        }

        linker.MoveTo(linkers[i]);
    });

    for (FTransactionLinker& linker : linkers)
        linker.Resolve(*_transaction);

    _transaction->Load();
}
//----------------------------------------------------------------------------
void FTransactionSerializer::SaveTransaction() {
    Assert(_transaction);
    Assert_NoAssume(_transaction->IsLoaded() || _transaction->IsMounted());

    FTransactionSaver saver(*_transaction, IdToTransaction(_id));
    const PSerializer serializer = ISerializer::FromExtname(saver.Filename().Extname());

    if (_flags & ETransactionFlags::Compressed) {
        FFilename fnameZ{ saver.Filename() };
        fnameZ.ReplaceExtension(FFSConstNames::Z());

        LOG(Serialize, Emphasis, L"saving compressed transaction '{0}' with namespace <{1}> in '{2}' ...",
            _id, _namespace, fnameZ);

        MEMORYSTREAM(Transient) raw;
        serializer->Serialize(saver, &raw);

        RAWSTORAGE(Transient, u8) compressed;
        const size_t compressedSize = Compression::CompressMemory(compressed, raw.MakeView(), Compression::HighCompression);
        raw.clear_ReleaseMemory();

        const UStreamWriter writer{ VFS_OpenBinaryWritable(fnameZ, EAccessPolicy::Truncate) };
        UsingDeferredStream(writer.get(), [&](IStreamWriter* async) {
            async->WriteView(compressed.MakeView().FirstNElements(compressedSize));
        });
    }
    else {
        LOG(Serialize, Emphasis, L"saving transaction '{0}' with namespace <{1}> in '{2}' ...",
            _id, _namespace, saver.Filename());

        const UStreamWriter writer{ VFS_OpenBinaryWritable(saver.Filename(), EAccessPolicy::Truncate) };
        UsingDeferredStream(writer.get(), [&](IBufferedStreamWriter* async) {
            serializer->Serialize(saver, async);
        });
    }
}
//----------------------------------------------------------------------------
void FTransactionSerializer::LoadTransaction() {
    Assert(not _transaction);

    FTransactionLinker linker(IdToTransaction(_id));
    const PSerializer serializer = ISerializer::FromExtname(linker.Filename().Extname());

    if (_flags & ETransactionFlags::Compressed) {
        FFilename fnameZ{ linker.Filename() };
        fnameZ.ReplaceExtension(FFSConstNames::Z());

        LOG(Serialize, Emphasis, L"loading compressed transaction '{0}' with namespace <{1}> from '{2}' ...",
            _id, _namespace, fnameZ);

        RAWSTORAGE(Transient, u8) compressed;
        {
            const UStreamReader reader{ VFS_OpenBinaryReadable(fnameZ) };
            UsingDeferredStream(reader.get(), [&](IBufferedStreamReader* async) {
                async->ReadAll(compressed);
            });
        }

        MEMORYSTREAM(Transient) raw;
        raw.resize(Compression::DecompressedSize(compressed.MakeView()));
        VerifyRelease(Compression::DecompressMemory(raw.MakeView(), compressed.MakeView()));
        compressed.clear_ReleaseMemory();

        serializer->Deserialize(raw, &linker);
    }
    else {
        LOG(Serialize, Emphasis, L"loading transaction '{0}' with namespace <{1}> from '{2}' ...",
            _id, _namespace, linker.Filename());

        const UStreamReader reader{ VFS_OpenBinaryReadable(linker.Filename()) };
        UsingDeferredStream(reader.get(), [&](IBufferedStreamReader* async) {
            serializer->Deserialize(*async, &linker);
        });
    }

    _transaction = NEW_REF(MetaSerialize, RTTI::FMetaTransaction, _namespace);

    linker.Resolve(*_transaction);

    _transaction->Load();
}
//----------------------------------------------------------------------------
void FTransactionSerializer::UnloadTransaction() {
    Assert(_transaction);
    AssertRelease_NoAssume(_transaction->IsLoaded());

    LOG(Serialize, Emphasis, L"unloading transaction '{0}' with namespace <{1}> ...",
        _id, _namespace);

    _transaction->Unload();

    RemoveRef_AssertReachZero(_transaction);
}
//----------------------------------------------------------------------------
void FTransactionSerializer::MountToDB() {
    Assert(_transaction);
    AssertRelease_NoAssume(_transaction->IsLoaded());

    LOG(Serialize, Emphasis, L"mounting transaction '{0}' with namespace <{1}> ...",
        _id, _namespace);

    _transaction->Mount();
}
//----------------------------------------------------------------------------
void FTransactionSerializer::UnmountFromDB() {
    Assert(_transaction);
    AssertRelease_NoAssume(_transaction->IsMounted());

    LOG(Serialize, Emphasis, L"unmounting transaction '{0}' with namespace <{1}> ...",
        _id, _namespace);

    _transaction->Unmount();
}
//----------------------------------------------------------------------------
void FTransactionSerializer::RTTI_Load(RTTI::ILoadContext& context) {
    RTTI_parent_type::RTTI_Load(context);

    if (_flags ^ ETransactionFlags::AutoBuild &&
        not VFS_FileExists(IdToTransaction(_id))) {
        FSources sources;
        BuildTransaction(sources);
        SaveTransaction();
        UnloadTransaction();
    }

    if (_flags ^ ETransactionFlags::AutoImport |
        _flags ^ ETransactionFlags::AutoMount )
        LoadTransaction();

    if (_flags ^ ETransactionFlags::AutoMount)
        MountToDB();
}
//----------------------------------------------------------------------------
void FTransactionSerializer::RTTI_Unload(RTTI::IUnloadContext& context) {
    if (_flags ^ ETransactionFlags::AutoMount)
        UnmountFromDB();

    if (_flags ^ ETransactionFlags::AutoImport |
        _flags ^ ETransactionFlags::AutoMount)
        UnloadTransaction();

    RTTI_parent_type::RTTI_Unload(context);
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FTransactionSerializer::RTTI_VerifyPredicates() const PPE_THROW() {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _id.empty());
    RTTI_VerifyPredicate(not _namespace.empty());
}
#endif //!WITH_RTTI_VERIFY_PREDICATES
//----------------------------------------------------------------------------
const FDirpath& FTransactionSerializer::TransactionPath() {
    ONE_TIME_INITIALIZE(FDirpath, GOutputPath, MakeStringView(L"Saved:/RTTI"));
    return GOutputPath;
}
//----------------------------------------------------------------------------
FFilename FTransactionSerializer::IdToTransaction(const RTTI::FName& name) {
    Assert(not name.empty());

    FDirpath absolute(TransactionPath());
    FStringView p(name.MakeView()), s;

    FFilename result;
    while (Split(p, '/', s)) {
        if (p.empty()) // basename
            result = FFilename(absolute, ToWString(s), FFSConstNames::Bnx());
        else // dirname
            absolute.Concat(ToWString(s).MakeView());
    }
    return result;
}
//----------------------------------------------------------------------------
RTTI::FName FTransactionSerializer::TransactionToId(const FFilename& transaction) {
    Assert(transaction.IsRelativeTo(TransactionPath()));

    FFilename rel = transaction.Relative(TransactionPath());
    AssertRelease(rel.Extname() == FFSConstNames::Bnx());

    // remove ext and convert to ansi string
    rel.ReplaceExtension(FExtname{});

    STACKLOCAL_POD_ARRAY(char, tmp, FileSystem::MaxPathLength);
    return RTTI::FName{ rel.ToCStr(tmp) };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDirectoryTransaction::FDirectoryTransaction(RTTI::FConstructorTag rtti)
:   FTransactionSerializer(rtti)
{}
//----------------------------------------------------------------------------
FDirectoryTransaction::FDirectoryTransaction(
    const RTTI::FName& id,
    const RTTI::FName& namespace_,
    FWString&& inputPattern,
    FInputPaths&& inputPaths,
    ETransactionFlags flags /* = ETransactionFlags::Default */)
:   FTransactionSerializer(id, namespace_, flags)
,   _inputPattern(std::move(inputPattern))
,   _inputPaths(std::move(inputPaths))
{}
//----------------------------------------------------------------------------
FDirectoryTransaction::~FDirectoryTransaction() = default;
//----------------------------------------------------------------------------
void FDirectoryTransaction::FetchSources(FSources& sources) {
    FWRegexp re;
    if (not _inputPattern.empty())
        re.Compile(_inputPattern, FileSystem::CaseSensitive);

    const TFunction<void(const FFilename&)> each_source([&sources](const FFilename& fname) {
        Add_Unique(sources, FFilename(fname));
    });

    for (const FDirpath& path : _inputPaths) {
        const size_t n = sources.size();
        UNUSED(n);

        if (_inputPattern.empty())
            VFS_EnumerateFiles(path, true, each_source);
        else
            VFS_MatchFiles(path, re, true, each_source);

        LOG(Serialize, Debug, L"found {0} source files for transaction '{1}' in '{2}{3}{4}'...",
            sources.size() - n, Id(), path, FileSystem::NormalizedSeparator, _inputPattern );
    }
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FDirectoryTransaction::RTTI_VerifyPredicates() const PPE_THROW() {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _inputPaths.empty());
}
#endif //!WITH_RTTI_VERIFY_PREDICATES
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
