package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"path"
	"strings"
)

type ModuleAlias struct {
	NamespaceName string
	ModuleName    string
}

func NewModuleAlias(module Module) ModuleAlias {
	return ModuleAlias{
		NamespaceName: path.Join(module.GetNamespace().Path()...),
		ModuleName:    module.GetModule().ModuleName,
	}
}
func (x ModuleAlias) Alias() utils.BuildAlias {
	return utils.BuildAlias(path.Join(x.NamespaceName, x.ModuleName))
}
func (x ModuleAlias) GetModuleAlias() utils.BuildAlias {
	return x.Alias()
}
func (x ModuleAlias) GetNamespaceAlias() utils.BuildAlias {
	return utils.BuildAlias(x.NamespaceName)
}
func (x ModuleAlias) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.NamespaceName)
	o.WriteString(x.ModuleName)
}
func (x ModuleAlias) Compare(o ModuleAlias) int {
	if x.NamespaceName == o.NamespaceName {
		return strings.Compare(x.ModuleName, o.ModuleName)
	} else {
		return strings.Compare(x.NamespaceName, o.NamespaceName)
	}
}
func (x ModuleAlias) String() string {
	return x.Alias().String()
}

type ModuleList []Module

func (list ModuleList) Len() int { return len(list) }
func (list ModuleList) Less(i, j int) bool {
	return list[i].GetModule().ModuleAlias().Compare(list[j].GetModule().ModuleAlias()) < 0
}
func (list ModuleList) Swap(i, j int) { list[i], list[j] = list[j], list[i] }

func (list *ModuleList) Append(it ...Module) {
	*list = append(*list, it...)
}
func (list *ModuleList) AppendUniq(it ...Module) {
	for _, x := range it {
		if _, found := utils.IndexIf(func(m Module) bool {
			return x.GetModule() == m.GetModule()
		}, *list...); !found {
			list.Append(x)
		}
	}
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
	ExtraDirs     utils.DirSet
}

func (x *ModuleSource) GetDigestable(o *bytes.Buffer) {
	x.SourceDirs.GetDigestable(o)
	x.SourceGlobs.GetDigestable(o)
	x.ExcludedGlobs.GetDigestable(o)
	x.SourceFiles.GetDigestable(o)
	x.ExcludedFiles.GetDigestable(o)
	x.IsolatedFiles.GetDigestable(o)
	x.ExtraFiles.GetDigestable(o)
	x.ExtraDirs.GetDigestable(o)
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

	// voluntary ignore ExtraDirs/ExtraFiles here
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

	Customs    CustomList
	Generateds GeneratedList

	Facet
	Source ModuleSource
}

type Module interface {
	ModuleAlias() ModuleAlias
	GetModule() *ModuleRules
	GetNamespace() *NamespaceRules
	utils.Digestable
	fmt.Stringer
}

func (rules *ModuleRules) ModuleAlias() ModuleAlias {
	return NewModuleAlias(rules)
}
func (rules *ModuleRules) String() string {
	return rules.ModuleAlias().Alias().String()
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

	unit.TransitiveFacet.ForceIncludes.Append(rules.ForceIncludes...)
	unit.TransitiveFacet.Libraries.Append(rules.Libraries...)
	unit.TransitiveFacet.LibraryPaths.Append(rules.LibraryPaths...)

	unit.IncludePaths.Append(rules.ModuleDir)
	if publicDir := rules.PublicDir(); publicDir.Exists() {
		unit.IncludePaths.Append(publicDir)
		unit.TransitiveFacet.IncludePaths.Append(publicDir)
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
		unit.TransitiveFacet.IncludePaths.Append(generatedPublicDir)
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

func (rules *ModuleRules) Generate(vis VisibilityType, name string, gen Generator) {
	rules.Generateds.Append(&GeneratedRules{
		GeneratedName: name,
		Visibility:    vis,
		Generator:     gen,
	})
}
