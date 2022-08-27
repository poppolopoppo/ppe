package utils

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"io"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
	"sync"
	"time"
)

/***************************************
 * Directory
 ***************************************/

var re_pathSeparator = regexp.MustCompile(`[\\\/]+`)

func SplitPath(in string) []string {
	return re_pathSeparator.Split(in, -1)
}

type Directory []string

func MakeDirectory(str string) Directory {
	return SplitPath(filepath.Clean(str))
}
func (d Directory) Basename() string {
	if len(d) > 0 {
		return d[len(d)-1]
	} else {
		return ""
	}
}
func (d Directory) Parent() Directory {
	return d[:len(d)-1]
}
func (d Directory) Folder(name ...string) Directory {
	return append(append([]string(nil), d...), name...)
}
func (d Directory) File(name ...string) Filename {
	return Filename{Dirname: append(d, name[:len(name)-1]...), Basename: name[len(name)-1]}
}
func (d Directory) AbsoluteFolder(rel ...string) (result Directory) {
	for _, x := range rel {
		result = append(d, SplitPath(x)...)
	}
	return result
}
func (d Directory) AbsoluteFile(rel ...string) (result Filename) {
	return MakeFilename(filepath.Join(append([]string{d.String()}, rel...)...))
}
func (d Directory) Relative(to Directory) string {
	if path, err := filepath.Rel(to.String(), d.String()); err == nil {
		return path
	} else {
		panic(err)
	}
}
func (d Directory) Normalize() (result Directory) {
	return MakeDirectory(filepath.Clean(d.String()))
}
func (d Directory) Equals(o Directory) bool {
	if len(d) != len(o) {
		return false
	}
	for i, x := range d {
		if x != o[i] {
			return false
		}
	}
	return true
}
func (d Directory) Compare(o Directory) int {
	n := len(o)
	if len(d) < len(o) {
		n = len(d)
	}
	for i := 0; i < n; i += 1 {
		if c := strings.Compare(d[i], o[i]); c != 0 {
			return c
		}
	}
	if len(d) == len(o) {
		return 0
	} else if len(d) < len(o) {
		return -1
	} else {
		return 1
	}
}
func (d Directory) GetDigestable(o *bytes.Buffer) {
	o.WriteByte(byte(len(d)))
	for _, x := range d {
		o.WriteString(x)
	}
}
func (d Directory) String() string {
	sb := strings.Builder{}
	for i, x := range d {
		if i > 0 {
			sb.WriteString("/")
		}
		sb.WriteString(x)
	}
	return sb.String()
}

/***************************************
 * Filename
 ***************************************/

type Filename struct {
	Dirname  Directory
	Basename string
}

func MakeFilename(str string) Filename {
	var x string
	str = filepath.Clean(str)
	str, x = filepath.Split(str)
	return Filename{
		Basename: x,
		Dirname:  MakeDirectory(str),
	}
}

func (f Filename) Ext() string {
	return path.Ext(f.Basename)
}
func (f Filename) TrimExt() string {
	return strings.TrimSuffix(f.Basename, f.Ext())
}
func (f Filename) ReplaceExt(ext string) Filename {
	return Filename{
		Basename: f.TrimExt() + ext,
		Dirname:  f.Dirname,
	}
}
func (f Filename) Relative(to Directory) string {
	if path, err := filepath.Rel(to.String(), f.String()); err == nil {
		return path
	} else {
		panic(err)
	}
}
func (f Filename) Normalize() (result Filename) {
	result = f
	result.Dirname = result.Dirname.Normalize()
	return result
}
func (f Filename) Equals(o Filename) bool {
	return (f.Basename == o.Basename && f.Dirname.Equals(o.Dirname))
}
func (f Filename) Compare(o Filename) int {
	if c := f.Dirname.Compare(o.Dirname); c != 0 {
		return c
	} else {
		return strings.Compare(f.Basename, o.Basename)
	}
}
func (f Filename) GetDigestable(o *bytes.Buffer) {
	f.Dirname.GetDigestable(o)
	o.WriteString(f.Basename)
}
func (f Filename) String() string {
	if len(f.Dirname) > 0 {
		return f.Dirname.String() + "/" + f.Basename
	} else {
		return f.Basename
	}
}

/***************************************
 * fmt.Value interface
 ***************************************/

