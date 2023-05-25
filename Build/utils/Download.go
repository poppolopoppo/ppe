package utils

import (
	"fmt"
	"net/http"
	"net/url"
	"os"
	"regexp"
	"strconv"
)

var LogDownload = NewLogCategory("Download")

type DownloadMode int32

const (
	DOWNLOAD_DEFAULT DownloadMode = iota
	DOWNLOAD_REDIRECT
)

func (x *DownloadMode) Serialize(ar Archive) {
	ar.Int32((*int32)(x))
}
func (x DownloadMode) String() string {
	switch x {
	case DOWNLOAD_DEFAULT:
		return "DOWNLOAD_DEFAULT"
	case DOWNLOAD_REDIRECT:
		return "DOWNLOAD_REDIRECT"
	default:
		UnexpectedValue(x)
		return ""
	}
}

func BuildDownloader(uri string, dst Filename, mode DownloadMode) BuildFactoryTyped[*Downloader] {
	parsedUrl, err := url.Parse(uri)
	if err != nil {
		LogFatal("download: %v", err)
	}
	return MakeBuildFactory(func(bi BuildInitializer) (Downloader, error) {
		return Downloader{
			Source:      *parsedUrl,
			Destination: dst,
			Mode:        mode,
		}, nil
	})
}

type Downloader struct {
	Source      url.URL
	Destination Filename
	Mode        DownloadMode
}

func (dl *Downloader) Alias() BuildAlias {
	return MakeBuildAlias("Download", dl.Destination.String())
}
func (dl *Downloader) Build(bc BuildContext) error {
	var err error
	switch dl.Mode {
	case DOWNLOAD_DEFAULT:
		err = DownloadFile(dl.Destination, dl.Source)
	case DOWNLOAD_REDIRECT:
		err = DownloadHttpRedirect(dl.Destination, dl.Source)
	}

	if err == nil {
		err = bc.OutputFile(dl.Destination)
	}
	if err == nil { // avoid re-downloading after each rebuild
		bc.Timestamp(UFS.MTime(dl.Destination))
	}
	return err
}
func (dl *Downloader) Serialize(ar Archive) {
	if ar.Flags().IsLoading() {
		var uri string
		ar.String(&uri)

		parsedUrl, err := url.Parse(uri)
		if err == nil {
			dl.Source = *parsedUrl
		} else {
			ar.OnError(err)
		}
	} else {
		uri := dl.Source.String()
		ar.String(&uri)
	}

	ar.Serializable(&dl.Destination)
	ar.Serializable(&dl.Mode)
}

type downloadCacheResult interface {
	ShouldCache() bool
	error
}
type invalidCacheItem struct {
	error
}
type nonCachableResponse struct {
	error
}

func (invalidCacheItem) ShouldCache() bool {
	return true
}
func (nonCachableResponse) ShouldCache() bool {
	return false
}

func downloadFromCache(resp *http.Response) (Filename, downloadCacheResult) {
	var contentHash []string
	if contentHash = resp.Header.Values("Content-Md5"); contentHash == nil {
		contentHash = resp.Header.Values("X-Goog-Hash")
	}

	if contentHash != nil {
		uid, err := SerializeAnyFingerprint(func(ar Archive) error {
			for _, it := range contentHash {
				ar.String(&it)
			}
			return nil
		}, Fingerprint{})
		LogPanicIfFailed(LogDownload, err)

		inCache := UFS.Transient.Folder("DownloadCache").File(fmt.Sprintf("%v.bin", uid))
		if info, err := inCache.Info(); info != nil && err == nil {
			var totalSize int
			totalSize, err = strconv.Atoi(resp.Header.Get("Content-Length"))
			if err != nil {
				return inCache, nonCachableResponse{err}
			}

			if totalSize != int(info.Size()) {
				return inCache, invalidCacheItem{fmt.Errorf("%v: size don't match (%v != %v)", inCache, totalSize, info.Size())}
			}

			return inCache, nil // cache hit
		} else {
			return inCache, invalidCacheItem{fmt.Errorf("%v: entry does not exist", inCache)}
		}
	}
	return Filename{}, nonCachableResponse{fmt.Errorf("can't find content hash in http header")}
}

func DownloadFile(dst Filename, src url.URL) error {
	LogVerbose(LogDownload, "downloading url '%v' to '%v'...", src.String(), dst.String())

	cacheFile, shouldCache := Filename{}, false
	err := UFS.CreateFile(dst, func(w *os.File) error {
		client := http.Client{
			CheckRedirect: func(r *http.Request, _ []*http.Request) error {
				r.URL.Opaque = r.URL.Path
				return nil
			},
		}
		defer client.CloseIdleConnections()

		resp, err := client.Get(src.String())
		if err != nil {
			return err
		}
		defer resp.Body.Close()

		var cacheResult downloadCacheResult
		cacheFile, cacheResult = downloadFromCache(resp)
		if cacheResult == nil { // cache hit
			LogDebug(LogDownload, "cache hit on '%v'", cacheFile)

			return UFS.OpenFile(cacheFile, func(r *os.File) error {
				if info, err := r.Stat(); err == nil {
					SetMTime(w, info.ModTime()) // keep mtime consistent
				} else {
					return err
				}

				return TransientIoCopy(w, r)
			})

		} else { // cache miss
			shouldCache = cacheResult.ShouldCache() // cachable ?

			if enableInteractiveShell {
				var totalSize int
				totalSize, err = strconv.Atoi(resp.Header.Get("Content-Length"))
				if err != nil {
					return err
				}

				err = CopyWithProgress(dst.Basename, int64(totalSize), w, resp.Body)
			} else {
				err = TransientIoCopy(w, resp.Body)
			}
		}

		return err
	})

	if err == nil && shouldCache {
		LogDebug(LogDownload, "cache store in '%v'", cacheFile)
		if err := UFS.Copy(dst, cacheFile); err != nil {
			LogWarning(LogDownload, "failed to cache download with %v", err)
		}
	}

	return err
}

var re_metaRefreshRedirect = regexp.MustCompile(`(?i)<meta.*http-equiv="refresh".*content=".*url=(.*)".*?/>`)

func DownloadHttpRedirect(dst Filename, src url.URL) error {
	LogVerbose(LogDownload, "download http redirect '%v' to '%v'...", src.String(), dst.String())

	client := http.Client{
		CheckRedirect: func(r *http.Request, _ []*http.Request) error {
			r.URL.Opaque = r.URL.Path
			return nil
		},
	}
	defer client.CloseIdleConnections()

	resp, err := client.Get(src.String())
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	parse := TransientBuffer.Allocate()
	defer TransientBuffer.Release(parse)

	err = TransientIoCopy(parse, resp.Body)

	if err == nil {
		match := re_metaRefreshRedirect.FindSubmatch(parse.Bytes())
		if len(match) > 1 {
			var url *url.URL
			if url, err = url.Parse(UnsafeStringFromBytes(match[1])); err == nil {
				return DownloadFile(dst, *url)
			}
		} else {
			err = fmt.Errorf("http: could not find html refresh meta")
		}
	}

	return err
}
