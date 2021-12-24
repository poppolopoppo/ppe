package compile

import (
	"build/utils"
	"fmt"
	"io"
	"path"
	"sort"
	"strings"
)

const NAMESPACEDESC_EXT = "-namespace.json"
const MODULEDESC_EXT = "-module.json"

const PCH_DEFAULT_HEADER = "stdafx.h"
const PCH_DEFAULT_SOURCE = "stdafx.cpp"

var AllArchtypes utils.SharedMapT[string, ModuleArchtype]

type ModuleArchtype func(*ModuleRules)

func RegisterArchtype(archtype string, fn ModuleArchtype) ModuleArchtype {
	archtype = strings.ToUpper(archtype)
	AllArchtypes.Add(archtype, fn)
	return fn
}

type ExtensionDesc struct {
	Archtypes utils.StringSet
	HAL       map[utils.HostId]*ModuleDesc
}

func (src ExtensionDesc) ApplyArchtypes(dst *ModuleDesc, name string) {
	src.Archtypes.Range(func(id string) {
		id = strings.ToUpper(id)
		if decorator, ok := AllArchtypes.Get(id); ok {
			utils.LogTrace("%v: inherit module archtype <%v>", name, id)
			decorator(dst.rules)
		} else {
			utils.LogFatal("%v: invalid module archtype <%v>", name, id)
		}
	})
}
func (src ExtensionDesc) ApplyHAL(dst *ModuleDesc, name string) {
	hostId := utils.CurrentHost().Id
	for id, desc := range src.HAL {
		var hal utils.HostId
		if err := hal.Set(id.String()); err == nil && hal == hostId {
			utils.LogTrace("%v: inherit platform facet [%v]", name, id)
			dst.Append(desc)
		}
	}
}
func (x *ExtensionDesc) ExtendDesc(other *ExtensionDesc) {
	x.Archtypes.Prepend(other.Archtypes...)
	for key, src := range other.HAL {
		if dst, ok := x.HAL[key]; ok {
			dst.Append(src)
		} else {
			new := &ModuleDesc{}
			*new = *src
			x.HAL[key] = new
		}
	}
}

type NamespaceDesc struct {
	Name     string
	Modules  utils.StringSet
	Children utils.StringSet

	Facet
	ExtensionDesc

	rules *NamespaceRules
}

func (x *NamespaceDesc) Serialize(dst io.Writer) error {
	return utils.JsonSerialize(x, dst)
}
func (x *NamespaceDesc) Deserialize(src io.Reader) error {
	x.Facet = NewFacet()
	return utils.JsonDeserialize(x, src)
}

type ModuleDesc struct {
	Name string

	CppRtti   CppRttiType
	CppStd    CppStdType
	Debug     DebugType
	PCH       PrecompiledHeaderType
	Link      LinkType
	Sanitizer SanitizerType
	Type      ModuleType
	Unity     UnityType

	SourceDirs    utils.StringSet
	SourceGlobs   utils.StringSet
	ExcludedGlobs utils.StringSet
	SourceFiles   utils.StringSet
	ExcludedFiles utils.StringSet
	ForceIncludes utils.StringSet
	IsolatedFiles utils.StringSet
	ExtraFiles    utils.StringSet

	PrecompiledHeader *string
	PrecompiledSource *string

	PrivateDependencies utils.StringSet
	PublicDependencies  utils.StringSet
	RuntimeDependencies utils.StringSet

	Facet
	ExtensionDesc

	rules *ModuleRules
}

