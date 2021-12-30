package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"path"
)

type ModuleList []Module

func (list ModuleList) Len() int { return len(list) }
func (list ModuleList) Less(i, j int) bool {
	return list[i].GetModule().String() < list[j].GetModule().String()
}
func (list ModuleList) Swap(i, j int) { list[i], list[j] = list[j], list[i] }

func (list *ModuleList) Append(it ...Module) {
	*list = append(*list, it...)
}
func (list *ModuleList) AppendUniq(it ...Module) {
	*list = utils.AppendUniq(*list, it...)
}
func (list ModuleList) Range(each func(Module)) {
	for _, x := range list {
		each(x)
	}
}
func (list ModuleList) GetDigestable(o *bytes.Buffer) {
	utils.MakeDigestable(o, list...)
}

type ModuleSource struct {
	SourceDirs    utils.DirSet
	SourceGlobs   utils.StringSet
	ExcludedGlobs utils.StringSet
	SourceFiles   utils.FileSet
	ExcludedFiles utils.FileSet
	IsolatedFiles utils.FileSet
	ExtraFiles    utils.FileSet
}

func (x *ModuleSource) GetDigestable(o *bytes.Buffer) {
	x.SourceDirs.GetDigestable(o)
	x.SourceGlobs.GetDigestable(o)
	x.ExcludedGlobs.GetDigestable(o)
	x.SourceFiles.GetDigestable(o)
	x.ExcludedFiles.GetDigestable(o)
	x.IsolatedFiles.GetDigestable(o)
	x.ExtraFiles.GetDigestable(o)
}
func (x *ModuleSource) GetFileSet() (result utils.FileSet) {
	includeRE := utils.MakeGlobRegexp(x.SourceGlobs...)
	excludeRE := utils.MakeGlobRegexp(x.ExcludedGlobs...)

	for _, dir := range x.SourceDirs {
		dir.MatchFilesRec(func(f utils.Filename) error {
			result.Append(f)
			return nil
		}, includeRE)
		dir.MatchFilesRec(func(f utils.Filename) error {
			result.Remove(f)
			return nil
		}, excludeRE)
	}

	result.AppendUniq(x.SourceFiles...)
	result.AppendUniq(x.IsolatedFiles...)
	result.Remove(x.ExcludedFiles...)
	return result
}

type ModuleRules struct {
	ModuleName string
	Namespace  Namespace

	ModuleDir  utils.Directory
	ModuleType ModuleType

	CppRtti    CppRttiType
	CppStd     CppStdType
	Debug      DebugType
	Exceptions ExceptionType
	PCH        PrecompiledHeaderType
	Link       LinkType
	Sanitizer  SanitizerType
	Unity      UnityType

	PrecompiledHeader *utils.Filename
	PrecompiledSource *utils.Filename

	PublicDependencies  utils.StringSet
	PrivateDependencies utils.StringSet
	RuntimeDependencies utils.StringSet

	Generateds GeneratedList

	Facet
	Source ModuleSource
}

type Module interface {
	GetModule() *ModuleRules
	GetNamespace() *NamespaceRules
	utils.Digestable
	fmt.Stringer
}

func (rules *ModuleRules) String() string {
	return path.Join(rules.Path()...)
}

func (rules *ModuleRules) Path() []string {
	return append(rules.Namespace.GetNamespace().Path(), rules.ModuleName)
}

func (rules *ModuleRules) PublicDir() utils.Directory {
	return rules.ModuleDir.Folder("Public")
}
func (rules *ModuleRules) PrivateDir() utils.Directory {
	return rules.ModuleDir.Folder("Private")
}
func (rules *ModuleRules) GeneratedDir(env *CompileEnv) utils.Directory {
	return env.GeneratedDir().AbsoluteFolder(rules.ModuleDir.Relative(utils.UFS.Source))
}

func (rules *ModuleRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *ModuleRules) GetModule() *ModuleRules {
	return rules
}
func (rules *ModuleRules) GetNamespace() *NamespaceRules {
	return rules.Namespace.GetNamespace()
}

func (rules *ModuleRules) Decorate(env *CompileEnv, unit *Unit) {
	rules.Namespace.GetNamespace().Decorate(env, unit)

	unit.Transitive.ForceIncludes.Append(rules.ForceIncludes...)
	unit.Transitive.Libraries.Append(rules.Libraries...)
	unit.Transitive.LibraryPaths.Append(rules.LibraryPaths...)

	unit.IncludePaths.Append(rules.ModuleDir)
	if publicDir := rules.PublicDir(); publicDir.Exists() {
		unit.IncludePaths.Append(publicDir)
		unit.Transitive.IncludePaths.Append(publicDir)
	}
	if privateDir := rules.PrivateDir(); privateDir.Exists() {
		unit.IncludePaths.Append(privateDir)
	}

	generatedVis := MakeVisibilityMask()
	for _, gen := range rules.Generateds {
		generatedVis.Append(gen.GetGenerated().Visibility)
	}
	if generatedVis.Has(PUBLIC) {
		generatedPublicDir := unit.GeneratedDir.Folder("Public")
		unit.IncludePaths.Append(generatedPublicDir)
		unit.Transitive.IncludePaths.Append(generatedPublicDir)
	}
	if generatedVis.Has(PRIVATE) {
		unit.IncludePaths.Append(unit.GeneratedDir.Folder("Private"))
	}
}

func (rules *ModuleRules) GetDigestable(o *bytes.Buffer) {
	o.WriteString(rules.ModuleName)
	o.WriteString(rules.GetNamespace().String())
	rules.ModuleDir.GetDigestable(o)
	rules.ModuleType.GetDigestable(o)
	rules.CppRtti.GetDigestable(o)
	rules.CppStd.GetDigestable(o)
	rules.Debug.GetDigestable(o)
	rules.Exceptions.GetDigestable(o)
	rules.PCH.GetDigestable(o)
	rules.Link.GetDigestable(o)
	rules.Sanitizer.GetDigestable(o)
	rules.Unity.GetDigestable(o)
	if rules.PrecompiledHeader != nil {
		rules.PrecompiledHeader.GetDigestable(o)
	} else {
		o.WriteString("PrecompiledHeader")
	}
	if rules.PrecompiledSource != nil {
		rules.PrecompiledSource.GetDigestable(o)
	} else {
		o.WriteString("PrecompiledSource")
	}
	rules.PublicDependencies.GetDigestable(o)
	rules.PrivateDependencies.GetDigestable(o)
	rules.RuntimeDependencies.GetDigestable(o)
	rules.Generateds.GetDigestable(o)
	rules.Facet.GetDigestable(o)
	rules.Source.GetDigestable(o)
}

// func (rules *ModuleRules) Generate(vis VisibilityType, name string, gen GeneratorFunc) {
// 	rules.Generateds.Append(&GeneratedRules{
// 		GeneratedName: name,
// 		Visibility:    vis,
// 	})
// }