func (d *Directory) Set(str string) error {
	if str != "" {
		if !filepath.IsAbs(str) {
			str = filepath.Join(UFS.Working.String(), str)
		}
		*d = MakeDirectory(str)
	} else {
		*d = Directory{}
	}
	return nil
}

func (f *Filename) Set(str string) error {
	if str != "" {
		if !filepath.IsAbs(str) {
			str = filepath.Join(UFS.Working.String(), str)
		}
		*f = MakeFilename(str)
	} else {
		*f = Filename{}
	}
	return nil
}

/***************************************
 * Entity info cache
 ***************************************/

type FileInfo struct {
	AbsolutePath string
	os.FileInfo
}
type DirectoryInfo struct {
	AbsolutePath string
	Files        []Filename
	Directories  []Directory
	os.FileInfo

	barrier sync.Mutex
}

type UFSCacheBin struct {
	barrier        sync.RWMutex
	FileCache      map[string]*FileInfo
	DirectoryCache map[string]*DirectoryInfo
}

type UFSCache struct {
	bins [256]UFSCacheBin
}

func newUFSCache() *UFSCache {
	result := &UFSCache{}
	for i := range result.bins {
		result.bins[i] = UFSCacheBin{
			barrier:        sync.RWMutex{},
			FileCache:      map[string]*FileInfo{},
			DirectoryCache: map[string]*DirectoryInfo{},
		}
	}
	return result
}
func (cache *UFSCache) getBin(x Digestable) *UFSCacheBin {
	h := MakeDigest(x)
	return &cache.bins[h[0]]
}

var ufsCache = newUFSCache()

func invalidate_file_info(f Filename) {
	cacheBin := ufsCache.getBin(f)
	cacheBin.barrier.Lock()
	defer cacheBin.barrier.Unlock()
	delete(cacheBin.FileCache, f.String())
}
func make_file_info(f Filename, optionalStat *os.FileInfo) (*FileInfo, error) {
	path := f.String()

	cacheBin := ufsCache.getBin(f)
	cacheBin.barrier.RLock()
	if cached, ok := cacheBin.FileCache[path]; ok {
		cacheBin.barrier.RUnlock()
		return cached, nil
	}
	cacheBin.barrier.RUnlock()

	cacheBin.barrier.Lock()
	defer cacheBin.barrier.Unlock()

	if cached, ok := cacheBin.FileCache[path]; !ok {
		cached = nil
		var err error
		var stat os.FileInfo
		if optionalStat != nil {
			cached = &FileInfo{
				AbsolutePath: path,
				FileInfo:     *optionalStat,
			}
		} else if stat, err = os.Stat(f.String()); !os.IsNotExist(err) {
			if !stat.IsDir() && stat.Mode().Type().IsRegular() {
				cached = &FileInfo{
					AbsolutePath: path,
					FileInfo:     stat,
				}
			} else {
				err = errors.New("path does not point to a file")
			}
		}
		cacheBin.FileCache[path] = cached
		return cached, err
	} else {
		return cached, nil
	}
}

func invalidate_directory_info(d Directory) {
	cacheBin := ufsCache.getBin(d)
	cacheBin.barrier.Lock()
	defer cacheBin.barrier.Unlock()
	delete(cacheBin.DirectoryCache, d.String())
}
func make_directory_info(d Directory, optionalStat *os.FileInfo) (*DirectoryInfo, error) {
	path := d.String()

	cacheBin := ufsCache.getBin(d)
	cacheBin.barrier.RLock()
	if cached, ok := cacheBin.DirectoryCache[path]; ok {
		cacheBin.barrier.RUnlock()
		return cached, nil
	}
	cacheBin.barrier.RUnlock()

	cacheBin.barrier.Lock()
	defer cacheBin.barrier.Unlock()

	if cached, ok := cacheBin.DirectoryCache[path]; !ok {
		var stat os.FileInfo
		var cached *DirectoryInfo
		var err error
		if optionalStat != nil {
			cached = &DirectoryInfo{
				AbsolutePath: path,
				FileInfo:     *optionalStat,
				Files:        nil,
				Directories:  nil,
				barrier:      sync.Mutex{},
			}
		} else if stat, err = os.Stat(d.String()); !os.IsNotExist(err) {
			if stat.IsDir() {
				cached = &DirectoryInfo{
					AbsolutePath: path,
					FileInfo:     stat,
					Files:        nil,
					Directories:  nil,
					barrier:      sync.Mutex{},
				}
			} else {
				err = errors.New("path does not point to a directory")
			}
		}
		cacheBin.DirectoryCache[path] = cached
		return cached, err
	} else {
		return cached, nil
	}
}

