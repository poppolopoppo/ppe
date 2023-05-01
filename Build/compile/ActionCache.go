package compile

import (
	. "build/utils"
	"fmt"
	"io"
	"os"
	"strings"
	"sync/atomic"

	"archive/zip"

	"github.com/klauspost/compress/zstd"
)

/***************************************
 * ActionCache
 ***************************************/

type ActionCache interface {
	GetCachePath() Directory
	GetCacheStats() ActionCacheStats
	CacheRead(a *ActionRules, artifacts FileSet) (ActionCacheKey, error)
	CacheWrite(action BuildAlias, key ActionCacheKey, artifacts FileSet, inputs FileSet) error
	AsyncCacheWrite(node BuildNode, key ActionCacheKey, artifacts FileSet) error
}

type actionCache struct {
	path  Directory
	seed  Fingerprint
	stats ActionCacheStats
}

var GetActionCache = Memoize(func() ActionCache {
	result := BuildActionCache(UFS.Cache).Build(CommandEnv.BuildGraph())
	if result.Failure() == nil {
		// print cache stats upon exit if specified on command-line
		if GetCommandFlags().Summary.Get() {
			CommandEnv.OnExit(func(*CommandEnvT) error {
				result.Success().stats.Print()
				return nil
			})
		}
	}
	return result.Success()
})

func BuildActionCache(path Directory) BuildFactoryTyped[*actionCache] {
	return func(bi BuildInitializer) (*actionCache, error) {
		return &actionCache{
			path: path,
		}, nil
	}
}

func (x *actionCache) Alias() BuildAlias {
	return MakeBuildAlias("Cache", "Actions", x.path.String())
}
func (x *actionCache) Serialize(ar Archive) {
	ar.Serializable(&x.path)
}
func (x *actionCache) Build(bc BuildContext) error {
	return CreateDirectory(bc, x.path)
}

func (x *actionCache) GetCacheStats() ActionCacheStats {
	return x.stats
}
func (x *actionCache) GetCachePath() Directory {
	return x.path
}

func (x *actionCache) CacheRead(a *ActionRules, artifacts FileSet) (key ActionCacheKey, err error) {
	readStat := StartBuildStats()
	defer x.stats.CacheRead.Append(readStat)
	defer GlobalChromeTracer.BeginScope(a.Alias(), "CacheRead", 1234)()

	key = x.makeActionKey(a)
	entry, err := x.fetchCacheEntry(key, false)
	if err == nil {
		inflateStat := StartBuildStats()
		if err = entry.CacheRead(a, artifacts); err == nil {
			x.stats.CacheInflate.Append(inflateStat)
		}
	}

	if err == nil {
		LogTrace("action-cache: cache hit for %q", a.Alias())
		LogDebug("action-cache: %q has the following artifacts ->\n\t - %v", a.Alias(), MakeStringer(func() string {
			return JoinString("\n\t - ", artifacts...)
		}))

		atomic.AddInt32(&x.stats.CacheHit, 1)
	} else {
		atomic.AddInt32(&x.stats.CacheMiss, 1)
	}
	return
}
func (x *actionCache) CacheWrite(action BuildAlias, key ActionCacheKey, artifacts FileSet, inputs FileSet) (err error) {
	scopedStat := StartBuildStats()
	defer x.stats.CacheWrite.Append(scopedStat)
	defer GlobalChromeTracer.BeginScope(action, "CacheWrite", 1234)()

	if entry, err := x.fetchCacheEntry(key, true); err == nil {
		deflateStat := StartBuildStats()

		var dirty bool
		if dirty, err = entry.CacheWrite(x.path, inputs, artifacts); err == nil {
			x.stats.CacheDeflate.Append(deflateStat)

			if dirty {
				if err = entry.writeCacheEntry(x.path); err == nil {
					atomic.AddInt32(&x.stats.CacheStore, 1)
				}
			}
		}
	}

	if err == nil {
		LogTrace("action-cache: cache store for %q", action)
		LogDebug("action-cache: %q has the following artifacts ->\n\t - %v", action, MakeStringer(func() string {
			return JoinString("\n\t - ", artifacts...)
		}))
	} else {
		LogError("action-cache: failed to cache in store %q with %v", action, err)
	}
	return
}
func (x *actionCache) AsyncCacheWrite(node BuildNode, cacheKey ActionCacheKey, artifacts FileSet) error {
	action := node.Alias()
	GetGlobalWorkerPool().Queue(func() {
		inputs, err := CommandEnv.BuildGraph().GetDependencyInputFiles(action)
		if err == nil {
			inputs.Sort()
			err = x.CacheWrite(action, cacheKey, artifacts, inputs)
		}
		LogPanicIfFailed(err)
	})
	return nil
}
func (x *actionCache) fetchCacheEntry(cacheKey ActionCacheKey, writing bool) (ActionCacheEntry, error) {
	entry := ActionCacheEntry{Key: cacheKey}
	if err := entry.readCacheEntry(x.path); err != nil {
		if writing {
			err = nil
		} else {
			return entry, err
		}
	}
	return entry, nil
}
func (x *actionCache) makeActionKey(a *ActionRules) ActionCacheKey {
	bg := CommandEnv.BuildGraph()

	fingerprint, err := SerializeAnyFingerprint(func(ar Archive) error {
		// serialize action inputs
		a.Serialize(ar)

		// serialize all input files content
		for _, it := range Map(func(it Filename) Future[*FileDigest] {
			return BuildFileDigest(it).Prepare(bg)
		}, a.Inputs...) {
			result := it.Join()
			if err := result.Failure(); err != nil {
				return err
			}

			ar.Serializable(&result.Success().Digest)
			LogDebug("action-cache: file digest for %q is %v", result.Success().Source, result.Success().Digest.ShortString())
		}

		return nil
	}, x.seed)
	LogPanicIfFailed(err)

	LogDebug("action-cache: action %q key is %v", a.Alias(), fingerprint.ShortString())
	return ActionCacheKey(fingerprint)
}

