package compile

import (
	utils "build/utils"
	"bytes"
	"strings"
)

type Facetable interface {
	GetFacet() *Facet
}

type FacetDecorator interface {
	Decorate(*CompileEnv, *Unit)
}

type VariableSubstitutions map[string]string

type Facet struct {
	Defines utils.StringSet

	ForceIncludes      utils.FileSet
	IncludePaths       utils.DirSet
	ExternIncludePaths utils.DirSet
	SystemIncludePaths utils.DirSet

	AnalysisOptions          utils.StringSet
	PreprocessorOptions      utils.StringSet
	CompilerOptions          utils.StringSet
	PrecompiledHeaderOptions utils.StringSet

	Libraries    utils.FileSet
	LibraryPaths utils.DirSet

	LibrarianOptions utils.StringSet
	LinkerOptions    utils.StringSet

	Tags    TagFlags
	Exports VariableSubstitutions
}

func (facet *Facet) GetFacet() *Facet {
	return facet
}
func (facet *Facet) GetDigestable(o *bytes.Buffer) {
	digest := utils.MakeDigester()
	digest.Append(facet.Defines)
	digest.Append(facet.ForceIncludes)
	digest.Append(facet.IncludePaths)
	digest.Append(facet.ExternIncludePaths)
	digest.Append(facet.SystemIncludePaths)
	digest.Append(facet.AnalysisOptions)
	digest.Append(facet.PreprocessorOptions)
	digest.Append(facet.CompilerOptions)
	digest.Append(facet.PrecompiledHeaderOptions)
	digest.Append(facet.Libraries)
	digest.Append(facet.LibraryPaths)
	digest.Append(facet.LibrarianOptions)
	digest.Append(facet.LinkerOptions)
	digest.Append(facet.Tags)
	// digest.Append(facet.Exports)
	result := digest.Finalize()
	o.Write(result[:])
}

func NewFacet() Facet {
	return Facet{
		Defines:                  utils.StringSet{},
		ForceIncludes:            utils.FileSet{},
		IncludePaths:             utils.DirSet{},
		ExternIncludePaths:       utils.DirSet{},
		SystemIncludePaths:       utils.DirSet{},
		AnalysisOptions:          utils.StringSet{},
		PreprocessorOptions:      utils.StringSet{},
		CompilerOptions:          utils.StringSet{},
		PrecompiledHeaderOptions: utils.StringSet{},
		Libraries:                utils.FileSet{},
		LibraryPaths:             utils.DirSet{},
		LibrarianOptions:         utils.StringSet{},
		LinkerOptions:            utils.StringSet{},
		Tags:                     TagFlags(0),
		Exports:                  VariableSubstitutions{},
	}
}

func (facet *Facet) Tagged(tag ...TagType) bool {
	return facet.Tags.Has(tag...)
}
func (facet *Facet) Append(others ...Facetable) {
	for _, o := range others {
		x := o.GetFacet()
		facet.Defines.Append(x.Defines...)
		facet.ForceIncludes.Append(x.ForceIncludes...)
		facet.IncludePaths.Append(x.IncludePaths...)
		facet.ExternIncludePaths.Append(x.ExternIncludePaths...)
		facet.SystemIncludePaths.Append(x.SystemIncludePaths...)
		facet.AnalysisOptions.Append(x.AnalysisOptions...)
		facet.PreprocessorOptions.Append(x.PreprocessorOptions...)
		facet.CompilerOptions.Append(x.CompilerOptions...)
		facet.PrecompiledHeaderOptions.Append(x.PrecompiledHeaderOptions...)
		facet.Libraries.Append(x.Libraries...)
		facet.LibraryPaths.Append(x.LibraryPaths...)
		facet.LibrarianOptions.Append(x.LibrarianOptions...)
		facet.LinkerOptions.Append(x.LinkerOptions...)
		facet.Tags.Append(x.Tags)
		for k, v := range x.Exports {
			if _, ok := facet.Exports[k]; !ok {
				facet.Exports[k] = v
			}
		}
	}
}
func (facet *Facet) AppendUniq(others ...Facetable) {
	for _, o := range others {
		x := o.GetFacet()
		facet.Defines.AppendUniq(x.Defines...)
		facet.ForceIncludes.AppendUniq(x.ForceIncludes...)
		facet.IncludePaths.AppendUniq(x.IncludePaths...)
		facet.ExternIncludePaths.AppendUniq(x.ExternIncludePaths...)
		facet.SystemIncludePaths.AppendUniq(x.SystemIncludePaths...)
		facet.AnalysisOptions.AppendUniq(x.AnalysisOptions...)
		facet.PreprocessorOptions.AppendUniq(x.PreprocessorOptions...)
		facet.CompilerOptions.AppendUniq(x.CompilerOptions...)
		facet.PrecompiledHeaderOptions.AppendUniq(x.PrecompiledHeaderOptions...)
		facet.Libraries.AppendUniq(x.Libraries...)
		facet.LibraryPaths.AppendUniq(x.LibraryPaths...)
		facet.LibrarianOptions.AppendUniq(x.LibrarianOptions...)
		facet.LinkerOptions.AppendUniq(x.LinkerOptions...)
		facet.Tags.Append(x.Tags)
		for k, v := range x.Exports {
			if _, ok := facet.Exports[k]; !ok {
				facet.Exports[k] = v
			}
		}
	}
}
func (facet *Facet) Prepend(others ...Facetable) {
	for _, o := range others {
		x := o.GetFacet()
		facet.Defines.Prepend(x.Defines...)
		facet.ForceIncludes.Prepend(x.ForceIncludes...)
		facet.IncludePaths.Prepend(x.IncludePaths...)
		facet.ExternIncludePaths.Prepend(x.ExternIncludePaths...)
		facet.SystemIncludePaths.Prepend(x.SystemIncludePaths...)
		facet.AnalysisOptions.Prepend(x.AnalysisOptions...)
		facet.PreprocessorOptions.Prepend(x.PreprocessorOptions...)
		facet.CompilerOptions.Prepend(x.CompilerOptions...)
		facet.PrecompiledHeaderOptions.Prepend(x.PrecompiledHeaderOptions...)
		facet.Libraries.Prepend(x.Libraries...)
		facet.LibraryPaths.Prepend(x.LibraryPaths...)
		facet.LibrarianOptions.Prepend(x.LibrarianOptions...)
		facet.LinkerOptions.Prepend(x.LinkerOptions...)
		facet.Tags.Append(facet.Tags)
		for k, v := range x.Exports {
			facet.Exports[k] = v
		}
	}
}

