#pragma once

#include "Core.Engine/Engine.h"

#include <mutex>

#include "Core/Allocator/BuddyHeap.h"
#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/PointerWFlags.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Thread/LockFreeCircularArray.h"
#include "Core/Thread/Task/Task.h"
#include "Core/Thread/Task/TaskCompletionPort.h"
#include "Core/Thread/ThreadPool.h"

#include "Core.Engine/Texture/TextureLoader.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(Texture);
FWD_REFPTR(Texture2D);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TextureCache : Meta::ThreadResource {
public:
    typedef BuddyAllocator<u8, ThreadSafeBuddyHeap >
            TexturePixelsAllocator;
    typedef RawStorage<u8, DECORATE_ALLOCATOR(Texture, TexturePixelsAllocator) >
            TexturePixels;

    class TextureData {
    public:
        TextureData(TextureCache *cache);
        ~TextureData();

        TextureHeader& Header() { return _header; }
        TexturePixels& Pixels() { return _pixels; }

        const TextureHeader& Header() const { return _header; }
        const TexturePixels& Pixels() const { return _pixels; }

        SINGLETON_POOL_ALLOCATED_DECL(TextureData);

    private:
        TextureHeader _header;
        TexturePixels _pixels;
    };

    class TextureEntry {
    public:
        friend class TextureCache;
        friend class TextureEntryAsyncJob;

        TextureEntry(const Core::Filename& filename, bool useSRGB, bool keepData);
        ~TextureEntry();

        TextureEntry(const TextureEntry& ) = delete;
        TextureEntry& operator =(const TextureEntry& ) = delete;

        const Core::Filename& Filename() const { return _filename; }

        bool UseSRGB() const {return _nextWFlags.Flag0(); }
        bool KeepData() const {return _nextWFlags.Flag1(); }

        TextureEntry *Next() { return _nextWFlags.Get(); }
        TextureEntry *Prev() { return _prev; }

        const TextureEntry *Next() const { return _nextWFlags.Get(); }
        const TextureEntry *Prev() const { return _prev; }

        void SetNext(TextureEntry *entry) { _nextWFlags.Set(entry); }
        void SetPrev(TextureEntry *entry) { _prev = entry; }

        bool Available() const { return nullptr != _texture.get(); }

        const TextureData *Data() const { return _data.get(); }
        const Graphics::Texture *Texture() const { return _texture.get(); }

        const Graphics::Texture2D *AsTexture2D() const;

        SINGLETON_POOL_ALLOCATED_DECL(TextureEntry);

    private:
        void SetData_(const TextureData *data);
        void SetTexture_(Graphics::Texture *texture);
        void SetKeepData_(bool value);

        const Core::Filename _filename;

        Graphics::PTexture _texture;
        UniquePtr<const TextureData> _data;

        Meta::PointerWFlags<TextureEntry> _nextWFlags;
        TextureEntry *_prev;
    };

    FWD_REFPTR(TextureEntryAsyncJob);
    class TextureEntryAsyncJob : public ITask {
    public:
        explicit TextureEntryAsyncJob(TextureCache *cache, TextureCache::TextureEntry *pentry)
        :   _cache(cache), _pentry(pentry) {}
        virtual ~TextureEntryAsyncJob() {}

        virtual TaskResult Invoke(const TaskContext& ctx) override;
        void Finalize_MainThread(Graphics::IDeviceAPIEncapsulator *device) const;

        SINGLETON_POOL_ALLOCATED_DECL(TextureEntryAsyncJob);

    private:
        TextureCache *_cache;
        TextureCache::TextureEntry *_pentry;
    };

    explicit TextureCache(size_t capacityInBytes);
    ~TextureCache();

    TextureCache(const TextureCache& ) = delete;
    TextureCache& operator =(const TextureCache& ) = delete;

    size_t Count() const { return _textures.size(); }
    const ThreadSafeBuddyHeap *Heap() const { return &_heap; }

    void SetFallbackTexture2D(const Filename& path);
    void SetFallbackTexture2D(const Graphics::Texture2D *texture);

    void Update();

    void PrepareTexture2D(const Filename& filename, bool useSRGB = false, bool asynchronous = true);

    bool FetchTexture2D(const Filename& filename, const Graphics::Texture2D **texture) const;
    const Graphics::Texture2D *FetchTexture2D_Fallback(const Filename& filename) const;

    void UnloadTexture(const Filename& filename);
    void UnloadLRUTextures();

    void Clear();

    void Start(Graphics::IDeviceAPIEncapsulator *device);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device);

private:
    TextureEntry *LoadTexture2D_Sync_(const Filename& filename, bool useSRGB, bool keepData);
    TextureEntry *LoadTexture2D_ASync_(const Filename& filename, bool useSRGB, bool keepData);
    void UnloadTextureEntry_(UniquePtr<TextureEntry>& pentry);

private:
    Graphics::IDeviceAPIEncapsulator *_device;

    ThreadSafeBuddyHeap _heap;
    TaskCompletionPort _completionPort;
    HASHMAP_THREAD_LOCAL(Texture, Filename, UniquePtr<TextureEntry>) _textures;

    TextureEntry *_lru;
    TextureEntry *_mru;

    Graphics::PCTexture2D _fallbackTexture2D;

    std::mutex _barrier;
    VECTOR(Texture, PCTextureEntryAsyncJob) _pjobs;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
