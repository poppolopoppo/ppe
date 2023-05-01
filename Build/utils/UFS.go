package utils

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"os"
	"path"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strings"
	"sync"
	"time"

	"github.com/djherbis/times"
)

/***************************************
 * Path to string
 ***************************************/

const OSPathSeparator = os.PathSeparator

func BuildSanitizedPath(sb *strings.Builder, pathname string, sep rune) error {
	hasSeparator := false
	for _, ch := range pathname {
		if os.IsPathSeparator((uint8)(ch)) {
			if !hasSeparator {
				hasSeparator = true
				if _, err := sb.WriteRune(sep); err != nil {
					return err
				}
			}
		} else if _, err := sb.WriteRune(ch); err != nil {
			return err
		} else {
			hasSeparator = false
		}
	}

	return nil
}
func SanitizePath(pathname string, sep rune) string {
	sb := strings.Builder{}
	sb.Grow(len(pathname))
	BuildSanitizedPath(&sb, pathname, sep)
	return sb.String()
}
func SplitPath(in string) (results []string) {
	first := 0
	for i, ch := range in {
		if os.IsPathSeparator((uint8)(ch)) {
			if i > first {
				results = append(results, in[first:i])
			}
			first = i + 1
		}
	}
	if first < len(in) {
		results = append(results, in[first:])
	}
	return
}

var localPathEnabled = true

func MakeLocalDirectory(d Directory) (relative string) {
	if localPathEnabled {
		return d.Relative(UFS.Root)
	} else {
		return d.String()
	}
}
func MakeLocalFilename(f Filename) (relative string) {
	if localPathEnabled {
		return f.Relative(UFS.Root)
	} else {
		return f.String()
	}
}

var cleanPathCache SharedMapT[string, string]

func CleanPath(in string) string {
	AssertMessage(func() bool { return filepath.IsAbs(in) }, "ufs: need absolute path -> %q", in)

	// Those checks are cheap compared to the followings
	in = filepath.Clean(in)
	if cleaned, err := filepath.Abs(in); err == nil {
		in = cleaned
	} else {
		LogPanicErr(err)
	}

	// /!\ EvalSymlinks() is **SUPER** expansive !
	// Try to mitigate with an ad-hoc concurrent cache
	if cleaned, ok := cleanPathCache.Get(in); ok {
		return cleaned // cache-hit: already processed
	}

	result, err := filepath.EvalSymlinks(in)
	if err != nil {
		result = in
		err = nil // if path does not exist yet
	}

	// Store cleaned path for future occurrences (expects many for directories)
	cleanPathCache.Add(in, result)
	return result
}

/***************************************
 * Directory
 ***************************************/

type Directory []string

