package compile

import (
	//lint:ignore ST1001 ignore dot imports warning
	. "build/utils"
	"fmt"
	"io"
	"math"
	"os"
	"sort"
)

type UnityRules struct {
	SourceFiles FileSet
	UnityFiles  BuildAliases
}

type Unity interface {
	GetUnity() *UnityRules
	Serializable
}

/***************************************
 * Unity Rules
 ***************************************/

func (x *UnityRules) GetUnity() *UnityRules { return x }
func (x *UnityRules) Serialize(ar Archive) {
	ar.Serializable(&x.SourceFiles)
	SerializeSlice(ar, x.UnityFiles.Ref())
}
func (x *UnityRules) Generate(u *Unit, bc BuildContext) (err error) {
	for _, filename := range x.UnityFiles {
		os.RemoveAll(filename.String())
	}

	x.SourceFiles = FileSet{}
	x.UnityFiles = BuildAliases{}

	x.SourceFiles, err = u.Source.GetFileSet(bc)
	if err != nil {
		return
	}
	if err = bc.NeedFile(x.SourceFiles...); err != nil {
		return
	}

	isolatedFiles := u.Source.IsolatedFiles
	if u.AdaptiveUnity.Get() {
		var scm *SourceControlModifiedFiles
		scm, err = BuildSourceControlModifiedFiles().Need(bc)
		if err != nil {
			return
		}

		isolatedFiles := NewFileSet(u.Source.IsolatedFiles...)
		for _, file := range scm.ModifiedFiles {
			if x.SourceFiles.Contains(file) {
				LogVerbose(LogCompile, "%v: adaptive unity isolated %q", u.Target, file)
				isolatedFiles.AppendUniq(file)
			}
		}
	}

	var totalSize int64
	sourceFileSizes := make([]int64, len(x.SourceFiles))
	for i, it := range x.SourceFiles {
		if info, err := it.Info(); err == nil {
			var size int64
			if !isolatedFiles.Contains(it) {
				size = info.Size()
			}
			sourceFileSizes[i] = size
			totalSize += size
		} else {
			return err
		}
	}

	var numUnityFiles int
	switch u.Unity {
	case UNITY_DISABLED:
		numUnityFiles = 0
	case UNITY_AUTOMATIC:
		numUnityFiles = int(math.Ceil(float64(totalSize) / float64(u.SizePerUnity)))
		LogVeryVerbose(LogCompile, "%v: %d unity files (%.2f KiB)", u.Target, numUnityFiles, float64(totalSize)/1024.0)
	case UNITY_INHERIT:
		UnexpectedValuePanic(u.Unity, UNITY_INHERIT)
	default:
		if u.Unity.Ord() > 0 {
			numUnityFiles = int(u.Unity.Ord())
		} else {
			UnexpectedValuePanic(u.Unity, u.Unity)
		}
	}

	if numUnityFiles >= len(u.SourceFiles) {
		LogWarning(LogCompile, "%v: %d unity files (%.2f KiB) is superiori to source files count (%d files), disabling unity (was %v)", u.Target, numUnityFiles, len(x.SourceFiles), float64(totalSize)/1024.0, u.Unity)
		numUnityFiles = 0
	}

	if numUnityFiles == 0 {
		// keep original source fileset
		return nil
	}
	AssertMessage(func() bool { return numUnityFiles > 0 }, "unity: invalid count of unity files %s (was %v)", numUnityFiles, u.Unity)

	// generate unity files
	unityDir := u.GeneratedDir.Folder("Unity")
	if err = CreateDirectory(bc, unityDir); err != nil {
		return
	}

	// detect PCH parameters: shoud unity files include "stdafx.h"?
	unityIncludes := StringSet{}
	switch u.PCH {
	case PCH_MONOLITHIC, PCH_SHARED:
		// add pch header
		unityIncludes.Append(u.PrecompiledHeader.Basename)
	case PCH_DISABLED:
		// no includes
	default:
		UnexpectedValuePanic(u.PCH, u.PCH)
	}

	// sort source files by descending size
	sourceFilesBySize := make([]int, len(x.SourceFiles))
	for i := range sourceFilesBySize {
		sourceFilesBySize[i] = i
	}
	sort.Slice(sourceFilesBySize, func(i, j int) bool {
		a, b := sourceFilesBySize[i], sourceFilesBySize[j]
		return sourceFileSizes[a] > sourceFileSizes[b]
	})

	// cluster source files from largest to smallest
	unitySizes := make([]int64, numUnityFiles)
	unityInputs := make([]FileSet, numUnityFiles)
	unityOutputs := make(FileSet, numUnityFiles)
	for i := range unityOutputs {
		unityOutputs[i] = unityDir.File(fmt.Sprintf("Unity_%d_of_%d.cpp", (i + 1), numUnityFiles))
	}

	for _, i := range sourceFilesBySize {
		if sourceFileSizes[i] == 0 {
			continue // this file is isolated
		}

		// find the smallest unity file
		unityDst := 0
		for j := 1; j < numUnityFiles; j += 1 {
			if unitySizes[j] < unitySizes[unityDst] {
				unityDst = j
			}
		}

		// append source file to chosen unity file
		unitySizes[unityDst] += sourceFileSizes[i]
		unityInputs[unityDst].Append(x.SourceFiles[i])
	}

	for i, outputFile := range unityOutputs {
		if unityFile, err := bc.NeedFactory(BuildUnityFile(outputFile, unityIncludes, unityInputs[i])); err == nil {
			x.UnityFiles.Append(unityFile.Alias())
		} else {
			return err
		}
	}

	// replace source fileset by generated unity + isolated files
	x.SourceFiles = append(unityOutputs, isolatedFiles...)
	return
}

/***************************************
 * Unity File
 ***************************************/

type UnityFile struct {
	Output   Filename
	Includes StringSet
	Inputs   FileSet
}

func BuildUnityFile(output Filename, includes StringSet, inputs FileSet) BuildFactoryTyped[*UnityFile] {
	Assert(func() bool { return output.Alias().Valid() })
	Assert(func() bool { return len(inputs) > 0 })
	return MakeBuildFactory(func(bi BuildInitializer) (UnityFile, error) {
		return UnityFile{
			Output:   output,
			Includes: includes,
			Inputs:   inputs,
		}, bi.NeedFile(inputs...)
	})
}
func (x *UnityFile) Alias() BuildAlias {
	return MakeBuildAlias("Compile", "Unity", x.Output.String())
}
func (x *UnityFile) Build(bc BuildContext) error {
	err := UFS.CreateBuffered(x.Output, func(w io.Writer) error {
		cpp := NewCppFile(w, true)
		for _, it := range x.Includes {
			cpp.Include(it)
		}
		for _, it := range x.Inputs {
			cpp.Pragma("message(\"unity: \" %q)", it)
			cpp.Include(SanitizePath(it.Relative(UFS.Source), '/'))
		}
		return nil
	})

	if err == nil {
		err = bc.OutputFile(x.Output)
	}
	return err
}
func (x *UnityFile) Serialize(ar Archive) {
	ar.Serializable(&x.Output)
	ar.Serializable(&x.Includes)
	ar.Serializable(&x.Inputs)
}
