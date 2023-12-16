#pragma once

#include "Core.Engine/Engine.h"

#include <mutex>

#include "Core/Allocator/BuddyHeap.h"
#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/PointerWFlags.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Thread/Task/Task.h"
#include "Core/Thread/ThreadPool.h"

#include "Core.Engine/Texture/TextureLoader.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(Texture);
FWD_REFPTR(Texture2D);
FWD_REFPTR(TextureCube);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTextureCache : Meta::FThreadResource {
public:
    typedef TBuddyAllocator<u8, FThreadSafeBuddyHeap >
            TexturePixelsAllocator;
    typedef TRawStorage<u8, DECORATE_ALLOCATOR(FTexture, TexturePixelsAllocator) >
            TexturePixels;

    class FTextureData {
    public:
        FTextureData(FTextureCache *cache);
        ~FTextureData();

        FTextureHeader& FHeader() { return _header; }
        TexturePixels& Pixels() { return _pixels; }

        const FTextureHeader& FHeader() const { return _header; }
        const TexturePixels& Pixels() const { return _pixels; }

        SINGLETON_POOL_ALLOCATED_DECL();

    private:
        FTextureHeader _header;
        TexturePixels _pixels;
    };

    class FTextureEntry {
    public:
        friend class FTextureCache;
        friend class FTextureEntryAsyncJob;

        FTextureEntry(const PPE::FFilename& filename, bool useSRGB, bool keepData);
        ~FTextureEntry();

        FTextureEntry(const FTextureEntry& ) = delete;
        FTextureEntry& operator =(const FTextureEntry& ) = delete;

        const PPE::FFilename& FFilename() const { return _filename; }

        bool UseSRGB() const {return _nextWFlags.Flag0(); }
        bool KeepData() const {return _nextWFlags.Flag1(); }

        FTextureEntry *Next() { return _nextWFlags.Get(); }
        FTextureEntry *Prev() { return _prev; }

        const FTextureEntry *Next() const { return _nextWFlags.Get(); }
        const FTextureEntry *Prev() const { return _prev; }

        void SetNext(FTextureEntry *entry) { _nextWFlags.Set(entry); }
        void SetPrev(FTextureEntry *entry) { _prev = entry; }

        bool Available() const { return nullptr != _texture.get(); }

        const FTextureData *Data() const { return _data.get(); }
        const Graphics::FTexture *FTexture() const { return _texture.get(); }

        const Graphics::FTexture2D *AsTexture2D() const;
        const Graphics::FTextureCube *AsTextureCube() const;

        SINGLETON_POOL_ALLOCATED_DECL();

    private:
        void SetData_(const FTextureData *data);
        void SetTexture_(Graphics::FTexture *texture);
        void SetKeepData_(bool value);

        const PPE::FFilename _filename;

        Graphics::PTexture _texture;
        TUniquePtr<const FTextureData> _data;

        Meta::TPointerWFlags<FTextureEntry> _nextWFlags;
        FTextureEntry *_prev;
    };

    FWD_REFPTR(TextureEntryAsyncJob);
    class FTextureEntryAsyncJob : public ITask {
    public:
        explicit FTextureEntryAsyncJob(FTextureCache *cache, FTextureCache::FTextureEntry *pentry)
        :   _cache(cache), _pentry(pentry) {}
        virtual ~FTextureEntryAsyncJob() {}

        virtual TaskResult Invoke(const TaskContext& ctx) override;
        void Finalize_MainThread(Graphics::IDeviceAPIEncapsulator *device) const;

        SINGLETON_POOL_ALLOCATED_DECL();

    private:
        FTextureCache *_cache;
        FTextureCache::FTextureEntry *_pentry;
    };

    explicit FTextureCache(size_t capacityInBytes);
    ~FTextureCache();

    FTextureCache(const FTextureCache& ) = delete;
    FTextureCache& operator =(const FTextureCache& ) = delete;

    size_t Count() const { return _textures.size(); }
    const FThreadSafeBuddyHeap *FHeap() const { return &_heap; }

    void SetFallbackTexture2D(const FFilename& path);
    void SetFallbackTexture2D(Graphics::FTexture2D *texture);

    void SetFallbackTextureCube(const FFilename& path);
    void SetFallbackTextureCube(Graphics::FTextureCube *texture);

    void Update();

    void PrepareTexture(const FFilename& filename, bool useSRGB = false, bool asynchronous = true);

    bool FetchTexture2D(const FFilename& filename, const Graphics::FTexture2D **texture) const;
    const Graphics::FTexture2D *FetchTexture2D_Fallback(const FFilename& filename) const;

    bool FetchTextureCube(const FFilename& filename, const Graphics::FTextureCube **texture) const;
    const Graphics::FTextureCube *FetchTextureCube_Fallback(const FFilename& filename) const;

    void UnloadTexture(const FFilename& filename);
    void UnloadLRUTextures();

    void Clear();
    void ReloadAllTextures();

    void Start(Graphics::IDeviceAPIEncapsulator *device);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device);

private:
    bool FetchTextureImpl_(const FFilename& filename, const Graphics::FTexture **texture) const;
    FTextureEntry *LoadTexture_Sync_(const FFilename& filename, bool useSRGB, bool keepData);
    FTextureEntry *LoadTexture_ASync_(const FFilename& filename, bool useSRGB, bool keepData);
    void UnloadTextureEntry_(TUniquePtr<FTextureEntry>& pentry);

private:
    Graphics::IDeviceAPIEncapsulator *_device;

    FThreadSafeBuddyHeap _heap;
    TaskCompletionPort _completionPort;
    HASHMAP_THREAD_LOCAL(FTexture, FFilename, TUniquePtr<FTextureEntry>) _textures;

    FTextureEntry *_lru;
    FTextureEntry *_mru;

    Graphics::PTexture2D _fallbackTexture2D;
    Graphics::PTextureCube _fallbackTextureCube;

    std::mutex _barrier;
    VECTOR(FTexture, PCTextureEntryAsyncJob) _pjobs;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
