package utils

import (
	"archive/tar"
	"archive/zip"
	"bytes"
	"compress/gzip"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
)

func NewZipExtractor(src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAlias) *ArchiveExtractor {
	return NewArchiveExtractor(ARCHIVE_ZIP, src, dst, acceptList, staticDeps...)
}
func NewTgzExtractor(src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAlias) *ArchiveExtractor {
	return NewArchiveExtractor(ARCHIVE_TGZ, src, dst, acceptList, staticDeps...)
}
func NewArchiveExtractor(arType ArchiveType, src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAlias) *ArchiveExtractor {
	result := &ArchiveExtractor{
		Type:        arType,
		Source:      src,
		Destination: dst,
		AcceptList:  NewStringSet(acceptList...),
	}
	return CommandEnv.BuildGraph().
		Create(result, append(staticDeps, src.Alias())...).
		GetBuildable().(*ArchiveExtractor)
}
func NewArchiveExtractorFromExt(src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAlias) *ArchiveExtractor {
	var arType ArchiveType
	extname := src.Ext()
	extNoDot := strings.TrimLeft(extname, ".")
	if err := arType.Set(extNoDot); err != nil {
		LogPanic("unknown archive type: %v", err)
		return nil
	}
	return NewArchiveExtractor(arType, src, dst, acceptList, staticDeps...)
}

type ArchiveType int32

const (
	ARCHIVE_ZIP ArchiveType = iota
	ARCHIVE_TGZ
)

func (x ArchiveType) String() string {
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
func (x *ArchiveType) Set(in string) (err error) {
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
func (x ArchiveType) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.String())
}
func (x ArchiveType) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *ArchiveType) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

type ArchiveExtractor struct {
	Type           ArchiveType
	Source         Filename
	Destination    Directory
	AcceptList     StringSet
	ExtractedFiles FileSet
}

func (ar *ArchiveExtractor) Alias() BuildAlias {
	return MakeBuildAlias(ar.Type.String(), ar.Destination.String())
}
func (ar *ArchiveExtractor) Build(bc BuildContext) (BuildStamp, error) {
	globRe := MakeGlobRegexp(ar.AcceptList.Slice()...)
	matchString := func(s string) bool {
		if globRe != nil {
			return globRe.MatchString(s)
		} else {
			return true
		}
	}

	exportFilter := func(s string) (Filename, bool) {
		dst := ar.Destination.AbsoluteFile(s)
		if matchString(s) {
			ar.ExtractedFiles.Append(dst)
			return dst, true
		}
		return dst, false
	}

	var err error
	ar.ExtractedFiles = NewFileSet()
	switch ar.Type {
	case ARCHIVE_TGZ:
		err = ExtractTgzEx(exportFilter, ar.Source)
	case ARCHIVE_ZIP:
		err = ExtractZipEx(exportFilter, ar.Source)
	default:
		UnexpectedValue(ar.Type)
	}

	if err == nil {
		for _, f := range ar.ExtractedFiles {
			bc.OutputFile(f)
		}

		return MakeBuildStamp(ar)
	} else {
		return BuildStamp{}, err
	}
}
func (ar *ArchiveExtractor) GetDigestable(o *bytes.Buffer) {
	ar.Type.GetDigestable(o)
	ar.Source.GetDigestable(o)
	ar.Destination.GetDigestable(o)
	ar.AcceptList.GetDigestable(o)
	ar.ExtractedFiles.GetDigestable(o)
}

/***************************************
 * TGZ
 ***************************************/

func UnGzip(source, target string) error {
	LogVerbose("gzip: decompress to '%v'...", target)
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

func ExtractTgz(dst Directory, src Filename) error {
	return ExtractTgzEx(func(s string) (Filename, bool) {
		return dst.AbsoluteFile(s), true
	}, src)
}
func ExtractTgzEx(exportFilter func(string) (Filename, bool), src Filename) error {
	LogVerbose("tgz: extracting archive '%v'...", src)

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
			LogTrace("tgz: extracting '%v'", fpath)

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

func ExtractZip(dst Directory, src Filename) error {
	return ExtractZipEx(func(s string) (Filename, bool) {
		return dst.AbsoluteFile(s), true
	}, src)
}
func ExtractZipEx(exportFilter func(string) (Filename, bool), src Filename) error {
	LogVerbose("zip: extracting archive '%v'...", src)

	rd, err := zip.OpenReader(src.String())
	if err != nil {
		return err
	}
	defer rd.Close()

	extractFile := func(f *zip.File, dst Filename) error {
		output, err := os.OpenFile(dst.String(), os.O_WRONLY|os.O_CREATE|os.O_TRUNC, f.Mode())
		if err != nil {
			return err
		}
		defer output.Close()

		input, err := f.Open()
		if err != nil {
			return err
		}
		defer input.Close()

		if err := CopyWithProgress(dst.Basename, f.FileInfo().Size(), output, input); err != nil {
			return err
		}

		return nil
	}

	pbar := LogProgress(0, len(rd.File), "extracting %s", src.String())
	defer pbar.Close()

	for _, f := range rd.File {
		validated := false
		if !f.FileInfo().IsDir() {
			var fpath Filename
			if fpath, validated = exportFilter(f.Name); validated {
				LogTrace("zip: extracting '%v'", fpath)

				UFS.Mkdir(fpath.Dirname)
				if err := extractFile(f, fpath); err != nil {
					LogPanicErr(err)
				}

				pbar.Inc()
			}
		}

		if !validated {
			pbar.Inc()
		}
	}

	return nil
}
