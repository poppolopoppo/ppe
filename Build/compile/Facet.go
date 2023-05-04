package compile

import (
	utils "build/utils"
	"fmt"
	"strings"
)

type Facetable interface {
	GetFacet() *Facet
}

type FacetDecorator interface {
	Decorate(*CompileEnv, *Unit) error
}

type VariableDefinition struct {
	Name, Value string
}
type VariableSubstitutions []VariableDefinition

/***************************************
 * Facet
 ***************************************/

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
func (facet *Facet) Serialize(ar utils.Archive) {
	ar.Serializable(&facet.Defines)

	ar.Serializable(&facet.ForceIncludes)
	ar.Serializable(&facet.IncludePaths)
	ar.Serializable(&facet.ExternIncludePaths)
	ar.Serializable(&facet.SystemIncludePaths)

	ar.Serializable(&facet.AnalysisOptions)
	ar.Serializable(&facet.PreprocessorOptions)
	ar.Serializable(&facet.CompilerOptions)
	ar.Serializable(&facet.PrecompiledHeaderOptions)

	ar.Serializable(&facet.Libraries)
	ar.Serializable(&facet.LibraryPaths)

	ar.Serializable(&facet.LibrarianOptions)
	ar.Serializable(&facet.LinkerOptions)

	ar.Serializable(&facet.Tags)
	ar.Serializable(&facet.Exports)
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

func (facet *Facet) Tagged(tag TagType) bool {
	return facet.Tags.Has(tag)
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
		facet.Exports.Append(x.Exports)
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
		facet.Exports.Append(x.Exports)
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
		facet.Exports.Prepend(x.Exports)
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

/***************************************
 * Variable Substitutions
 ***************************************/

func (vars *VariableSubstitutions) Add(name, value string) {
	if i, ok := vars.IndexOf(name); ok {
		(*vars)[i].Value = value
	} else {
		*vars = append(*vars, VariableDefinition{
			Name: name, Value: value,
		})
	}
}
func (vars VariableSubstitutions) ExpandString(str string) string {
	if len(vars) > 0 && strings.ContainsAny(str, "[[:") {
		for _, it := range vars {
			str = strings.ReplaceAll(str, it.Name, it.Value)
		}
	}
	return str
}
func (vars VariableSubstitutions) ExpandDirectory(dir utils.Directory) (result utils.Directory) {
	if len(vars) > 0 {
		return utils.MakeDirectory(vars.ExpandString(dir.String()))
	} else {
		return dir
	}
}
func (vars VariableSubstitutions) ExpandFilename(it utils.Filename) utils.Filename {
	return utils.Filename{
		Dirname:  vars.ExpandDirectory(it.Dirname),
		Basename: vars.ExpandString(it.Basename),
	}
}
func (vars VariableSubstitutions) Get(from string) string {
	if i, ok := vars.IndexOf(from); ok {
		return vars[i].Value
	} else {
		utils.LogPanic("variable-substitutions: could not find [[:%s:]] in %v", from, vars)
		return ""
	}
}
func (vars VariableSubstitutions) IndexOf(from string) (int, bool) {
	for i, it := range vars {
		if it.Name == from {
			return i, true
		}
	}
	return len(vars), false
}
func (vars *VariableSubstitutions) Append(other VariableSubstitutions) {
	for _, it := range other {
		if _, ok := vars.IndexOf(it.Name); !ok {
			*vars = append(*vars, it)
		}
	}
}
func (vars *VariableSubstitutions) Prepend(other VariableSubstitutions) {
	for _, it := range other {
		vars.Add(it.Name, it.Value)
	}
}
func (vars *VariableSubstitutions) Serialize(ar utils.Archive) {
	utils.SerializeSlice(ar, (*[]VariableDefinition)(vars))
}

func (def VariableDefinition) String() string {
	return fmt.Sprint(def.Name, `=`, def.Value)
}
func (def *VariableDefinition) Set(in string) error {
	parsed := strings.SplitN(in, `=`, 2)
	if len(parsed) != 2 {
		return fmt.Errorf("invalid variable definition %q", in)
	}

	def.Name = parsed[0]
	def.Value = parsed[1]
	return nil
}
func (x VariableDefinition) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *VariableDefinition) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (def *VariableDefinition) Serialize(ar utils.Archive) {
	ar.String(&def.Name)
	ar.String(&def.Value)
}
