package utils

import (
	"archive/zip"
	"bytes"
	"io"
	"os"
	"sync"
)

func NewZipExtractor(src Filename, dst Directory, acceptList StringSet, staticDeps ...BuildAlias) *ZipExtractor {
	result := &ZipExtractor{
		Source:      src,
		Destination: dst,
		AcceptList:  NewStringSet(acceptList...),
	}
	return CommandEnv.BuildGraph().
		Create(result, append(staticDeps, src.Alias())...).
		GetBuildable().(*ZipExtractor)
}

type ZipExtractor struct {
	Source         Filename
	Destination    Directory
	AcceptList     StringSet
	ExtractedFiles FileSet
}

func (zip *ZipExtractor) Alias() BuildAlias {
	return MakeBuildAlias("Zip", zip.Destination.String())
}
func (zip *ZipExtractor) Build(bc BuildContext) (BuildStamp, error) {
	globRe := MakeGlobRegexp(zip.AcceptList.Slice()...)
	matchString := func(s string) bool {
		if globRe != nil {
			return globRe.MatchString(s)
		} else {
			return true
		}
	}

	zip.ExtractedFiles = NewFileSet()
	err := UnzipEx(func(s string) (Filename, bool) {
		dst := zip.Destination.AbsoluteFile(s)
		if matchString(s) {
			zip.ExtractedFiles.Append(dst)
			return dst, true
		}
		return dst, false
	}, zip.Source)

	for _, f := range zip.ExtractedFiles {
		bc.OutputFile(f)
	}

	if err == nil {
		return MakeBuildStamp(zip)
	} else {
		return BuildStamp{}, err
	}
}
func (zip *ZipExtractor) GetDigestable(o *bytes.Buffer) {
	zip.Source.GetDigestable(o)
	zip.Destination.GetDigestable(o)
	zip.AcceptList.GetDigestable(o)
	zip.ExtractedFiles.GetDigestable(o)
}

func Unzip(dst Directory, src Filename) error {
	return UnzipEx(func(s string) (Filename, bool) {
		return dst.AbsoluteFile(s), true
	}, src)
}
func UnzipEx(exportFilter func(string) (Filename, bool), src Filename) error {
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

		_, err = io.Copy(output, input)
		if err != nil {
			return err
		}

		return nil
	}

	pbar := LogProgress(0, len(rd.File), src.String())
	defer pbar.Close()

	wg := sync.WaitGroup{}
	defer wg.Wait()

	for _, f := range rd.File {
		validated := false
		if !f.FileInfo().IsDir() {
			var fpath Filename
			if fpath, validated = exportFilter(f.Name); validated {
				LogTrace("zip: extracting '%v'", fpath)

				wg.Add(1)
				go func(zip *zip.File) {
					defer pbar.Inc()
					defer wg.Done()

					UFS.Mkdir(fpath.Dirname)
					if err := extractFile(zip, fpath); err != nil {
						panic(err)
					}
				}(f)
			}
		}

		if !validated {
			pbar.Inc()
		}
	}

	return nil
}
