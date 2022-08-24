package utils

import (
	"bytes"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"regexp"
	"strconv"
)

type DownloadMode int32

const (
	DOWNLOAD_DEFAULT DownloadMode = iota
	DOWNLOAD_REDIRECT
)

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

func NewDownloader(uri string, dst Filename, mode DownloadMode) *Downloader {
	parsedUrl, err := url.Parse(uri)
	if err != nil {
		LogFatal("download: %v", err)
	}
	result := &Downloader{
		Source:      *parsedUrl,
		Destination: dst,
		Mode:        mode,
	}
	return CommandEnv.BuildGraph().
		Create(result).
		GetBuildable().(*Downloader)
}

type Downloader struct {
	Source      url.URL
	Destination Filename
	Mode        DownloadMode
}

func (dl *Downloader) Alias() BuildAlias {
	return MakeBuildAlias("Download", dl.Destination.String())
}
func (dl *Downloader) Build(bc BuildContext) (BuildStamp, error) {
	var err error
	switch dl.Mode {
	case DOWNLOAD_DEFAULT:
		err = DownloadFile(dl.Destination, dl.Source)
	case DOWNLOAD_REDIRECT:
		err = DownloadHttpRedirect(dl.Destination, dl.Source)
	}

	if err == nil {
		bc.OutputFile(dl.Destination)
		return MakeBuildStamp(dl)
	} else {
		return BuildStamp{}, err
	}
}
func (dl *Downloader) GetDigestable(o *bytes.Buffer) {
	o.WriteString(dl.Source.String())
	dl.Destination.GetDigestable(o)
	o.WriteString(dl.Mode.String())
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

func downloadFromCachedArtifcats(resp *http.Response) (Filename, downloadCacheResult) {
	var contentHash []string
	if contentHash = resp.Header.Values("Content-Md5"); contentHash == nil {
		contentHash = resp.Header.Values("X-Goog-Hash")
	}
	if contentHash != nil {
		digest := MakeDigester()
		for _, x := range contentHash {
			digest.Write([]byte(x))
		}

		uid := digest.Finalize()
		inCache := UFS.Transient.Folder("DownloadCache").File(fmt.Sprintf("%X.bin", uid.Slice()))
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
	LogVerbose("http: downloading url '%v' to '%v'...", src.String(), dst.String())

	cacheFile, shouldCache := Filename{}, false
	err := UFS.SafeCreate(dst, func(w io.Writer) error {
		client := http.Client{
			CheckRedirect: func(r *http.Request, _ []*http.Request) error {
				r.URL.Opaque = r.URL.Path
				return nil
			},
		}

		resp, err := client.Get(src.String())
		if err != nil {
			return err
		}
		defer resp.Body.Close()

		var cacheResult downloadCacheResult
		cacheFile, cacheResult = downloadFromCachedArtifcats(resp)
		if cacheResult == nil { // cache hit
			LogDebug("http: cache hit on '%v'", cacheFile)
			return UFS.Open(cacheFile, func(r io.Reader) error {
				_, err := io.Copy(w, r)
				return err
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
				_, err = io.Copy(w, resp.Body)
			}
		}

		return err
	})

	if err == nil && shouldCache {
		LogDebug("http: cache store in '%v'", cacheFile)
		if err := UFS.Copy(dst, cacheFile); err != nil {
			LogWarning("http: failed to cache download with %v", err)
		}
	}

	return err
}

var re_metaRefreshRedirect = regexp.MustCompile(`(?i)<meta.*http-equiv="refresh".*content=".*url=(.*)".*?/>`)

func DownloadHttpRedirect(dst Filename, src url.URL) error {
	LogVerbose("http: download http redirect '%v' to '%v'...", src.String(), dst.String())

	client := http.Client{
		CheckRedirect: func(r *http.Request, _ []*http.Request) error {
			r.URL.Opaque = r.URL.Path
			return nil
		},
	}

	resp, err := client.Get(src.String())
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	parse := bytes.Buffer{}
	_, err = io.Copy(&parse, resp.Body)

	if err == nil {
		match := re_metaRefreshRedirect.FindSubmatch(parse.Bytes())
		if len(match) > 1 {
			var url *url.URL
			if url, err = url.Parse(string(match[1])); err == nil {
				return DownloadFile(dst, *url)
			}
		} else {
			err = fmt.Errorf("http: could not find html refresh meta")
		}
	}

	return err
}