func enumerate_directory(d Directory) (*DirectoryInfo, error) {
	if info, err := d.Info(); err == nil && info != nil {
		info.barrier.Lock() // lock the directory, not all cache
		defer info.barrier.Unlock()

		if info.Files == nil || info.Directories == nil {
			var entries []os.DirEntry
			entries, err = os.ReadDir(info.AbsolutePath)

			if err == nil {
				files := []Filename{}
				directories := []Directory{}

				for _, it := range entries {
					var stat os.FileInfo
					stat, err = it.Info()
					if err != nil {
						continue
					} else if stat.IsDir() {
						child := d.Folder(it.Name())
						make_directory_info(child, &stat)
						directories = append(directories, child)
					} else if it.Type().IsRegular() {
						child := d.File(it.Name())
						make_file_info(child, &stat)
						files = append(files, child)
					}
				}

				info.Files = files
				info.Directories = directories
			}
		}
		return info, err
	} else {
		return nil, err
	}
}

/***************************************
 * IO
 ***************************************/

func (f Filename) Info() (*FileInfo, error) {
	return make_file_info(f, nil)
}
func (f Filename) Invalidate() {
	invalidate_file_info(f)
}
func (f Filename) Exists() bool {
	info, err := f.Info()
	return (err == nil && info != nil)
}

func (d Directory) Info() (*DirectoryInfo, error) {
	return make_directory_info(d, nil)
}
func (d Directory) Invalidate() {
	invalidate_directory_info(d)
}
func (d Directory) Exists() bool {
	info, err := d.Info()
	return (err == nil && info != nil)
}
func (d Directory) Files() []Filename {
	if info, err := enumerate_directory(d); err == nil {
		return info.Files
	} else {
		LogError("Directory.Files(): %v", err)
		return []Filename{}
	}
}
func (d Directory) Directories() []Directory {
	if info, err := enumerate_directory(d); err == nil {
		return info.Directories
	} else {
		LogError("Directory.Directories(): %v", err)
		return []Directory{}
	}
}
func (d Directory) MatchDirectories(each func(Directory) error, r *regexp.Regexp) error {
	if r == nil {
		return nil
	}
	LogVeryVerbose("match directories in '%v' for /%v/...", d, r)
	if info, err := enumerate_directory(d); err == nil {
		for _, s := range info.Directories {
			if r.MatchString(s[len(s)-1]) {
				if err := each(s); err != nil {
					return err
				}
			}
		}
		return nil
	} else {
		return err
	}
}
func (d Directory) MatchFiles(each func(Filename) error, r *regexp.Regexp) error {
	if r == nil {
		return nil
	}
	LogVeryVerbose("match files in '%v' for /%v/...", d, r)
	if info, err := enumerate_directory(d); err == nil {
		for _, f := range info.Files {
			if r.MatchString(f.Basename) {
				if err := each(f); err != nil {
					return err
				}
			}
		}
		return nil
	} else {
		return err
	}
}
func (d Directory) MatchFilesRec(each func(Filename) error, r *regexp.Regexp) error {
	if r == nil {
		return nil
	}
	LogVeryVerbose("match files rec in '%v' for /%v/...", d, r)
	if info, err := enumerate_directory(d); err == nil {
		for _, f := range info.Files {
			if r.MatchString(f.Basename) {
				if err := each(f); err != nil {
					return err
				}
			}
		}
		for _, s := range info.Directories {
			if err := s.MatchFilesRec(each, r); err != nil {
				return err
			}
		}
		return nil
	} else {
		return err
	}
}
func (d Directory) FindFileRec(r *regexp.Regexp) (Filename, error) {
	LogVeryVerbose("find file rec in '%v' for /%v/...", d, r)
	if info, err := enumerate_directory(d); err == nil {
		for _, f := range info.Files {
			if r.MatchString(f.Basename) {
				return f, nil
			}
		}
		for _, s := range info.Directories {
			if f, err := s.FindFileRec(r); err == nil {
				return f, nil
			}
		}
		return Filename{}, fmt.Errorf("file not found '%v' in '%v'", r, d)
	} else {
		return Filename{}, err
	}
}