func (facet *Facet) AddCompilationFlag(flags ...string) {
	facet.AnalysisOptions.Append(flags...)
	facet.AddCompilationFlag_NoAnalysis(flags...)
}
func (facet *Facet) AddCompilationFlag_NoAnalysis(flags ...string) {
	facet.PrecompiledHeaderOptions.Append(flags...)
	facet.PreprocessorOptions.Append(flags...)
	facet.CompilerOptions.Append(flags...)
}
func (facet *Facet) AddCompilationFlag_NoPreprocessor(flags ...string) {
	facet.PrecompiledHeaderOptions.Append(flags...)
	facet.CompilerOptions.Append(flags...)
}
func (facet *Facet) RemoveCompilationFlag(flags ...string) {
	facet.AnalysisOptions.Remove(flags...)
	facet.PrecompiledHeaderOptions.Remove(flags...)
	facet.PreprocessorOptions.Remove(flags...)
	facet.CompilerOptions.Remove(flags...)
}

func (facet *Facet) PerformSubstitutions() {
	if len(facet.Exports) > 0 {
		facet.Defines = utils.Map(facet.Exports.ExpandString, facet.Defines...)
		facet.ForceIncludes = utils.Map(facet.Exports.ExpandFilename, facet.ForceIncludes...)
		facet.IncludePaths = utils.Map(facet.Exports.ExpandDirectory, facet.IncludePaths...)
		facet.ExternIncludePaths = utils.Map(facet.Exports.ExpandDirectory, facet.ExternIncludePaths...)
		facet.SystemIncludePaths = utils.Map(facet.Exports.ExpandDirectory, facet.SystemIncludePaths...)
		facet.AnalysisOptions = utils.Map(facet.Exports.ExpandString, facet.AnalysisOptions...)
		facet.PreprocessorOptions = utils.Map(facet.Exports.ExpandString, facet.PreprocessorOptions...)
		facet.CompilerOptions = utils.Map(facet.Exports.ExpandString, facet.CompilerOptions...)
		facet.PrecompiledHeaderOptions = utils.Map(facet.Exports.ExpandString, facet.PrecompiledHeaderOptions...)
		facet.Libraries = utils.Map(facet.Exports.ExpandFilename, facet.Libraries...)
		facet.LibraryPaths = utils.Map(facet.Exports.ExpandDirectory, facet.LibraryPaths...)
		facet.LibrarianOptions = utils.Map(facet.Exports.ExpandString, facet.LibrarianOptions...)
		facet.LinkerOptions = utils.Map(facet.Exports.ExpandString, facet.LinkerOptions...)
	}
}

func (facet *Facet) String() string {
	return utils.PrettyPrint(facet)
}

func (vars *VariableSubstitutions) Add(key, value string) {
	(*vars)[key] = value
}
func (vars *VariableSubstitutions) Get(key string) string {
	if x, ok := (*vars)[key]; ok {
		return x
	} else {
		utils.LogPanic("unknown variable substitution: '%s' (got [%v])", key, strings.Join(utils.Keys((*vars)), ","))
		return ""
	}
}
func (vars *VariableSubstitutions) ExpandString(it string) string {
	if len(*vars) > 0 && strings.ContainsAny(it, "[[:") {
		for old, new := range *vars {
			it = strings.ReplaceAll(it, "[[:"+old+":]]", new)
		}
	}
	return it
}
func (vars *VariableSubstitutions) ExpandDirectory(it utils.Directory) (result utils.Directory) {
	result = make([]string, len(it))
	for i, x := range it {
		result[i] = vars.ExpandString(x)
	}
	return result
}
func (vars *VariableSubstitutions) ExpandFilename(it utils.Filename) utils.Filename {
	return utils.Filename{
		Dirname:  vars.ExpandDirectory(it.Dirname),
		Basename: vars.ExpandString(it.Basename),
	}
}