/***************************************
 * ActionCacheKey
 ***************************************/

type ActionCacheKey Fingerprint

func (x *ActionCacheKey) Serialize(ar Archive) {
	ar.Serializable((*Fingerprint)(x))
}
func (x ActionCacheKey) String() string {
	return x.GetFingerprint().ShortString()
}
func (x ActionCacheKey) GetFingerprint() Fingerprint {
	return (Fingerprint)(x)
}

var actionCacheEntryExtname = ".cache" + bulkCompressDefault.Extname

func (x ActionCacheKey) GetEntryPath(cachePath Directory) Filename {
	return makeCachePath(cachePath, x.GetFingerprint(), actionCacheEntryExtname)
}

func makeCachePath(cachePath Directory, h Fingerprint, extname string) Filename {
	str := h.String()
	return cachePath.Folder(str[0:2]).Folder(str[2:4]).File(str).ReplaceExt(extname)
}

/***************************************
 * ActionCacheBulk
 ***************************************/

type bulkCompress struct {
	Extname      string
	Method       uint16
	Compressor   zip.Compressor
	Decompressor zip.Decompressor
}

var bulkCompressLz4 bulkCompress = bulkCompress{
	Extname:      ".lz4",
	Method:       0xFFFF,
	Compressor:   NewPooledLz4Writer,
	Decompressor: NewPooledLz4Reader,
}
var bulkCompressZStd bulkCompress = bulkCompress{
	Extname:      ".zstd",
	Method:       zstd.ZipMethodWinZip,
	Compressor:   zstd.ZipCompressor(zstd.WithEncoderLevel(zstd.SpeedFastest)),
	Decompressor: zstd.ZipDecompressor(),
}

var bulkCompressDefault bulkCompress = func() bulkCompress {
	if UseLz4OverZStd {
		return bulkCompressLz4
	} else {
		return bulkCompressZStd
	}
}()

type ActionCacheBulk struct {
	Path   Filename
	Inputs []FileDigest
}

var actionCacheBulkExtname = ".bulk" + bulkCompressDefault.Extname