/***************************************
 * Containers
 ***************************************/

type DirSet []Directory

func NewDirSet(x ...Directory) DirSet {
	return x
}

func (list DirSet) Len() int           { return len(list) }
func (list DirSet) Less(i, j int) bool { return list[i].Compare(list[j]) < 0 }
func (list DirSet) Swap(i, j int)      { list[i], list[j] = list[j], list[i] }

func (list *DirSet) Contains(it ...Directory) bool {
	for _, x := range it {
		if _, ok := IndexIf(x.Equals, (*list)...); !ok {
			return false
		}
	}
	return true
}
func (list *DirSet) Append(it ...Directory) {
	*list = AppendEquatable_CheckUniq(*list, it...)
}
func (list *DirSet) AppendUniq(it ...Directory) {
	for _, x := range it {
		if !list.Contains(x) {
			list.Append(x)
		}
	}
}
func (list *DirSet) Prepend(it ...Directory) {
	*list = PrependEquatable_CheckUniq(it, *list...)
}
func (list *DirSet) Remove(it ...Directory) {
	for _, x := range it {
		*list = RemoveUnless(x.Equals, *list...)
	}
}
func (list *DirSet) Clear() {
	*list = []Directory{}
}
func (list DirSet) Concat(it ...Directory) (result DirSet) {
	result = make(DirSet, len(list)+len(it))
	copy(result, list)
	copy(result[len(list):], it)
	return result
}
func (list DirSet) GetDigestable(o *bytes.Buffer) {
	for _, x := range list {
		x.GetDigestable(o)
	}
}
func (list DirSet) StringSet() StringSet {
	return MakeStringerSet(list...)
}
func (list DirSet) Join(delim string) string {
	return JoinString(delim, list...)
}

type FileSet []Filename

func NewFileSet(x ...Filename) FileSet {
	return x
}

func (list FileSet) Len() int           { return len(list) }
func (list FileSet) Less(i, j int) bool { return list[i].Compare(list[j]) < 0 }
func (list FileSet) Swap(i, j int)      { list[i], list[j] = list[j], list[i] }

func (list *FileSet) Contains(it ...Filename) bool {
	for _, x := range it {
		if _, ok := IndexIf(x.Equals, (*list)...); !ok {
			return false
		}
	}
	return true
}
func (list *FileSet) Append(it ...Filename) {
	*list = AppendEquatable_CheckUniq(*list, it...)
}
func (list *FileSet) AppendUniq(it ...Filename) {
	for _, x := range it {
		if !list.Contains(x) {
			list.Append(x)
		}
	}
}
func (list *FileSet) Prepend(it ...Filename) {
	*list = PrependEquatable_CheckUniq(it, *list...)
}
func (list *FileSet) Remove(it ...Filename) {
	for _, x := range it {
		*list = RemoveUnless(x.Equals, *list...)
	}
}
func (list *FileSet) Clear() {
	*list = []Filename{}
}
func (list FileSet) Concat(it ...Filename) (result FileSet) {
	result = make(FileSet, len(list)+len(it))
	copy(result, list)
	copy(result[len(list):], it)
	return result
}
func (list FileSet) ConcatUniq(it ...Filename) (result FileSet) {
	result = NewFileSet(list...)
	for _, x := range it {
		result.AppendUniq(x)
	}
	return result
}
func (list FileSet) TotalSize() (result int64) {
	for _, x := range list {
		if info, err := x.Info(); info != nil {
			result += info.Size()
		} else {
			LogError("%v: %v", x, err)
		}
	}
	// LogDebug("total size of %d files: %.3f KiB", len(list), float32(result)/1024.0)
	return result
}
func (list FileSet) GetDigestable(o *bytes.Buffer) {
	for _, x := range list {
		x.GetDigestable(o)
	}
}
func (list FileSet) StringSet() StringSet {
	return MakeStringerSet(list...)
}
func (list FileSet) Join(delim string) string {
	return JoinString(delim, list...)
}

/***************************************
 * JSON: marshal as string instead of array
 ***************************************/

func (f Filename) MarshalJSON() ([]byte, error) {
	return MarshalJSON(f)
}
func (f *Filename) UnmarshalJSON(data []byte) error {
	return UnmarshalJSON(f, data)
}

