package utils

import (
	"archive/tar"
	"archive/zip"
	"compress/gzip"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
)

var LogCompressedArchive = NewLogCategory("CompressedArchive")

type CompressedArchiveFromDownload struct {
	Download   *Downloader
	ExtractDir Directory
}

func (x CompressedArchiveFromDownload) Equals(other CompressedArchiveFromDownload) bool {
	return (x.Download == other.Download && x.ExtractDir.Equals(other.ExtractDir))
}

func BuildZipExtractor(src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAliasable) BuildFactoryTyped[*CompressedArchiveExtractor] {
	return BuildCompressedArchiveExtractor(ARCHIVE_ZIP, src, dst, acceptList, staticDeps...)
}
func BuildTgzExtractor(src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAliasable) BuildFactoryTyped[*CompressedArchiveExtractor] {
	return BuildCompressedArchiveExtractor(ARCHIVE_TGZ, src, dst, acceptList, staticDeps...)
}
func BuildCompressedArchiveExtractor(arType CompressedArchiveType, src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAliasable) BuildFactoryTyped[*CompressedArchiveExtractor] {
	return MakeBuildFactory(func(bi BuildInitializer) (CompressedArchiveExtractor, error) {
		return CompressedArchiveExtractor{
				Type:        arType,
				Source:      src,
				Destination: dst,
				AcceptList:  acceptList,
			}, AnyError(
				bi.NeedFile(src),
				bi.NeedBuildable(staticDeps...))
	})
}
func BuildCompressedArchiveExtractorFromExt(src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAliasable) BuildFactoryTyped[*CompressedArchiveExtractor] {
	var arType CompressedArchiveType
	extname := src.Ext()
	extNoDot := strings.TrimLeft(extname, ".")
	if err := arType.Set(extNoDot); err != nil {
		LogPanic(LogCompressedArchive, "unknown archive type: %v", err)
	}
	return BuildCompressedArchiveExtractor(arType, src, dst, acceptList, staticDeps...)
}
func BuildCompressedArchiveExtractorFromDownload(prms CompressedArchiveFromDownload, acceptList StringSet) BuildFactoryTyped[*CompressedArchiveExtractor] {
	return BuildCompressedArchiveExtractorFromExt(
		prms.Download.Destination,
		prms.ExtractDir,
		acceptList,
		prms.Download)
}

/***************************************
 * Compressed Archive Type
 ***************************************/

type CompressedArchiveType int32

const (
	ARCHIVE_ZIP CompressedArchiveType = iota
	ARCHIVE_TGZ
)

func (x CompressedArchiveType) String() string {
	switch x {
	case ARCHIVE_ZIP:
		return "ZIP"
	case ARCHIVE_TGZ:
		return "TGZ"
	default:
		UnexpectedValue(x)
		return ""
	}
}
func (x *CompressedArchiveType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case ARCHIVE_ZIP.String():
		*x = ARCHIVE_ZIP
	case ARCHIVE_TGZ.String():
		*x = ARCHIVE_TGZ
	default:
		err = MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *CompressedArchiveType) Serialize(ar Archive) {
	ar.Int32((*int32)(x))
}
func (x CompressedArchiveType) MarshalText() ([]byte, error) {
	return UnsafeBytesFromString(x.String()), nil
}
func (x *CompressedArchiveType) UnmarshalText(data []byte) error {
	return x.Set(UnsafeStringFromBytes(data))
}

/***************************************
 * Compressed Archive Extractor
 ***************************************/

type CompressedArchiveExtractor struct {
	Type           CompressedArchiveType
	Source         Filename
	Destination    Directory
	AcceptList     StringSet
	ExtractedFiles FileSet
}

func (x CompressedArchiveExtractor) Alias() BuildAlias {
	return MakeBuildAlias(x.Type.String(), x.Destination.String())
}
func (x *CompressedArchiveExtractor) Build(bc BuildContext) error {
	globRe := MakeGlobRegexp(x.AcceptList.Slice()...)
	matchString := func(s string) bool {
		if globRe != nil {
			return globRe.MatchString(s)
		} else {
			return true
		}
	}

	exportFilter := func(s string) (Filename, bool) {
		dst := x.Destination.AbsoluteFile(s)
		if matchString(s) {
			x.ExtractedFiles.Append(dst)
			return dst, true
		}
		return dst, false
	}

	x.ExtractedFiles = NewFileSet()

	switch x.Type {
	case ARCHIVE_TGZ:
		if err := ExtractTgzEx(bc, exportFilter, x.Source); err != nil {
			return err
		}
	case ARCHIVE_ZIP:
		if err := ExtractZipEx(bc, exportFilter, x.Destination, x.Source); err != nil {
			return err
		}
	default:
		UnexpectedValue(x.Type)
	}

	for _, f := range x.ExtractedFiles {
		if err := bc.OutputFile(f); err != nil {
			return err
		}
	}

	// avoid re-extracting after each rebuild
	bc.Timestamp(UFS.MTime(x.Source))

	return nil
}
func (x *CompressedArchiveExtractor) Serialize(ar Archive) {
	ar.Serializable(&x.Type)
	ar.Serializable(&x.Source)
	ar.Serializable(&x.Destination)
	ar.Serializable(&x.AcceptList)
	ar.Serializable(&x.ExtractedFiles)
}