func (x *ModuleDesc) Append(other *ModuleDesc) {
	if x.CppRtti == CPPRTTI_INHERIT {
		x.CppRtti = other.CppRtti
	}
	if x.CppStd == CPPSTD_INHERIT {
		x.CppStd = other.CppStd
	}
	if x.Debug == DEBUG_INHERIT {
		x.Debug = other.Debug
	}
	if x.PCH == PCH_INHERIT {
		x.PCH = other.PCH
	}
	if x.Link == LINK_INHERIT {
		x.Link = other.Link
	}
	if x.Sanitizer == SANITIZER_INHERIT {
		x.Sanitizer = other.Sanitizer
	}
	if x.Unity == UNITY_INHERIT {
		x.Unity = other.Unity
	}

	x.SourceDirs.Append(other.SourceDirs...)
	x.SourceGlobs.Append(other.SourceGlobs...)
	x.ExcludedGlobs.Append(other.ExcludedGlobs...)
	x.SourceFiles.Append(other.SourceFiles...)
	x.ExcludedFiles.Append(other.ExcludedFiles...)
	x.ForceIncludes.Append(other.ForceIncludes...)
	x.IsolatedFiles.Append(other.IsolatedFiles...)
	x.ExtraFiles.Append(other.ExtraFiles...)

	if x.PrecompiledHeader == nil {
		x.PrecompiledHeader = other.PrecompiledHeader
	}
	if x.PrecompiledSource == nil {
		x.PrecompiledSource = other.PrecompiledSource
	}

	x.PrivateDependencies.Append(other.PrivateDependencies...)
	x.PublicDependencies.Append(other.PublicDependencies...)
	x.RuntimeDependencies.Append(other.RuntimeDependencies...)

	x.Archtypes = append(x.Archtypes, other.Archtypes...)

	x.Facet.Append(other)
}
func (x *ModuleDesc) Serialize(dst io.Writer) error {
	return utils.JsonSerialize(x, dst)
}
func (x *ModuleDesc) Deserialize(src io.Reader) error {
	x.Facet = NewFacet()
	return utils.JsonDeserialize(x, src)
}

func loadModuleDesc(src utils.Filename, namespace *NamespaceDesc) (result *ModuleDesc, err error) {
	utils.LogTrace("loading data-driven module from '%v'", src)

	result = &ModuleDesc{
		Name: strings.TrimSuffix(src.Basename, MODULEDESC_EXT),
	}

	if err := utils.UFS.Open(src, result.Deserialize); err == nil {
		moduleId := path.Join(namespace.Name, result.Name)
		utils.LogVerbose("parsed new module: '%v'", moduleId)

		result.ExtendDesc(&namespace.ExtensionDesc)
		result.ApplyHAL(result, moduleId)

		rootDir := src.Dirname
		result.rules = &ModuleRules{
			ModuleName: result.Name,
			Namespace:  namespace.rules,
			ModuleDir:  rootDir,
			ModuleType: result.Type,
			CppRtti:    result.CppRtti,
			CppStd:     result.CppStd,
			Debug:      result.Debug,
			PCH:        result.PCH,
			Link:       result.Link,
			Sanitizer:  result.Sanitizer,
			Unity:      result.Unity,
			Source: ModuleSource{
				SourceGlobs:   result.SourceGlobs,
				ExcludedGlobs: result.ExcludedGlobs,
				SourceDirs:    result.SourceDirs.ToDirSet(rootDir),
				SourceFiles:   result.SourceFiles.ToFileSet(rootDir),
				ExcludedFiles: result.ExcludedFiles.ToFileSet(rootDir),
				IsolatedFiles: result.IsolatedFiles.ToFileSet(rootDir),
				ExtraFiles:    result.ExtraFiles.ToFileSet(rootDir),
			},
			PrivateDependencies: result.PrivateDependencies,
			PublicDependencies:  result.PublicDependencies,
			RuntimeDependencies: result.RuntimeDependencies,
			Facet:               result.Facet,
		}

		result.rules.ForceIncludes.Append(result.ForceIncludes.ToFileSet(rootDir)...)

		result.ApplyArchtypes(result, moduleId)

		if result.PrecompiledHeader != nil {
			f := rootDir.AbsoluteFile(*result.PrecompiledHeader).Normalize()
			result.rules.PrecompiledHeader = &f
		} else if f := rootDir.File(PCH_DEFAULT_HEADER); f.Exists() {
			result.rules.PrecompiledHeader = &f
		}
		if result.PrecompiledSource != nil {
			f := rootDir.AbsoluteFile(*result.PrecompiledSource).Normalize()
			result.rules.PrecompiledSource = &f
		} else if f := rootDir.File(PCH_DEFAULT_SOURCE); f.Exists() {
			result.rules.PrecompiledSource = &f
		}

		return result, nil
	} else {
		return nil, err
	}
}