func NewActionCacheBulk(cachePath Directory, key ActionCacheKey, inputs FileSet) (bulk ActionCacheBulk, err error) {
	bulk.Inputs = make([]FileDigest, len(inputs))

	var fingerprint Fingerprint
	fingerprint, err = SerializeAnyFingerprint(func(ar Archive) error {
		bg := CommandEnv.BuildGraph()
		for i, future := range Map(func(it Filename) Future[*FileDigest] {
			return BuildFileDigest(it).Prepare(bg)
		}, inputs...) {
			result := future.Join()
			if err = result.Failure(); err != nil {
				return err
			}

			bulk.Inputs[i] = *result.Success()
		}
		return nil
	}, key.GetFingerprint())

	if err == nil {
		bulk.Path = makeCachePath(cachePath, fingerprint, actionCacheBulkExtname)
	}
	return
}
func (x *ActionCacheBulk) Equals(y ActionCacheBulk) bool {
	return x.Path.Equals(y.Path)
}
func (x *ActionCacheBulk) CacheHit() bool {
	bg := CommandEnv.BuildGraph()
	for i, future := range Map(func(it FileDigest) Future[*FileDigest] {
		return BuildFileDigest(it.Source).Prepare(bg)
	}, x.Inputs...) {
		result := future.Join()
		if err := result.Failure(); err != nil {
			return false
		}
		if result.Success().Digest != x.Inputs[i].Digest {
			LogDebug("action-cache: cache miss due to %q, was %v ang got %v",
				x.Inputs[i].Source, x.Inputs[i].Digest.ShortString(), result.Success().Digest.ShortString())
			return false
		}
	}
	return true
}
func (x *ActionCacheBulk) Deflate(artifacts ...Filename) error {
	benchmark := LogBenchmark("action-cache: deflate bulk storage %q", x.Path.Basename)
	defer benchmark.Close()

	return UFS.CreateBuffered(x.Path, func(w io.Writer) error {
		zw := zip.NewWriter(w)
		zw.RegisterCompressor(bulkCompressDefault.Method, bulkCompressDefault.Compressor)

		for _, file := range artifacts {
			info, err := file.Info()
			if err != nil {
				return err
			}

			name := MakeLocalFilename(file)
			w, err := zw.CreateHeader(&zip.FileHeader{
				Name:     name,
				Method:   bulkCompressDefault.Method,
				Modified: info.ModTime(), // save modified time stable
			})
			if err != nil {
				return err
			}

			if err := UFS.Open(file, func(r io.Reader) error {
				return CopyWithProgress(file.String(), info.Size(), w, r)
			}); err != nil {
				return err
			}
		}

		return zw.Close()
	})
}
func (x *ActionCacheBulk) Inflate() (FileSet, error) {
	benchmark := LogBenchmark("action-cache: inflate bulk storage %q", x.Path.Basename)
	defer benchmark.Close()

	var artifacts FileSet
	return artifacts, UFS.OpenFile(x.Path, func(r *os.File) error {
		info, err := r.Stat()
		if err != nil {
			return err
		}

		zr, err := zip.NewReader(r, info.Size())
		if err != nil {
			return err
		}
		zr.RegisterDecompressor(bulkCompressLz4.Method, bulkCompressLz4.Decompressor)
		zr.RegisterDecompressor(bulkCompressZStd.Method, bulkCompressZStd.Decompressor)

		for _, file := range zr.File {
			rc, err := file.Open()
			if err != nil {
				return err
			}

			dst := UFS.Root.AbsoluteFile(file.Name)
			err = UFS.Create(dst, func(w io.Writer) error {
				return CopyWithProgress(dst.String(), int64(file.UncompressedSize64), w, rc)
			})
			rc.Close()

			if err != nil {
				return err
			}

			// restore modified time to keep other build stable
			UFS.SetMTime(dst, file.Modified)

			artifacts.Append(dst)
		}

		return nil
	})
}
func (x *ActionCacheBulk) Serialize(ar Archive) {
	ar.Serializable(&x.Path)
	SerializeSlice(ar, &x.Inputs)
}

/***************************************
 * ActionCacheEntry
 ***************************************/

type ActionCacheEntry struct {
	Key   ActionCacheKey
	Bulks []ActionCacheBulk
}

func (x *ActionCacheEntry) Serialize(ar Archive) {
	ar.Serializable((*Fingerprint)(&x.Key))
	SerializeSlice(ar, &x.Bulks)
}
func (x *ActionCacheEntry) CacheRead(a *ActionRules, artifacts FileSet) error {
	for _, bulk := range x.Bulks {
		if bulk.CacheHit() {
			retrieved, err := bulk.Inflate()

			if err == nil && !retrieved.Equals(artifacts) {
				err = fmt.Errorf("action-cache: artifacts file set do not match for action %q", a.Alias())
			}
			return err
		}
	}
	return fmt.Errorf("action-cache: cache miss for action %q, recompiling", a.Alias())
}
func (x *ActionCacheEntry) CacheWrite(cachePath Directory, inputs FileSet, artifacts FileSet) (bool, error) {
	bulk, err := NewActionCacheBulk(cachePath, x.Key, inputs)
	if err != nil {
		return false, err
	}

	dirty := true
	for i, b := range x.Bulks {
		// check if the same bulk is already present
		if b.Equals(bulk) {
			dirty = len(b.Inputs) != len(bulk.Inputs)
			if !dirty {
				// check if bulk with the same has also the same inputs
				for j, it := range b.Inputs {
					jt := bulk.Inputs[j]
					if !it.Source.Equals(jt.Source) || it.Digest != jt.Digest {
						dirty = true
						break
					}
				}
			}
			if dirty {
				x.Bulks[i] = bulk
			}
			break
		}
	}

	if dirty {
		x.Bulks = append(x.Bulks, bulk)
		err = bulk.Deflate(artifacts...)
	}
	return dirty, err
}