/***************************************
 * TGZ
 ***************************************/

func UnGzip(source, target string) error {
	LogVerbose(LogCompressedArchive, "decompress gzip to '%v'...", target)
	reader, err := os.Open(source)
	if err != nil {
		return err
	}
	defer reader.Close()

	archive, err := gzip.NewReader(reader)
	if err != nil {
		return err
	}
	defer archive.Close()

	target = filepath.Join(target, archive.Name)
	writer, err := os.Create(target)
	if err != nil {
		return err
	}
	defer writer.Close()

	return CopyWithSpinner(target, writer, archive)
}

func ExtractTgz(bc BuildContext, dst Directory, src Filename) error {
	return ExtractTgzEx(bc, func(s string) (Filename, bool) {
		return dst.AbsoluteFile(s), true
	}, src)
}
func ExtractTgzEx(bc BuildContext, exportFilter func(string) (Filename, bool), src Filename) error {
	LogVerbose(LogCompressedArchive, "extracting tgz archive '%v'...", src)

	tarball := src.ReplaceExt(".tar").String()
	if err := UnGzip(src.String(), tarball); err != nil {
		return err
	}

	reader, err := os.Open(tarball)
	if err != nil {
		return err
	}
	defer reader.Close()
	tarReader := tar.NewReader(reader)

	pbar := LogSpinner(tarball)
	defer pbar.Close()

	extractFile := func(fpath Filename, info fs.FileInfo) error {
		file, err := os.OpenFile(fpath.String(), os.O_CREATE|os.O_TRUNC|os.O_WRONLY, info.Mode())
		if err != nil {
			return err
		}
		defer file.Close()
		return CopyWithProgress(fpath.Basename, info.Size(), file, tarReader)
	}

	for {
		header, err := tarReader.Next()
		if err == io.EOF {
			break
		} else if err != nil {
			return err
		}

		info := header.FileInfo()
		if info.IsDir() {
			continue
		}

		if fpath, validated := exportFilter(header.Name); validated {
			LogTrace(LogCompressedArchive, "extracting from tgz '%v'", fpath)

			UFS.Mkdir(fpath.Dirname)
			if err := extractFile(fpath, info); err != nil {
				return err
			}
		}

		pbar.Inc()
	}
	return nil
}

/***************************************
 * ZIP
 ***************************************/

func ExtractZip(bc BuildContext, dst Directory, src Filename) error {
	return ExtractZipEx(bc, func(s string) (Filename, bool) {
		return dst.AbsoluteFile(s), true
	}, dst, src)
}
func ExtractZipEx(bc BuildContext, exportFilter func(string) (Filename, bool), dst Directory, src Filename) error {
	LogVerbose(LogCompressedArchive, "extracting zip archive '%v'...", src)

	rd, err := zip.OpenReader(src.String())
	if err != nil {
		return err
	}
	defer rd.Close()

	selectedFiles := make([]struct {
		zip   *zip.File
		fpath Filename
	}, 0, len(rd.File))

	for _, f := range rd.File {
		if f.FileInfo().IsDir() {
			continue
		}

		if fpath, validated := exportFilter(f.Name); validated {
			LogTrace(LogCompressedArchive, "extracting from zip '%v'", fpath)

			if _, err := BuildDirectory(fpath.Dirname).Need(bc); err != nil {
				return err
			}

			selectedFiles = append(selectedFiles, struct {
				zip   *zip.File
				fpath Filename
			}{zip: f, fpath: fpath})
		}
	}

	pbar := LogProgress(0, len(selectedFiles), "extracting %s", src.String())
	defer pbar.Close()

	return ParallelRange(func(selected struct {
		zip   *zip.File
		fpath Filename
	}) error {
		defer pbar.Inc()

		output, err := os.OpenFile(selected.fpath.String(), os.O_WRONLY|os.O_CREATE|os.O_TRUNC, selected.zip.Mode())
		if err != nil {
			return err
		}
		defer output.Close()

		input, err := selected.zip.Open()
		if err != nil {
			return err
		}
		defer input.Close()

		return CopyWithProgress(selected.fpath.Basename, selected.zip.FileInfo().Size(), output, input)

	}, selectedFiles...)
}