func loadNamespaceDesc(
	ctx utils.BuildContext,
	src utils.Filename,
	parent *NamespaceDesc,
	modules *[]*ModuleDesc) (desc *NamespaceDesc, err error) {
	utils.LogTrace("loading data-driven namespace from '%v'", src)

	desc = &NamespaceDesc{}

	if err := utils.UFS.Open(src, desc.Deserialize); err == nil {
		if desc.Name == "" {
			desc.Name = strings.TrimSuffix(src.Basename, NAMESPACEDESC_EXT)
		}

		desc.rules = &NamespaceRules{
			NamespaceName:     desc.Name,
			NamespaceParent:   nil,
			NamespaceChildren: utils.NewStringSet(),
			NamespaceDir:      src.Dirname,
			NamespaceModules:  utils.NewStringSet(),
			Facet:             desc.Facet,
		}

		if parent != nil {
			desc.rules.NamespaceParent = parent.rules
			desc.ExtendDesc(&parent.ExtensionDesc)
		}

		utils.LogVerbose("parsed new namespace: '%v'", desc.rules.Alias())

		for _, x := range desc.Modules {
			f := desc.rules.NamespaceDir.Folder(x).File(x + MODULEDESC_EXT)
			if module, err := loadModuleDesc(f, desc); err == nil {
				ctx.NeedFile(f)
				desc.rules.NamespaceModules.Append(module.rules.String())
				*modules = append(*modules, module)
			} else {
				return nil, err
			}
		}

		for _, x := range desc.Children {
			f := desc.rules.NamespaceDir.Folder(x).File(x + NAMESPACEDESC_EXT)
			if namespace, err := loadNamespaceDesc(ctx, f, desc, modules); err == nil {
				ctx.NeedFile(f)
				desc.rules.NamespaceChildren = append(desc.rules.NamespaceChildren, namespace.rules.String())
			} else {
				return nil, err
			}
		}

		return desc, nil
	} else {
		return nil, err
	}
}

func validateModuleDep(x *BuildModulesT, src utils.StringSet) error {
	for _, id := range src {
		if _, ok := x.Modules[id]; !ok {
			return fmt.Errorf("missing module: '%v'", id)
		}
	}
	return nil
}

type BuildModulesT struct {
	Source     utils.Filename
	Modules    map[string]Module
	Namespaces map[string]Namespace
	Root       Namespace
}

func (x *BuildModulesT) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Data", "BuildModules")
}
func (x *BuildModulesT) Build(ctx utils.BuildContext) (utils.BuildStamp, error) {
	ctx.NeedFile(x.Source)

	var moduleDescs []*ModuleDesc
	if root, err := loadNamespaceDesc(ctx, x.Source, nil, &moduleDescs); err != nil {
		return utils.BuildStamp{}, err
	} else {
		x.Root = root.rules
	}

	var moduleRules ModuleList
	for _, desc := range moduleDescs {
		moduleRules.Append(desc.rules)
		x.Modules[desc.rules.String()] = desc.rules

		namespace := desc.rules.GetNamespace()
		x.Namespaces[namespace.String()] = namespace
	}

	sort.Sort(moduleRules)

	for _, module := range moduleRules {
		rules := module.GetModule()
		if err := validateModuleDep(x, rules.PrivateDependencies); err != nil {
			return utils.BuildStamp{}, fmt.Errorf("private dep for '%v': %v", rules.Alias(), err)
		}
		if err := validateModuleDep(x, rules.PublicDependencies); err != nil {
			return utils.BuildStamp{}, fmt.Errorf("public dep for '%v': %v", rules.Alias(), err)
		}
		if err := validateModuleDep(x, rules.RuntimeDependencies); err != nil {
			return utils.BuildStamp{}, fmt.Errorf("runtime dep for '%v': %v", rules.Alias(), err)
		}
	}

	return utils.MakeBuildStamp(moduleRules)
}

var BuildModules = utils.MakeBuildable(func(bi utils.BuildInit) *BuildModulesT {
	result := &BuildModulesT{
		Source:     utils.CommandEnv.RootFile(),
		Modules:    map[string]Module{},
		Namespaces: map[string]Namespace{},
	}
	return result
})
