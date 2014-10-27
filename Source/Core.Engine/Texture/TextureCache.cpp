#include "stdafx.h"

#include "TextureCache.h"

#include "TextureLoader.h"

#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"
#include "Core.Graphics/Device/Texture/Texture.h"
#include "Core.Graphics/Device/Texture/Texture2D.h"

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
    TextureCache::TextureEntry **lru,
    TextureCache::TextureEntry **mru,
    TextureCache::TextureEntry *node ) {
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
    TextureCache::TextureEntry **lru,
    TextureCache::TextureEntry **mru,
    TextureCache::TextureEntry *node ) {
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
TextureCache::TextureData::TextureData(TextureCache *cache)
:   _pixels(TexturePixelsAllocator(&cache->_heap)) {}
//----------------------------------------------------------------------------
TextureCache::TextureData::~TextureData() {}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(TextureCache::TextureData, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TextureCache::TextureEntry::TextureEntry(const Core::Filename& filename, bool useSRGB, bool keepData)
:   _filename(filename)
,   _prev(nullptr) {
    _nextWFlags.Reset(nullptr, useSRGB, keepData);

    LOG(Information, L"[TextureCache] Loading texture '{0}', SRGB = {1:A}, Keep data = {2:A} ...",
        _filename, UseSRGB(), KeepData() );
}
//----------------------------------------------------------------------------
TextureCache::TextureEntry::~TextureEntry() {
    if (_texture)
        RemoveRef_AssertReachZero(_texture);

    LOG(Information, L"[TextureCache] Unloading texture '{0}' ...",
        _filename, UseSRGB(), KeepData() );
}
//----------------------------------------------------------------------------
const Graphics::Texture2D *TextureCache::TextureEntry::AsTexture2D() const {
    Assert(_texture);
    return checked_cast<const Graphics::Texture2D *>(_texture.get());
}
//----------------------------------------------------------------------------
void TextureCache::TextureEntry::SetData_(const TextureData *data) {
    _data.reset(data);
}
//----------------------------------------------------------------------------
void TextureCache::TextureEntry::SetTexture_(Graphics::Texture *texture) {
    Assert(texture);
    Assert(!_texture);

    _texture.reset(texture);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(TextureCache::TextureEntry, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TaskResult TextureCache::TextureEntryAsyncJob::Invoke(const TaskContext& ) {
    Assert(_pentry);

    Texture2DLoader loader;
    TextureCache::TextureData *pdata = new TextureCache::TextureData(_cache);
    if (!loader.Read(pdata->Pixels(), _pentry->Filename()))
        AssertNotImplemented();

    pdata->Header() = loader.Header();
    _pentry->SetData_(pdata);

    std::unique_lock<std::mutex> scopeLock(_cache->_barrier);
    _cache->_pjobs.push_back(this);

    return TaskResult::Succeed;
}
//----------------------------------------------------------------------------
void TextureCache::TextureEntryAsyncJob::Finalize_MainThread(Graphics::IDeviceAPIEncapsulator *device) const {
    Assert(_pentry->Data());

    Texture2DLoader loader(_pentry->Data()->Header());

    const MemoryView<const u8> pixels = _pentry->Data()->Pixels().MakeConstView();
    Graphics::Texture2D *const texture2D = loader.CreateTexture2D(
        device, pixels, _pentry->Filename().ToString(), _pentry->UseSRGB() );
    Assert(texture2D);

    _pentry->SetTexture_(texture2D);

    if (!_pentry->KeepData())
        _pentry->SetData_(nullptr);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(TextureCache::TextureEntryAsyncJob, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TextureCache::TextureCache(size_t capacityInBytes)
:   _device(nullptr)
,   _heap(capacityInBytes, true)
,   _completionPort("TextureCache", IOThreadPool::Instance().Evaluator())
,   _lru(nullptr)
,   _mru(nullptr) {
    Assert(capacityInBytes >= Units::Storage::Megabytes(128).Value());

    _pjobs.reserve(128);
}
//----------------------------------------------------------------------------
TextureCache::~TextureCache() {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
void TextureCache::SetFallbackTexture2D(const Filename& path) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!path.empty());
    Assert(_device);

    const bool useSRGB = true;
    const bool keepData = false;

    TextureEntry *pentry = LoadTexture2D_Sync_(path, useSRGB, keepData);
    Assert(pentry);

    SetFallbackTexture2D(pentry->AsTexture2D());
}
//----------------------------------------------------------------------------
void TextureCache::SetFallbackTexture2D(const Graphics::Texture2D *texture) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(texture);
    Assert(_device);

    _fallbackTexture2D = texture;
}
//----------------------------------------------------------------------------
void TextureCache::Update() {
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
TextureCache::TextureEntry *TextureCache::LoadTexture2D_Sync_(const Filename& filename, bool useSRGB, bool keepData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(_textures.end() == _textures.find(filename));

    UniquePtr<TextureEntry>& pentry = _textures[filename];
    Assert(!pentry);

    pentry.reset(new TextureEntry(filename, useSRGB, keepData));

    Texture2DLoader loader;
    if (keepData) {
        TextureCache::TextureData *pdata = new TextureCache::TextureData(this);
        if (!loader.Read(pdata->Pixels(), pentry->Filename()))
            AssertNotImplemented();

        pdata->Header() = loader.Header();
        pentry->SetData_(pdata);

        const MemoryView<const u8> pixels = pdata->Pixels().MakeConstView();
        Graphics::Texture2D *const texture2D = loader.CreateTexture2D(_device, pixels, filename.ToString(), useSRGB);
        Assert(texture2D);

        pentry->SetTexture_(texture2D);
    }
    else {
        TextureCache::TextureData data(this);
        if (!loader.Read(data.Pixels(), pentry->Filename()))
            AssertNotReached();

        const MemoryView<const u8> pixels = data.Pixels().MakeConstView();
        Graphics::Texture2D *const texture2D = loader.CreateTexture2D(_device, pixels, filename.ToString(), useSRGB);
        Assert(texture2D);

        pentry->SetTexture_(texture2D);
    }

    Assert(pentry->Texture());
    return pentry.get();
}
//----------------------------------------------------------------------------
TextureCache::TextureEntry *TextureCache::LoadTexture2D_ASync_(const Filename& filename, bool useSRGB, bool keepData) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(_textures.end() == _textures.find(filename));

    UniquePtr<TextureEntry>& pentry = _textures[filename];
    Assert(!pentry);

    pentry.reset(new TextureEntry(filename, useSRGB, keepData));

    _completionPort.Produce(new TextureEntryAsyncJob(this, pentry.get()));

    return pentry.get();
}
//----------------------------------------------------------------------------
void TextureCache::UnloadTextureEntry_(UniquePtr<TextureEntry>& pentry) {
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
void TextureCache::UnloadTexture(const Filename& filename) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(_device);

    _completionPort.WaitAll();

    const auto it = _textures.find(filename);
    AssertRelease(_textures.end() != it);

    UniquePtr<TextureEntry>& pentry = it->second;
    Assert(pentry);
    Assert(pentry->Filename() == it->first);

    UnloadTextureEntry_(pentry);

    _textures.erase(it);
}
//----------------------------------------------------------------------------
void TextureCache::UnloadLRUTextures() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    while (_lru && 1 == _lru->_texture->RefCount())
        UnloadTexture(_lru->Filename());
}
//----------------------------------------------------------------------------
void TextureCache::PrepareTexture2D(const Filename& filename, bool useSRGB/* = false */, bool asynchronous/* = true */) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(_device);

    TextureEntry *pentry = nullptr;

    const bool keepData = false;

    const auto it = _textures.find(filename);
    if (_textures.end() == it) {
        if (VFS_FileExists(filename))
            pentry = (asynchronous)
                ? LoadTexture2D_ASync_(filename, useSRGB, keepData)
                : LoadTexture2D_Sync_(filename, useSRGB, keepData);
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
bool TextureCache::FetchTexture2D(const Filename& filename, const Graphics::Texture2D **texture) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!filename.empty());
    Assert(texture);
    Assert(_device);

    const auto it = _textures.find(filename);
    if (_textures.end() == it)
        return false;

    const UniquePtr<TextureEntry>& pentry = it->second;
    Assert(pentry);
    Assert(pentry->Filename() == filename);

    if (!pentry->Available())
        return false;

    *texture = pentry->AsTexture2D();
    return true;
}
//----------------------------------------------------------------------------
const Graphics::Texture2D *TextureCache::FetchTexture2D_Fallback(const Filename& filename) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    const Graphics::Texture2D *texture = nullptr;
    if (!FetchTexture2D(filename, &texture))
        texture = _fallbackTexture2D.get();

    Assert(texture);
    return texture;
}
//----------------------------------------------------------------------------
void TextureCache::Clear() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);

    _completionPort.WaitAll();

    _fallbackTexture2D = nullptr;

    for (Pair<const Filename, UniquePtr<TextureEntry> >& entry : _textures) {
        UniquePtr<TextureEntry>& pentry = entry.second;
        Assert(pentry);
        Assert(pentry->Filename() == entry.first);

        UnloadTextureEntry_(pentry);
    }

    _textures.clear();
}
//----------------------------------------------------------------------------
void TextureCache::Start(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(!_device);

    LOG(Information, L"[TextureCache] Starting with device <{0}> ...",
        device );

    _device = device;
    _heap.Start();
}
//----------------------------------------------------------------------------
void TextureCache::Shutdown(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device);
    Assert(device == _device);

    LOG(Information, L"[TextureCache] Shutting down with device <{0}> ...",
        device );

    _completionPort.WaitAll();

    Clear();

    _heap.Shutdown();
    _device = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