func (d Directory) MarshalJSON() ([]byte, error) {
	return MarshalJSON(d)
}
func (d *Directory) UnmarshalJSON(data []byte) error {
	return UnmarshalJSON(d, data)
}

/***************************************
 * Frontend
 ***************************************/

var UFS UFSFrontEnd = make_ufs_frontend()

type UFSFrontEnd struct {
	Working Directory

	Root   Directory
	Build  Directory
	Extras Directory
	Source Directory
	Output Directory

	Binaries     Directory
	Cache        Directory
	Generated    Directory
	Intermediate Directory
	Projects     Directory
	Saved        Directory
	Transient    Directory
}

func (ufs *UFSFrontEnd) File(str string) Filename {
	return MakeFilename(str)
}
func (ufs *UFSFrontEnd) Dir(str string) Directory {
	return MakeDirectory(str)
}
func (ufs *UFSFrontEnd) Touch(dst Filename) error {
	LogDebug("ufs: touch %v", dst)
	path := dst.String()
	currentTime := time.Now().Local()
	if err := os.Chtimes(path, currentTime, currentTime); err == nil {
		invalidate_file_info(dst)
		return nil
	} else {
		return err
	}
}
func (ufs *UFSFrontEnd) Mkdir(dst Directory) {
	path := dst.String()
	if st, err := os.Stat(path); st != nil && err != nil {
		if os.IsExist(err) {
			if !st.IsDir() {
				LogDebug("ufs: mkdir %v", dst)
				LogPanic("ufs: '%v' already exist, but is not a directory", dst)
			}
		}
	} else {
		LogDebug("ufs: mkdir %v", dst)
		invalidate_directory_info(dst)
		if err := os.MkdirAll(path, os.ModePerm); err != nil {
			LogPanic("%v: '%v'", dst, err)
		}
	}
}
func (ufs *UFSFrontEnd) CreateWriter(dst Filename) (*os.File, error) {
	invalidate_file_info(dst)
	ufs.Mkdir(dst.Dirname)
	LogDebug("ufs: create '%v'", dst)
	return os.Create(dst.String())
}
func (ufs *UFSFrontEnd) Create(dst Filename, write func(io.Writer) error) error {
	outp, err := ufs.CreateWriter(dst)
	if outp != nil {
		defer outp.Close()
	}
	if err == nil {
		if err = write(outp); err == nil {
			return nil
		}
	}
	LogWarning("ufs: %v", err)
	return err
}
func (ufs *UFSFrontEnd) SafeCreate(dst Filename, write func(io.Writer) error) error {
	os.Remove(dst.String())
	ufs.Mkdir(dst.Dirname)

	file, err := os.CreateTemp(
		dst.Dirname.Relative(UFS.Root),
		dst.ReplaceExt("-*"+dst.Ext()).Relative(dst.Dirname))
	if err != nil {
		return err
	}

	buf := bufio.NewWriter(file)
	if err = write(buf); err == nil {
		if err = buf.Flush(); err == nil {
			if err = file.Close(); err == nil {
				//LogVeryVerbose("moving temporary file '%v' to final destination '%v'", file.Name(), dst)
				invalidate_file_info(dst)
				return os.Rename(file.Name(), dst.String())
			}
		}
	}

	defer os.Remove(file.Name())
	file.Close()
	LogWarning("UFS.SafeCreate: %v", err)
	return err
}
func (ufs *UFSFrontEnd) LazyCreate(dst Filename, write func(io.Writer) error) error {
	if org, err := dst.Info(); err == nil {
		tmp := bytes.Buffer{}
		write(&tmp)

		new := tmp.Bytes()

		if org.Size() == int64(tmp.Len()) {
			async := AsyncFileDigest(dst)

			digester := MakeDigester()
			if err := DigestReader(digester, bytes.NewReader(new)); err == nil {
				current := digester.Finalize()
				if previous := async.Join(); previous.Failure() == nil {
					if previous.Success() == current {
						LogVerbose("ufs: skip update of '%v', content didn't change", dst)
						return nil // we finally can skip writing
					} else {
						LogVerbose("ufs: keep update of '%v', content DID change\n\told: %v\n\tnew: %v", dst, previous.Success(), current)
					}
				}
			}
		}

		return ufs.SafeCreate(dst, func(w io.Writer) error {
			w.Write(new) // reuse already bytes already dumped in memory
			return nil
		})
	}
	return ufs.SafeCreate(dst, write)
}
func (ufs *UFSFrontEnd) Open(src Filename, read func(io.Reader) error) error {
	input, err := os.Open(src.String())
	LogDebug("ufs: open '%v'", src)

	if input != nil {
		defer input.Close()
	}

	if err == nil {
		if err = read(input); err == nil {
			return nil
		}
	}

	LogWarning("UFS.Open: %v", err)
	return err
}
func (ufs *UFSFrontEnd) Scan(src Filename, re *regexp.Regexp, match func([]string) error) error {
	return ufs.Open(src, func(rd io.Reader) error {
		LogDebug("ufs: scan '%v' with regexp %v", src, re)

		const capacity = TRANSIENT_BYTES_CAPACITY / 2
		buf := TransientBytes.Allocate()
		defer TransientBytes.Release(buf)

		scanner := bufio.NewScanner(rd)
		scanner.Buffer(buf, capacity)
		scanner.Split(SplitRegex(re, capacity))

		for scanner.Scan() {
			if err := scanner.Err(); err == nil {
				txt := scanner.Text()
				m := re.FindStringSubmatch(txt)
				if err := match(m[1:]); err != nil {
					return err
				}
			} else {
				return err
			}
		}
		return nil
	})
}
func (ufs *UFSFrontEnd) Rename(src, dst Filename) error {
	ufs.Mkdir(dst.Dirname)
	invalidate_file_info(src)
	invalidate_file_info(dst)
	LogDebug("ufs: rename file '%v' to '%v'", src, dst)
	return os.Rename(src.String(), dst.String())
}
func (ufs *UFSFrontEnd) Copy(src, dst Filename) error {
	ufs.Mkdir(dst.Dirname)
	invalidate_file_info(dst)
	LogDebug("ufs: copy file '%v' to '%v'", src, dst)
	return ufs.Open(src, func(r io.Reader) error {
		info, err := src.Info()
		if err != nil {
			return err
		}
		return ufs.SafeCreate(dst, func(w io.Writer) error {
			return CopyWithProgress(dst.Basename, info.Size(), w, r)
		})
	})
}

