package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"path"
	"sort"
	"strings"
)

type ModuleAlias struct {
	NamespaceName string
	ModuleName    string
}

func NewModuleAlias(module Module) ModuleAlias {
	return ModuleAlias{
		NamespaceName: utils.InternString(path.Join(module.GetNamespace().Path()...)),
		ModuleName:    module.GetModule(nil).ModuleName,
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
	o.WriteByte(0)
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
func (x *ModuleAlias) Set(in string) (err error) {
	if parts := utils.SplitPath(in); parts != nil && len(parts) > 1 {
		x.ModuleName = utils.InternString(parts[len(parts)-1])
		x.NamespaceName = utils.InternString(path.Join(parts[0 : len(parts)-1]...))
		return nil
	}
	return fmt.Errorf("malformed ModuleAlias: '%s'", in)
}
func (x ModuleAlias) MarshalText() ([]byte, error) {
	return []byte(x.NamespaceName + "/" + x.ModuleName), nil
}
func (x *ModuleAlias) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

type ModuleList []Module

func (list ModuleList) Len() int { return len(list) }
func (list ModuleList) Less(i, j int) bool {
	return list[i].GetModule(nil).ModuleAlias().Compare(list[j].GetModule(nil).ModuleAlias()) < 0
}
func (list ModuleList) Swap(i, j int) { list[i], list[j] = list[j], list[i] }

func (list *ModuleList) Append(it ...Module) {
	*list = append(*list, it...)
}
func (list *ModuleList) AppendUniq(it ...Module) {
	for _, x := range it {
		if _, found := utils.IndexIf(func(m Module) bool {
			return x.GetModule(nil) == m.GetModule(nil)
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

func (x *ModuleSource) Append(o ModuleSource) {
	x.SourceDirs.Append(o.SourceDirs...)
	x.SourceGlobs.Append(o.SourceGlobs...)
	x.ExcludedGlobs.Append(o.ExcludedGlobs...)
	x.SourceFiles.Append(o.SourceFiles...)
	x.ExcludedFiles.Append(o.ExcludedFiles...)
	x.IsolatedFiles.Append(o.IsolatedFiles...)
	x.ExtraFiles.Append(o.ExtraFiles...)
	x.ExtraDirs.Append(o.ExtraDirs...)
}
func (x *ModuleSource) Prepend(o ModuleSource) {
	x.SourceDirs.Prepend(o.SourceDirs...)
	x.SourceGlobs.Prepend(o.SourceGlobs...)
	x.ExcludedGlobs.Prepend(o.ExcludedGlobs...)
	x.SourceFiles.Prepend(o.SourceFiles...)
	x.ExcludedFiles.Prepend(o.ExcludedFiles...)
	x.IsolatedFiles.Prepend(o.IsolatedFiles...)
	x.ExtraFiles.Prepend(o.ExtraFiles...)
	x.ExtraDirs.Prepend(o.ExtraDirs...)
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

type ModuleAliases = utils.SetT[ModuleAlias]

type ModuleRules struct {
	ModuleName string
	Namespace  Namespace

	ModuleDir  utils.Directory
	ModuleType ModuleType

	CppRules

	PrecompiledHeader *utils.Filename
	PrecompiledSource *utils.Filename

	PublicDependencies  ModuleAliases
	PrivateDependencies ModuleAliases
	RuntimeDependencies ModuleAliases

	Customs    CustomList
	Generateds GeneratedList

	Facet
	Source ModuleSource

	PerTags map[TagFlags]*ModuleRules
}

type Module interface {
	ModuleAlias() ModuleAlias
	GetModule(env *CompileEnv) *ModuleRules
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

func (rules *ModuleRules) GetCpp() *CppRules {
	return rules.CppRules.GetCpp()
}
func (rules *ModuleRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *ModuleRules) expandTagsRec(env *CompileEnv, dst *ModuleRules) {
	for tags, tagged := range rules.PerTags {
		if selectedTags := env.Tags.Intersect(tags); !selectedTags.Empty() {
			utils.LogVeryVerbose("expand module <%v> with rules tagged [%v]", dst.ModuleName, selectedTags)
			dst.Prepend(tagged)
			tagged.expandTagsRec(env, dst)
		}
	}
}
func (rules *ModuleRules) GetModule(env *CompileEnv) *ModuleRules {
	// we use this getter to create new rules and apply PerTags properties
	if env != nil && len(rules.PerTags) > 0 {
		// make a copy of the current rules
		custom := &ModuleRules{}
		*custom = *rules
		// apply tags matching compile env recursively
		rules.expandTagsRec(env, custom)
		return custom
	}
	// nothing todo: just return the original rules
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
	rules.CppRules.GetDigestable(o)
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
	utils.MakeDigestable(o, rules.PublicDependencies...)
	utils.MakeDigestable(o, rules.PrivateDependencies...)
	utils.MakeDigestable(o, rules.RuntimeDependencies...)
	rules.Generateds.GetDigestable(o)
	rules.Facet.GetDigestable(o)
	rules.Source.GetDigestable(o)

	// sort for determinisn, since map[] order is random
	sortedTags := utils.Keys(rules.PerTags)
	sort.SliceStable(sortedTags, func(i, j int) bool {
		return sortedTags[i].int32() < int32(sortedTags[j].int32())
	})
	for _, tags := range sortedTags {
		tags.GetDigestable(o)
		rules.PerTags[tags].GetDigestable(o)
	}
}

func (rules *ModuleRules) Generate(vis VisibilityType, name string, gen Generator) {
	rules.Generateds.Append(&GeneratedRules{
		GeneratedName: name,
		Visibility:    vis,
		Generator:     gen,
	})
}

func (x *ModuleRules) Append(other *ModuleRules) {
	x.CppRules.Inherit(other.GetCpp())

	x.ForceIncludes.Append(other.ForceIncludes...)

	x.Source.Append(other.Source)

	if x.PrecompiledHeader == nil {
		x.PrecompiledHeader = other.PrecompiledHeader
	}
	if x.PrecompiledSource == nil {
		x.PrecompiledSource = other.PrecompiledSource
	}

	x.PrivateDependencies.Append(other.PrivateDependencies...)
	x.PublicDependencies.Append(other.PublicDependencies...)
	x.RuntimeDependencies.Append(other.RuntimeDependencies...)

	x.Customs.Append(other.Customs...)
	x.Generateds.Append(other.Generateds...)

	x.Facet.Append(other)
}
func (x *ModuleRules) Prepend(other *ModuleRules) {
	x.Overwrite(other.GetCpp())

	x.ForceIncludes.Prepend(other.ForceIncludes...)

	x.Source.Prepend(other.Source)

	if other.PrecompiledHeader != nil {
		x.PrecompiledHeader = other.PrecompiledHeader
	}
	if other.PrecompiledSource != nil {
		x.PrecompiledSource = other.PrecompiledSource
	}

	x.PrivateDependencies.Prepend(other.PrivateDependencies...)
	x.PublicDependencies.Prepend(other.PublicDependencies...)
	x.RuntimeDependencies.Prepend(other.RuntimeDependencies...)

	x.Customs.Prepend(other.Customs...)
	x.Generateds.Prepend(other.Generateds...)

	x.Facet.Prepend(other)
}
