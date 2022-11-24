// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TextureCache.h"

#include "TextureLoader.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"
#include "Core.Graphics/Device/Texture/Texture.h"
#include "Core.Graphics/Device/Texture/Texture2D.h"
#include "Core.Graphics/Device/Texture/TextureCube.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Maths/Units.h"
#include "Core/Thread/ThreadPool.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void TextureEntry_InsertMRU_(
    FTextureCache::FTextureEntry **lru,
    FTextureCache::FTextureEntry **mru,
    FTextureCache::FTextureEntry *node ) {
    Assert(node);
    Assert(nullptr == node->Prev());
    Assert(nullptr == node->Next());

    if (*mru) {
        Assert(*lru);
        Assert(!(*mru)->Next());

        (*mru)->SetNext(node);
    }
    else {
        Assert(!*lru);

        *lru = node;
    }

    node->SetNext(nullptr);
    node->SetPrev(*mru);

    *mru = node;
}
//----------------------------------------------------------------------------
static void TextureEntry_Erase_(
    FTextureCache::FTextureEntry **lru,
    FTextureCache::FTextureEntry **mru,
    FTextureCache::FTextureEntry *node ) {
    Assert(node);
    Assert(!*lru || (*lru)->Prev() == nullptr);
    Assert(!*mru || (*mru)->Next() == nullptr);

    if (node->Prev())
        node->Prev()->SetNext(node->Next());
    if (node->Next())
        node->Next()->SetPrev(node->Prev());

    if (*lru == node)
        *lru = (*lru)->Next();
    if (*mru == node)
        *mru = (*mru)->Prev();

    node->SetPrev(nullptr);
    node->SetNext(nullptr);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextureCache::FTextureData::FTextureData(FTextureCache *cache)
:   _pixels(TexturePixelsAllocator(&cache->_heap)) {}
//----------------------------------------------------------------------------
FTextureCache::FTextureData::~FTextureData() {}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FTextureCache::FTextureData, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextureCache::FTextureEntry::FTextureEntry(const PPE::FFilename& filename, bool useSRGB, bool keepData)
:   _filename(filename)
,   _prev(nullptr) {
    _nextWFlags.Reset(nullptr, useSRGB, keepData);

    LOG(Info, L"[FTextureCache] Loading texture '{0}', SRGB = {1:A}, Keep data = {2:A} ...",
        _filename, UseSRGB(), KeepData() );
}
//----------------------------------------------------------------------------
FTextureCache::FTextureEntry::~FTextureEntry() {
    if (_texture)
        RemoveRef_AssertReachZero(_texture);

    LOG(Info, L"[FTextureCache] Unloading texture '{0}' ...",
        _filename, UseSRGB(), KeepData() );
}
//----------------------------------------------------------------------------
const Graphics::FTexture2D *FTextureCache::FTextureEntry::AsTexture2D() const {
    Assert(_texture);
    return checked_cast<const Graphics::FTexture2D *>(_texture.get());
}
//----------------------------------------------------------------------------
const Graphics::FTextureCube *FTextureCache::FTextureEntry::AsTextureCube() const {
    Assert(_texture);
    return checked_cast<const Graphics::FTextureCube *>(_texture.get());
}
//----------------------------------------------------------------------------
void FTextureCache::FTextureEntry::SetData_(const FTextureData *data) {
    _data.reset(data);
}
//----------------------------------------------------------------------------
void FTextureCache::FTextureEntry::SetTexture_(Graphics::FTexture *texture) {
    Assert(texture);
    Assert(!_texture);

    _texture.reset(texture);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FTextureCache::FTextureEntry, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskResult FTextureCache::FTextureEntryAsyncJob::Invoke(const TaskContext& ) {
    Assert(_pentry);

    FTexture2DLoader loader;
    FTextureCache::FTextureData *pdata = new FTextureCache::FTextureData(_cache);
    if (!loader.Read(pdata->Pixels(), _pentry->Filename()))
        AssertNotImplemented();

    pdata->Header() = loader.Header();
    _pentry->SetData_(pdata);

    std::unique_lock<std::mutex> scopeLock(_cache->_barrier);
    _cache->_pjobs.push_back(this);

    return TaskResult::Succeed;
}
//----------------------------------------------------------------------------
void FTextureCache::FTextureEntryAsyncJob::Finalize_MainThread(Graphics::IDeviceAPIEncapsulator *device) const {
    Assert(_pentry->Data());

    FTexture2DLoader loader(_pentry->Data()->Header());

    const TMemoryView<const u8> pixels = _pentry->Data()->Pixels().MakeConstView();
    Graphics::FTexture *const texture = loader.CreateTexture(
        device, pixels, _pentry->Filename().ToString(), _pentry->UseSRGB() );
    Assert(texture);

    _pentry->SetTexture_(texture);

    if (!_pentry->KeepData())
        _pentry->SetData_(nullptr);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FTextureCache::FTextureEntryAsyncJob, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextureCache::FTextureCache(size_t capacityInBytes)
:   _device(nullptr)
,   _heap(capacityInBytes, true)
,   _completionPort("FTextureCache", IOThreadPool::Instance().Evaluator())
,   _lru(nullptr)
,   _mru(nullptr) {
    Assert(capacityInBytes >= Units::Storage::Megabytes(128).Value());

    _pjobs.reserve(128);
}
//----------------------------------------------------------------------------
FTextureCache::~FTextureCache() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void FTextureCache::SetFallbackTexture2D(const FFilename& path) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!path.empty());
    Assert(_device);

    const bool useSRGB = true;
    Graphics::FTexture *texture = FTexture2DLoader::Load(_device, path, useSRGB);
    Assert(texture);

    SetFallbackTexture2D(checked_cast<Graphics::FTexture2D *>(texture));
}
//----------------------------------------------------------------------------
void FTextureCache::SetFallbackTexture2D(Graphics::FTexture2D *texture) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(texture->Available());

    _fallbackTexture2D = texture;
}
//----------------------------------------------------------------------------
void FTextureCache::SetFallbackTextureCube(const FFilename& path) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!path.empty());
    Assert(_device);

    const bool useSRGB = true;
    Graphics::FTexture *texture = FTexture2DLoader::Load(_device, path, useSRGB);
    Assert(texture);

    SetFallbackTextureCube(checked_cast<Graphics::FTextureCube *>(texture));
}
//----------------------------------------------------------------------------
void FTextureCache::SetFallbackTextureCube(Graphics::FTextureCube *texture) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(texture->Frozen());
    Assert(texture->Available());

    _fallbackTextureCube = texture;
}
//----------------------------------------------------------------------------
void FTextureCache::Update() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    if (_heap.ConsumedInBytes() > _heap.ChunkSizeInBytes()) // has grown
        UnloadLRUTextures(); // try to free some unused textures before potentially loading new ones

    AllocaView<PCTextureEntryAsyncJob> pjobs;
    {
        std::unique_lock<std::mutex> scopeLock(_barrier);

        const size_t jobsToFinalize = _pjobs.size();
        if (jobsToFinalize) {
            // move content to release lock immediately
            pjobs = MALLOCA_VIEW(PCTextureEntryAsyncJob, jobsToFinalize);

            for (size_t i = 0; i < jobsToFinalize; ++i)
                new ((void *) &pjobs[i]) PCTextureEntryAsyncJob(std::move(_pjobs[i]));

            _pjobs.clear();
        }
    }

    for (PCTextureEntryAsyncJob& pjob : pjobs) {
        Assert(pjob);

        pjob->Finalize_MainThread(_device); // create gpu resources in main thread for async loaded textures

        RemoveRef_AssertReachZero(pjob);
    }
}
//----------------------------------------------------------------------------
FTextureCache::FTextureEntry *FTextureCache::LoadTexture_Sync_(const FFilename& filename, bool useSRGB, bool keepData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(_textures.end() == _textures.find(filename));

    TUniquePtr<FTextureEntry>& pentry = _textures[filename];
    Assert(!pentry);

    pentry.reset(new FTextureEntry(filename, useSRGB, keepData));

    FTexture2DLoader loader;
    if (keepData) {
        FTextureCache::FTextureData *pdata = new FTextureCache::FTextureData(this);
        if (!loader.Read(pdata->Pixels(), pentry->Filename()))
            AssertNotImplemented();

        pdata->Header() = loader.Header();
        pentry->SetData_(pdata);

        const TMemoryView<const u8> pixels = pdata->Pixels().MakeConstView();
        Graphics::FTexture *const texture = loader.CreateTexture(_device, pixels, filename.ToString(), useSRGB);
        Assert(texture);

        pentry->SetTexture_(texture);
    }
    else {
        FTextureCache::FTextureData data(this);
        if (!loader.Read(data.Pixels(), pentry->Filename()))
            AssertNotReached();

        const TMemoryView<const u8> pixels = data.Pixels().MakeConstView();
        Graphics::FTexture *const texture = loader.CreateTexture(_device, pixels, filename.ToString(), useSRGB);
        Assert(texture);

        pentry->SetTexture_(texture);
    }

    Assert(pentry->Texture());
    return pentry.get();
}
//----------------------------------------------------------------------------
FTextureCache::FTextureEntry *FTextureCache::LoadTexture_ASync_(const FFilename& filename, bool useSRGB, bool keepData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(_textures.end() == _textures.find(filename));

    TUniquePtr<FTextureEntry>& pentry = _textures[filename];
    Assert(!pentry);

    pentry.reset(new FTextureEntry(filename, useSRGB, keepData));

    _completionPort.Produce(new FTextureEntryAsyncJob(this, pentry.get()));

    return pentry.get();
}
//----------------------------------------------------------------------------
void FTextureCache::UnloadTextureEntry_(TUniquePtr<FTextureEntry>& pentry) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(pentry);
    Assert(pentry->Available());
    Assert(pentry->Texture()->RefCount() == 1);

    if (pentry->_texture) {
        Assert(pentry->_texture->Frozen());
        Assert(pentry->_texture->Available());

        pentry->_texture->Destroy(_device);
    }

    TextureEntry_Erase_(&_lru, &_mru, pentry.get());

    pentry.reset(nullptr);
}
//----------------------------------------------------------------------------
void FTextureCache::UnloadTexture(const FFilename& filename) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(_device);

    _completionPort.WaitAll();

    const auto it = _textures.find(filename);
    AssertRelease(_textures.end() != it);

    TUniquePtr<FTextureEntry>& pentry = it->second;
    Assert(pentry);
    Assert(pentry->Filename() == it->first);

    UnloadTextureEntry_(pentry);

    _textures.erase(it);
}
//----------------------------------------------------------------------------
void FTextureCache::UnloadLRUTextures() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    while (_lru && 1 == _lru->_texture->RefCount())
        UnloadTexture(_lru->Filename());
}
//----------------------------------------------------------------------------
void FTextureCache::PrepareTexture(const FFilename& filename, bool useSRGB/* = false */, bool asynchronous/* = true */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(_device);

    FTextureEntry *pentry = nullptr;

    const bool keepData = false;

    const auto it = _textures.find(filename);
    if (_textures.end() == it) {
        if (VFS_FileExists(filename))
            pentry = (asynchronous)
                ? LoadTexture_ASync_(filename, useSRGB, keepData)
                : LoadTexture_Sync_(filename, useSRGB, keepData);
    }
    else {
        pentry = it->second.get();
        Assert(pentry);
        Assert(pentry->Filename() == filename);

        TextureEntry_Erase_(&_lru, &_mru, pentry);
    }

    if (pentry)
        TextureEntry_InsertMRU_(&_lru, &_mru, pentry);
}
//----------------------------------------------------------------------------
bool FTextureCache::FetchTexture2D(const FFilename& filename, const Graphics::FTexture2D **texture) const {
    const Graphics::FTexture *t = nullptr;
    if (!FetchTextureImpl_(filename, &t))
        return false;

    *texture = checked_cast<const Graphics::FTexture2D *>(t);
    return true;
}
//----------------------------------------------------------------------------
const Graphics::FTexture2D *FTextureCache::FetchTexture2D_Fallback(const FFilename& filename) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    const Graphics::FTexture2D *texture = nullptr;
    if (!FetchTexture2D(filename, &texture))
        texture = _fallbackTexture2D.get();

    Assert(texture);
    return texture;
}
//----------------------------------------------------------------------------
bool FTextureCache::FetchTextureCube(const FFilename& filename, const Graphics::FTextureCube **texture) const {
    const Graphics::FTexture *t = nullptr;
    if (!FetchTextureImpl_(filename, &t))
        return false;

    *texture = checked_cast<const Graphics::FTextureCube *>(t);
    return true;
}
//----------------------------------------------------------------------------
const Graphics::FTextureCube *FTextureCache::FetchTextureCube_Fallback(const FFilename& filename) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    const Graphics::FTextureCube *texture = nullptr;
    if (!FetchTextureCube(filename, &texture))
        texture = _fallbackTextureCube.get();

    Assert(texture);
    return texture;
}
//----------------------------------------------------------------------------
bool FTextureCache::FetchTextureImpl_(const FFilename& filename, const Graphics::FTexture **texture) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(texture);
    Assert(_device);

    const auto it = _textures.find(filename);
    if (_textures.end() == it)
        return false;

    const TUniquePtr<FTextureEntry>& pentry = it->second;
    Assert(pentry);
    Assert(pentry->Filename() == filename);

    if (!pentry->Available())
        return false;

    *texture = pentry->Texture();
    return true;
}
//----------------------------------------------------------------------------
void FTextureCache::Clear() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    _completionPort.WaitAll();

    for (TPair<const FFilename, TUniquePtr<FTextureEntry> >& entry : _textures) {
        TUniquePtr<FTextureEntry>& pentry = entry.second;
        Assert(pentry);
        Assert(pentry->Filename() == entry.first);

        UnloadTextureEntry_(pentry);
    }

    _textures.clear();
}
//----------------------------------------------------------------------------
void FTextureCache::ReloadAllTextures() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    _completionPort.WaitAll();

    const size_t textureCount = _textures.size();

    LOG(Info, L"[FTextureCache] Reloading {0} textures ...", textureCount);

    struct FTextureQuery {
        PPE::FFilename FFilename;
        bool KeepData   : 1;
        bool UseSRGB    : 1;
    };

    STACKLOCAL_POD_ARRAY(FTextureQuery, queries, textureCount);

    size_t i = 0;
    for (TPair<const FFilename, TUniquePtr<FTextureEntry> >& entry : _textures) {
        TUniquePtr<FTextureEntry>& pentry = entry.second;
        Assert(pentry);
        Assert(pentry->Filename() == entry.first);

        new ((void *)&queries[i].Filename) PPE::FFilename(pentry->Filename());
        queries[i].UseSRGB = pentry->UseSRGB();
        queries[i].KeepData = pentry->KeepData();

        UnloadTextureEntry_(pentry);

        ++i;
    }
    Assert(textureCount == i);

    _textures.clear();

    for (const FTextureQuery& query : queries) {
        FTextureEntry *const pentry = LoadTexture_ASync_(query.Filename, query.UseSRGB, query.KeepData);
        Assert(pentry);
        TextureEntry_InsertMRU_(&_lru, &_mru, pentry);
        query.Filename.~FFilename();
    }
}
//----------------------------------------------------------------------------
void FTextureCache::Start(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(!_device);

    LOG(Info, L"[FTextureCache] Starting with device <{0}> ...",
        device );

    _device = device;
    _heap.Start();
}
//----------------------------------------------------------------------------
void FTextureCache::Shutdown(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(device == _device);

    LOG(Info, L"[FTextureCache] Shutting down with device <{0}> ...",
        device );

    _completionPort.WaitAll();

    Clear();

    if (_fallbackTexture2D) {
        _fallbackTexture2D->Destroy(_device);
        _fallbackTexture2D = nullptr;
    }
    if (_fallbackTextureCube) {
        _fallbackTextureCube->Destroy(_device);
        _fallbackTextureCube = nullptr;
    }

    _heap.Shutdown();
    _device = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