func make_ufs_frontend() (ufs UFSFrontEnd) {
	_, caller, _, ok := runtime.Caller(1)
	if !ok {
		LogPanic("no caller information")
	}

	caller = path.Dir(caller)
	var x string
	if caller, x = filepath.Split(caller); x != "utils" {
		UnreachableCode()
	}
	caller = caller[:len(caller)-1]
	if caller, x = filepath.Split(caller); x != "Build" {
		UnreachableCode()
	}

	if wd, err := os.Getwd(); ok {
		ufs.Working = MakeDirectory(wd)
	} else {
		LogPanic("working directory: %v", err)
	}

	ufs.Root = ufs.Dir(caller)

	ufs.Build = ufs.Root.Folder("Build")
	ufs.Extras = ufs.Root.Folder("Extras")
	ufs.Source = ufs.Root.Folder("Source")
	ufs.Output = ufs.Root.Folder("Output")

	ufs.Binaries = ufs.Output.Folder("Binaries")
	ufs.Cache = ufs.Output.Folder("Cache")
	ufs.Generated = ufs.Output.Folder("Generated")
	ufs.Intermediate = ufs.Output.Folder("Intermediate")
	ufs.Projects = ufs.Output.Folder("Projects")
	ufs.Transient = ufs.Output.Folder("Transient")
	ufs.Saved = ufs.Output.Folder("Saved")

	return ufs
}

func MakeGlobRegexp(glob ...string) *regexp.Regexp {
	if len(glob) == 0 {
		return nil
	}
	expr := "(?i)^("
	for i, x := range glob {
		x = regexp.QuoteMeta(x)
		x = strings.ReplaceAll(x, "\\?", ".")
		x = strings.ReplaceAll(x, "\\*", ".*?")
		x = strings.ReplaceAll(x, "/", "[\\\\/]")
		x = "(" + x + ")"
		if i == 0 {
			expr += x
		} else {
			expr += "|" + x
		}
	}
	return regexp.MustCompile(expr + ")$")
}