func (x *ActionCacheEntry) readCacheEntry(cachePath Directory) error {
	path := x.Key.GetEntryPath(cachePath)
	if !path.Exists() {
		return fmt.Errorf("action-cache: no cache entry with key %q", x.Key)
	}

	benchmark := LogBenchmark("action-cache: read cache entry with key %q", x.Key)
	defer benchmark.Close()

	return UFS.Open(path, func(r io.Reader) error {
		_, err := CompressedArchiveFileRead(r, func(ar Archive) {
			ar.Serializable(x)
		})
		return err
	})

}
func (x *ActionCacheEntry) writeCacheEntry(cachePath Directory) error {
	path := x.Key.GetEntryPath(cachePath)

	benchmark := LogBenchmark("action-cache: store cache entry with key %q", x.Key)
	defer benchmark.Close()

	return UFS.Create(path, func(w io.Writer) error {
		return CompressedArchiveFileWrite(w, func(ar Archive) {
			ar.Serializable(x)
		})
	})
}

/***************************************
 * ActionCacheStats
 ***************************************/

type ActionCacheStats struct {
	CacheRead    BuildStats
	CacheInflate BuildStats

	CacheWrite   BuildStats
	CacheDeflate BuildStats

	CacheHit   int32
	CacheMiss  int32
	CacheStore int32
}

func (x ActionCacheStats) Print() {
	LogForwardf("\nAction cache was hit %d times and missed %d times, stored %d new cache entries (hit rate: %.3f%%)",
		x.CacheHit, x.CacheMiss, x.CacheStore,
		100*float32(x.CacheHit)/float32(x.CacheHit+x.CacheMiss))

	LogForwardf("   READ <==  %10.4f seconds - %3d cache entries",
		x.CacheRead.Duration.Exclusive.Seconds(), x.CacheRead.Count)
	LogForwardf("INFLATE <-   %10.4f seconds - %3d cache bulks",
		x.CacheInflate.Duration.Exclusive.Seconds(), x.CacheInflate.Count)

	LogForwardf("  WRITE ==>  %10.4f seconds - %3d cache entries",
		x.CacheWrite.Duration.Exclusive.Seconds(), x.CacheWrite.Count)
	LogForwardf("DEFLATE  ->  %10.4f seconds - %3d cache bulks",
		x.CacheDeflate.Duration.Exclusive.Seconds(), x.CacheDeflate.Count)
}

/***************************************
 * CacheModeType
 ***************************************/

type CacheModeType int32

const (
	CACHE_INHERIT CacheModeType = iota
	CACHE_NONE
	CACHE_READ
	CACHE_READWRITE
)

func CacheModeTypes() []CacheModeType {
	return []CacheModeType{
		CACHE_INHERIT,
		CACHE_NONE,
		CACHE_READ,
		CACHE_READWRITE,
	}
}
func (x CacheModeType) String() string {
	switch x {
	case CACHE_INHERIT:
		return "INHERIT"
	case CACHE_NONE:
		return "NONE"
	case CACHE_READ:
		return "READ"
	case CACHE_READWRITE:
		return "READWRITE"
	default:
		UnexpectedValue(x)
		return ""
	}
}
func (x CacheModeType) IsInheritable() bool {
	return x == CACHE_INHERIT
}
func (x *CacheModeType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case CACHE_INHERIT.String():
		*x = CACHE_INHERIT
	case CACHE_NONE.String():
		*x = CACHE_NONE
	case CACHE_READ.String():
		*x = CACHE_READ
	case CACHE_READWRITE.String():
		*x = CACHE_READWRITE
	default:
		err = MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *CacheModeType) Serialize(ar Archive) {
	ar.Int32((*int32)(x))
}
func (x CacheModeType) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *CacheModeType) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}
func (x *CacheModeType) AutoComplete(in AutoComplete) {
	for _, it := range CacheModeTypes() {
		in.Add(it.String())
	}
}

func (x CacheModeType) HasRead() bool {
	switch x {
	case CACHE_READ, CACHE_READWRITE:
		return true
	case CACHE_INHERIT, CACHE_NONE:
	default:
		UnexpectedValuePanic(x, x)
	}
	return false
}
func (x CacheModeType) HasWrite() bool {
	switch x {
	case CACHE_READWRITE:
		return true
	case CACHE_INHERIT, CACHE_NONE, CACHE_READ:
	default:
		UnexpectedValuePanic(x, x)
	}
	return false
}