func MakeDirectory(str string) Directory {
	return SplitPath(CleanPath(str))
}
func (d Directory) Valid() bool {
	return len(d) > 0
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
func (d Directory) IsIn(o Directory) bool {
	return o.IsParentOf(d)
}
func (d Directory) IsParentOf(o Directory) bool {
	n := len(d)
	if n > len(o) {
		return false
	}
	for i := range d[:n] {
		if d[i] != o[i] {
			return false
		}
	}
	return true
}
func (d Directory) AbsoluteFolder(rel ...string) (result Directory) {
	result = append([]string(nil), d...)
	for _, x := range rel {
		path := SplitPath(x)
		result = append(result, path...)
	}
	return
}
func (d Directory) AbsoluteFile(rel ...string) (result Filename) {
	result.Dirname = d.AbsoluteFolder(rel...)
	result.Basename = result.Dirname.Basename()
	result.Dirname = result.Dirname.Parent()
	return
}
func (d Directory) Relative(to Directory) string {
	if path, err := filepath.Rel(to.String(), d.String()); err == nil {
		return path
	} else {
		return d.String()
	}
}
func (d Directory) Normalize() (result Directory) {
	return MakeDirectory(d.String())
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
func (d Directory) String() string {
	sb := strings.Builder{}
	if err := d.StringBuilder(&sb); err != nil {
		LogPanicErr(err)
	}
	return sb.String()
}
func (d Directory) stringBuilderCapacity() (capacity int) {
	for i, x := range d {
		if i > 0 {
			capacity += 1
		}
		capacity += len(x)
	}
	return
}
func (d Directory) StringBuilder(sb *strings.Builder) error {
	sb.Grow(d.stringBuilderCapacity())

	for i, x := range d {
		if i > 0 {
			if _, err := sb.WriteRune(OSPathSeparator); err != nil {
				return err
			}
		}
		if _, err := sb.WriteString(x); err != nil {
			return err
		}
	}
	return nil
}

/***************************************
 * Filename
 ***************************************/

type Filename struct {
	Dirname  Directory
	Basename string
}

func MakeFilename(str string) Filename {
	dirname, basename := filepath.Split(str)
	return Filename{
		Basename: basename,
		Dirname:  MakeDirectory(dirname),
	}
}

func (f Filename) Valid() bool {
	return len(f.Basename) > 0
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
	if path, err := filepath.Rel(to.String(), f.Dirname.String()); err == nil {
		return filepath.Join(path, f.Basename)
	} else {
		return f.String()
	}
}
func (f Filename) Normalize() (result Filename) {
	return MakeFilename(f.String())
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
func (f Filename) String() string {
	if len(f.Dirname) > 0 {
		sb := strings.Builder{}
		sb.Grow(f.Dirname.stringBuilderCapacity() + 1 + len(f.Basename))
		if err := f.Dirname.StringBuilder(&sb); err != nil {
			LogPanicErr(err)
		}
		if _, err := sb.WriteRune(OSPathSeparator); err != nil {
			LogPanicErr(err)
		}
		if _, err := sb.WriteString(f.Basename); err != nil {
			LogPanicErr(err)
		}
		return sb.String()
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

	once sync.Once
}

func GetAccessTime(stat os.FileInfo) time.Time {
	return times.Get(stat).AccessTime()
}
func GetCreationTime(stat os.FileInfo) time.Time {
	return times.Get(stat).BirthTime()
}
func GetModificationTime(stat os.FileInfo) time.Time {
	return times.Get(stat).ModTime()
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
func (cache *UFSCache) getBin(x Serializable) *UFSCacheBin {
	h := SerializeFingerpint(x, Fingerprint{})
	return &cache.bins[h[0]]
}

var ufsCache = newUFSCache()

func invalidate_file_info(f Filename) {
	cacheBin := ufsCache.getBin(&f)
	cacheBin.barrier.Lock()
	defer cacheBin.barrier.Unlock()
	delete(cacheBin.FileCache, f.String())
}
func make_file_info(f Filename, optionalStat *os.FileInfo) (*FileInfo, error) {
	path := f.String() //.Normalize().String()

	cacheBin := ufsCache.getBin(&f)
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
	cacheBin := ufsCache.getBin(&d)
	cacheBin.barrier.Lock()
	defer cacheBin.barrier.Unlock()
	delete(cacheBin.DirectoryCache, d.String())
}
func make_directory_info(d Directory, optionalStat *os.FileInfo) (*DirectoryInfo, error) {
	path := d.String() //.Normalize().String()

	cacheBin := ufsCache.getBin(&d)
	cacheBin.barrier.RLock()
	if cached, ok := cacheBin.DirectoryCache[path]; ok && cached != nil {
		cacheBin.barrier.RUnlock()
		return cached, nil
	}
	cacheBin.barrier.RUnlock()

	cacheBin.barrier.Lock()
	defer cacheBin.barrier.Unlock()

	if cached, ok := cacheBin.DirectoryCache[path]; !ok || cached == nil {
		var stat os.FileInfo
		var cached *DirectoryInfo
		var err error
		if optionalStat != nil {
			cached = &DirectoryInfo{
				AbsolutePath: path,
				FileInfo:     *optionalStat,
				Files:        nil,
				Directories:  nil,
			}
		} else if stat, err = os.Stat(d.String()); !os.IsNotExist(err) {
			if stat.IsDir() {
				cached = &DirectoryInfo{
					AbsolutePath: path,
					FileInfo:     stat,
					Files:        nil,
					Directories:  nil,
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
		info.once.Do(func() {
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
		})
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
 * DirSet
 ***************************************/

type DirSet []Directory

func NewDirSet(x ...Directory) DirSet {
	return x
}

func (list DirSet) Len() int           { return len(list) }
func (list DirSet) Less(i, j int) bool { return list[i].Compare(list[j]) < 0 }
func (list DirSet) Slice() []Directory { return list }
func (list DirSet) Swap(i, j int)      { list[i], list[j] = list[j], list[i] }

func (list *DirSet) Sort() {
	sort.Slice(list.Slice(), func(i, j int) bool {
		return (*list)[i].Compare((*list)[j]) < 0
	})
}
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
func (list DirSet) ConcatUniq(it ...Directory) (result DirSet) {
	result = NewDirSet(list...)
	for _, x := range it {
		result.AppendUniq(x)
	}
	return result
}
func (list *DirSet) Serialize(ar Archive) {
	SerializeSlice(ar, (*[]Directory)(list))
}
func (list DirSet) Equals(other DirSet) bool {
	if len(list) != len(other) {
		return false
	}
	for i, it := range list {
		if !other[i].Equals(it) {
			return false
		}
	}
	return true
}
func (list DirSet) StringSet() StringSet {
	return MakeStringerSet(list...)
}
func (list DirSet) Join(delim string) string {
	return JoinString(delim, list...)
}
func (list DirSet) Local(path Directory) StringSet {
	return NewStringSet(Map(MakeLocalDirectory, list...)...)
}

/***************************************
 * FileSet
 ***************************************/

type FileSet []Filename

func NewFileSet(x ...Filename) FileSet {
	return x
}

func (list FileSet) Len() int           { return len(list) }
func (list FileSet) Less(i, j int) bool { return list[i].Compare(list[j]) < 0 }
func (list FileSet) Slice() []Filename  { return list }
func (list FileSet) Swap(i, j int)      { list[i], list[j] = list[j], list[i] }

func (list *FileSet) Sort() {
	sort.Slice(list.Slice(), func(i, j int) bool {
		return (*list)[i].Compare((*list)[j]) < 0
	})
}
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
func (list *FileSet) Serialize(ar Archive) {
	SerializeSlice(ar, (*[]Filename)(list))
}
func (list FileSet) Equals(other FileSet) bool {
	if len(list) != len(other) {
		return false
	}
	for i, it := range list {
		if !other[i].Equals(it) {
			return false
		}
	}
	return true
}
func (list FileSet) StringSet() StringSet {
	return MakeStringerSet(list...)
}
func (list FileSet) Join(delim string) string {
	return JoinString(delim, list...)
}
func (list FileSet) Local(path Directory) StringSet {
	return NewStringSet(Map(MakeLocalFilename, list...)...)
}

/***************************************
 * JSON: marshal as string instead of array
 ***************************************/

func (x Filename) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *Filename) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

func (x Directory) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *Directory) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

/***************************************
 * Frontend
 ***************************************/

var UFS UFSFrontEnd = make_ufs_frontend()

type UFSFrontEnd struct {
	Executable Filename
	Working    Directory

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
	return ufs.SetMTime(dst, time.Now().Local())
}
func (ufs *UFSFrontEnd) SetMTime(dst Filename, mtime time.Time) error {
	LogDebug("ufs: chtimes %v", dst)
	path := dst.String()
	if err := os.Chtimes(path, mtime, mtime); err == nil {
		invalidate_file_info(dst)
		return nil
	} else {
		return err
	}
}
func (ufs *UFSFrontEnd) Mkdir(dst Directory) {
	if err := ufs.MkdirEx(dst); err != nil {
		LogPanicErr(err)
	}
}
func (ufs *UFSFrontEnd) MkdirEx(dst Directory) error {
	path := dst.String()
	if st, err := os.Stat(path); st != nil && (err == nil || os.IsExist(err)) {
		if !st.IsDir() {
			LogDebug("ufs: mkdir %v", dst)
			return fmt.Errorf("ufs: '%v' already exist, but is not a directory", dst)
		}
	} else {
		LogDebug("ufs: mkdir %v", dst)
		invalidate_directory_info(dst)
		if err := os.MkdirAll(path, os.ModePerm); err != nil {
			return fmt.Errorf("ufs: mkdir '%v' got error %v", dst, err)
		}
	}
	return nil
}
func (ufs *UFSFrontEnd) CreateWriter(dst Filename) (*os.File, error) {
	invalidate_file_info(dst)
	ufs.Mkdir(dst.Dirname)
	LogDebug("ufs: create '%v'", dst)
	return os.Create(dst.String())
}
func (ufs *UFSFrontEnd) Create(dst Filename, write func(io.Writer) error) error {
	outp, err := ufs.CreateWriter(dst)
	if err == nil {
		defer outp.Close()
		if err = write(outp); err == nil {
			return nil
		}
	}
	LogWarning("ufs: caught %v while trying to create %v", err, dst)
	return err
}
func (ufs *UFSFrontEnd) CreateBuffered(dst Filename, write func(io.Writer) error) error {
	return ufs.Create(dst, func(w io.Writer) error {
		buffered := bufio.NewWriter(w)
		if err := write(buffered); err != nil {
			return err
		}
		return buffered.Flush()
	})
}
func (ufs *UFSFrontEnd) SafeCreate(dst Filename, write func(io.Writer) error) error {
	ufs.Mkdir(dst.Dirname)

	file, err := os.CreateTemp(
		dst.Dirname.Relative(UFS.Root),
		dst.ReplaceExt("-*"+dst.Ext()).Relative(dst.Dirname))
	if err != nil {
		return err
	}
	defer os.Remove(file.Name())

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

	file.Close()
	LogWarning("UFS.SafeCreate: %v", err)
	return err
}
func (ufs *UFSFrontEnd) MTime(src Filename) time.Time {
	if info, err := src.Info(); err == nil {
		return info.ModTime()
	} else {
		LogPanicErr(err)
		return time.Time{}
	}
}
func (ufs *UFSFrontEnd) OpenFile(src Filename, read func(*os.File) error) error {
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
func (ufs *UFSFrontEnd) Open(src Filename, read func(io.Reader) error) error {
	return ufs.OpenFile(src, func(f *os.File) error {
		return read(f)
	})
}
func (ufs *UFSFrontEnd) OpenBuffered(src Filename, read func(io.Reader) error) error {
	return ufs.Open(src, func(r io.Reader) error {
		buffered := bufio.NewReader(r)
		return read(buffered)
	})
}
func (ufs *UFSFrontEnd) ReadAll(src Filename, read func([]byte) error) error {
	return UFS.Open(src, func(r io.Reader) error {
		useBuffer := func(buffer []byte) error {
			n, err := r.Read(buffer)
			if err == io.EOF {
				err = nil
			}
			if err == nil {
				return read(buffer[:n])
			} else {
				return err
			}
		}

		// check if the file is small enough to fit in a transient buffer
		if info, err := src.Info(); info.Size() < TRANSIENT_BYTES_CAPACITY {
			LogPanicIfFailed(err)

			transient := TransientBytes.Allocate()
			defer TransientBytes.Release(transient)
			return useBuffer(transient)

		} else {
			// for large files we revert to a dedicated allocation
			largeBuffer := make([]byte, info.Size())
			return useBuffer(largeBuffer) // don't want to keep large allocations alive
		}
	})
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

func (ufs *UFSFrontEnd) MountOutputDir(output Directory) {
	LogTrace("ufs: mount output dir %q", output)
	ufs.Output = output
	ufs.Binaries = ufs.Output.Folder("Binaries")
	ufs.Cache = ufs.Output.Folder("Cache")
	ufs.Generated = ufs.Output.Folder("Generated")
	ufs.Intermediate = ufs.Output.Folder("Intermediate")
	ufs.Projects = ufs.Output.Folder("Projects")
	ufs.Transient = ufs.Output.Folder("Transient")
	ufs.Saved = ufs.Output.Folder("Saved")
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

	ufs.Root = ufs.Dir(caller)

	executable, err := os.Executable()
	LogPanicIfFailed(err)

	ufs.Executable = MakeFilename(executable)
	if !ufs.Executable.Exists() {
		ufs.Executable = ufs.Executable.ReplaceExt(".exe")
	}
	if !ufs.Executable.Exists() {
		LogPanic("hal: executable path %q does not point to a valid file", ufs.Executable)
	}

	if wd, err := os.Getwd(); err == nil {
		ufs.Working = MakeDirectory(wd)
	} else {
		LogPanic("working directory: %v", err)
	}

	ufs.Build = ufs.Root.Folder("Build")
	ufs.Extras = ufs.Root.Folder("Extras")
	ufs.Source = ufs.Root.Folder("Source")
	ufs.MountOutputDir(ufs.Root.Folder("Output"))

	return ufs
}

func MakeGlobRegexp(glob ...string) *regexp.Regexp {
	if len(glob) == 0 {
		return nil
	}
	expr := "(?i)("
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
	return regexp.MustCompile(expr + ")")
}

/***************************************
 * UFS Bindings for Build Graph
 ***************************************/

func (x Filename) Alias() BuildAlias {
	return BuildAlias(x.String())
}
func (x Filename) Digest() (BuildStamp, error) {
	x.Invalidate()
	if info, err := x.Info(); err == nil {
		return MakeTimedBuildStamp(info.ModTime(), &x), nil
	} else {
		return BuildStamp{}, err
	}
}
func (x Filename) Build(bc BuildContext) error {
	x.Invalidate()
	if info, err := x.Info(); err == nil {
		bc.Timestamp(info.ModTime())
		return nil
	} else {
		return err
	}
}
func (x *Filename) Serialize(ar Archive) {
	ar.Serializable(&x.Dirname)
	ar.String(&x.Basename)
}

func (x Directory) Alias() BuildAlias {
	return BuildAlias(x.String())
}
func (x Directory) Build(bc BuildContext) error {
	x.Invalidate()
	if info, err := x.Info(); err == nil {
		bc.Timestamp(GetCreationTime(info))
		return nil
	} else {
		return err
	}
}
func (x *Directory) Serialize(ar Archive) {
	SerializeMany(ar, ar.String, (*[]string)(x))
}

func BuildFile(source Filename, staticDeps ...BuildAlias) BuildFactoryTyped[*Filename] {
	return func(bi BuildInitializer) (*Filename, error) {
		buildable := new(Filename)
		*buildable = source //.Normalize()
		return buildable, bi.DependsOn(staticDeps...)
	}
}
func BuildDirectory(source Directory, staticDeps ...BuildAlias) BuildFactoryTyped[*Directory] {
	return func(bi BuildInitializer) (*Directory, error) {
		buildable := new(Directory)
		*buildable = source //.Normalize()
		return buildable, bi.DependsOn(staticDeps...)
	}
}

/***************************************
 * File Digest
 ***************************************/

func DigestFile(bc BuildContext, source Filename) (Fingerprint, error) {
	file, err := BuildFileDigest(source).Need(bc)
	return file.Digest, err
}

type FileDigest struct {
	Source Filename
	Digest Fingerprint
}

func BuildFileDigest(source Filename) BuildFactoryTyped[*FileDigest] {
	return func(bi BuildInitializer) (*FileDigest, error) {
		if err := bi.NeedFile(source); err != nil {
			return nil, err
		}
		return &FileDigest{
			Source: source, //.Normalize(),
		}, nil
	}
}

func (x *FileDigest) Alias() BuildAlias {
	return MakeBuildAlias("UFS", "Digest", x.Source.String())
}
func (x *FileDigest) Build(bc BuildContext) (err error) {
	x.Digest, err = FileFingerprint(x.Source, Fingerprint{} /* no seed here */)
	LogTrace("ufs: file digest %s for %q", x.Digest.ShortString(), x.Source)
	return
}
func (x *FileDigest) Serialize(ar Archive) {
	ar.Serializable(&x.Source)
	ar.Serializable(&x.Digest)
}

/***************************************
 * Directory Creator
 ***************************************/

func CreateDirectory(bc BuildInitializer, source Directory) error {
	_, err := BuildDirectoryCreator(source).Need(bc)
	return err
}

type DirectoryCreator struct {
	Source Directory
}

func BuildDirectoryCreator(source Directory) BuildFactoryTyped[*DirectoryCreator] {
	return func(init BuildInitializer) (*DirectoryCreator, error) {
		return &DirectoryCreator{
			Source: source, //.Normalize(),
		}, nil
	}
}

func (x *DirectoryCreator) Alias() BuildAlias {
	return MakeBuildAlias("UFS", "Create", x.Source.String())
}
func (x *DirectoryCreator) Build(bc BuildContext) error {
	if err := bc.OutputNode(BuildDirectory(x.Source)); err != nil {
		return err
	}

	return UFS.MkdirEx(x.Source)
}
func (x *DirectoryCreator) Serialize(ar Archive) {
	ar.Serializable(&x.Source)
}

/***************************************
 * Directory List
 ***************************************/

func ListDirectory(bc BuildContext, source Directory) (FileSet, error) {
	factory := BuildDirectoryList(source)
	if list, err := factory.Need(bc); err == nil {
		return list.Results, nil
	} else {
		return FileSet{}, err
	}
}

type DirectoryList struct {
	Source  Directory
	Results FileSet
}

func BuildDirectoryList(source Directory) BuildFactoryTyped[*DirectoryList] {
	return func(init BuildInitializer) (*DirectoryList, error) {
		if err := init.NeedDirectory(source); err != nil {
			return nil, err
		}
		return &DirectoryList{
			Source:  source, // .Normalize(),
			Results: FileSet{},
		}, nil
	}
}

func (x *DirectoryList) Alias() BuildAlias {
	return MakeBuildAlias("UFS", "List", x.Source.String())
}
func (x *DirectoryList) Build(bc BuildContext) error {
	x.Results = FileSet{}

	if info, err := x.Source.Info(); err == nil {
		bc.Timestamp(info.ModTime())
	} else {
		return err
	}

	x.Results = x.Source.Files()
	for i, filename := range x.Results {
		//filename = filename.Normalize()
		x.Results[i] = filename

		if err := bc.NeedFile(filename); err != nil {
			return err
		}
	}

	return nil
}
func (x *DirectoryList) Serialize(ar Archive) {
	ar.Serializable(&x.Source)
	ar.Serializable(&x.Results)
}

/***************************************
 * Directory Glob
 ***************************************/

func GlobDirectory(
	bc BuildContext,
	source Directory,
	includedGlobs StringSet,
	excludedGlobs StringSet,
	excludedFiles FileSet) (FileSet, error) {
	factory := BuildDirectoryGlob(source, includedGlobs, excludedGlobs, excludedFiles)
	if glob, err := factory.Need(bc); err == nil {
		return glob.Results, nil
	} else {
		return FileSet{}, err
	}
}

type DirectoryGlob struct {
	Source        Directory
	IncludedGlobs StringSet
	ExcludedGlobs StringSet
	ExcludedFiles FileSet
	Results       FileSet
}

func BuildDirectoryGlob(
	source Directory,
	includedGlobs StringSet,
	excludedGlobs StringSet,
	excludedFiles FileSet) BuildFactoryTyped[*DirectoryGlob] {
	return func(init BuildInitializer) (*DirectoryGlob, error) {
		if err := init.NeedDirectory(source); err != nil {
			return nil, err
		}
		return &DirectoryGlob{
			Source:        source.Normalize(),
			IncludedGlobs: includedGlobs,
			ExcludedGlobs: excludedGlobs,
			ExcludedFiles: excludedFiles,
			Results:       FileSet{},
		}, nil
	}
}

func (x *DirectoryGlob) Alias() BuildAlias {
	return MakeBuildAlias("UFS", "Glob", strings.Join([]string{
		x.Source.String(),
		x.IncludedGlobs.Join(";"),
		x.ExcludedGlobs.Join(";"),
		x.ExcludedFiles.Join(";")},
		"|"))
}
func (x *DirectoryGlob) Build(bc BuildContext) error {
	x.Results = FileSet{}

	if dirInfo, err := x.Source.Info(); err == nil {
		bc.Timestamp(dirInfo.ModTime())
	} else {
		return err
	}

	includeRE := MakeGlobRegexp(x.IncludedGlobs...)
	excludeRE := MakeGlobRegexp(x.ExcludedGlobs...)
	if includeRE == nil {
		includeRE = MakeGlobRegexp("*")
	}

	return x.Source.MatchFilesRec(func(f Filename) error {
		// f = f.Normalize()
		if !x.ExcludedFiles.Contains(f) {
			if excludeRE == nil || !excludeRE.MatchString(f.String()) {
				x.Results.Append(f)
			}
		}
		return nil
	}, includeRE)
}
func (x *DirectoryGlob) Serialize(ar Archive) {
	ar.Serializable(&x.Source)
	ar.Serializable(&x.IncludedGlobs)
	ar.Serializable(&x.ExcludedGlobs)
	ar.Serializable(&x.ExcludedFiles)
	ar.Serializable(&x.Results)
}
