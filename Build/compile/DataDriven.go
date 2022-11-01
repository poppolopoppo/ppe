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

var AllArchetypes utils.SharedMapT[string, ModuleArchetype]

type ModuleArchetype func(*ModuleRules)

func RegisterArchetype(archtype string, fn ModuleArchetype) ModuleArchetype {
	archtype = strings.ToUpper(archtype)
	AllArchetypes.Add(archtype, fn)
	return fn
}

type ExtensionDesc struct {
	Archetypes utils.StringSet
	HAL        map[utils.HostId]*ModuleDesc
	TAG        map[TagFlags]*ModuleDesc
}

func (src ExtensionDesc) ApplyArchetypes(dst *ModuleDesc, name string) {
	src.Archetypes.Range(func(id string) {
		id = strings.ToUpper(id)
		if decorator, ok := AllArchetypes.Get(id); ok {
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
			dst.Archetypes.Prepend(desc.Archetypes...)
			dst.rules.Prepend(desc.rules)
		} else if err != nil {
			utils.LogError("%v: invalid platform id [%v], %v", name, id, err)
		}
	}
}
func (x *ExtensionDesc) ApplyExtensions(other *ExtensionDesc) {
	x.Archetypes.Prepend(other.Archetypes...)

	for key, src := range other.HAL {
		if dst, ok := x.HAL[key]; ok {
			dst.Archetypes.Prepend(src.Archetypes...)
			dst.rules.Append(src.rules)
		} else {
			new := &ModuleDesc{}
			*new = *src
			x.HAL[key] = new
		}
	}

	for key, src := range other.TAG {
		if dst, ok := x.TAG[key]; ok {
			dst.Archetypes.Prepend(src.Archetypes...)
			dst.rules.Append(src.rules)
		} else {
			new := &ModuleDesc{}
			*new = *src
			x.TAG[key] = new
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
	ExtraDirs     utils.StringSet

	PrecompiledHeader *string
	PrecompiledSource *string

	PrivateDependencies utils.StringSet
	PublicDependencies  utils.StringSet
	RuntimeDependencies utils.StringSet

	Facet
	ExtensionDesc

	rules *ModuleRules
}

func (x *ModuleDesc) Serialize(dst io.Writer) error {
	return utils.JsonSerialize(x, dst)
}
func (x *ModuleDesc) Deserialize(src io.Reader) error {
	x.Facet = NewFacet()
	return utils.JsonDeserialize(x, src)
}
func (x *ModuleDesc) CreateRules(rootDir utils.Directory, namespace *NamespaceDesc, moduleId string) error {
	utils.LogVerbose("create rules for module: '%v'", moduleId)

	x.rules = &ModuleRules{
		ModuleName: x.Name,
		Namespace:  namespace.rules,
		ModuleDir:  rootDir,
		ModuleType: x.Type,
		CppRtti:    x.CppRtti,
		CppStd:     x.CppStd,
		Debug:      x.Debug,
		PCH:        x.PCH,
		Link:       x.Link,
		Sanitizer:  x.Sanitizer,
		Unity:      x.Unity,
		Source: ModuleSource{
			SourceGlobs:   x.SourceGlobs,
			ExcludedGlobs: x.ExcludedGlobs,
			SourceDirs:    x.SourceDirs.ToDirSet(rootDir),
			SourceFiles:   x.SourceFiles.ToFileSet(rootDir),
			ExcludedFiles: x.ExcludedFiles.ToFileSet(rootDir),
			IsolatedFiles: x.IsolatedFiles.ToFileSet(rootDir),
			ExtraFiles:    x.ExtraFiles.ToFileSet(rootDir),
			ExtraDirs:     x.ExtraDirs.ToDirSet(rootDir),
		},
		PrivateDependencies: x.PrivateDependencies,
		PublicDependencies:  x.PublicDependencies,
		RuntimeDependencies: x.RuntimeDependencies,
		Facet:               x.Facet,
		PerTags:             map[TagFlags]*ModuleRules{},
	}

	for _, desc := range x.HAL {
		if err := desc.CreateRules(rootDir, namespace, moduleId); err != nil {
			return err
		}
	}

	for _, desc := range x.TAG {
		if err := desc.CreateRules(rootDir, namespace, moduleId); err != nil {
			return err
		}
	}

	x.ApplyHAL(x, moduleId)

	for tags, desc := range x.TAG {
		x.rules.PerTags[tags] = desc.rules
	}

	return nil
}

func loadModuleDesc(src utils.Filename, namespace *NamespaceDesc) (*ModuleDesc, error) {
	utils.LogTrace("loading data-driven module from '%v'", src)

	result := &ModuleDesc{
		Name: strings.TrimSuffix(src.Basename, MODULEDESC_EXT),
	}

	if err := utils.UFS.Open(src, result.Deserialize); err == nil {
		moduleId := path.Join(namespace.Name, result.Name)
		rootDir := src.Dirname

		if err = result.CreateRules(rootDir, namespace, moduleId); err == nil {
			result.ApplyExtensions(&namespace.ExtensionDesc)

			result.rules.ForceIncludes.Append(result.ForceIncludes.ToFileSet(rootDir)...)
			result.rules.Source.ExtraFiles.AppendUniq(src)

			result.ApplyArchetypes(result, moduleId)

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
		}

		return result, err
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
			desc.ApplyExtensions(&parent.ExtensionDesc)
		}

		utils.LogVerbose("parsed new namespace: '%v'", desc.rules)

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
func validateModuleRec(x *BuildModulesT, rules *ModuleRules) error {
	if err := validateModuleDep(x, rules.PrivateDependencies); err != nil {
		return fmt.Errorf("private dep for '%v': %v", rules.String(), err)
	}
	if err := validateModuleDep(x, rules.PublicDependencies); err != nil {
		return fmt.Errorf("public dep for '%v': %v", rules.String(), err)
	}
	if err := validateModuleDep(x, rules.RuntimeDependencies); err != nil {
		return fmt.Errorf("runtime dep for '%v': %v", rules.String(), err)
	}

	for _, tagged := range rules.PerTags {
		if err := validateModuleRec(x, tagged); err != nil {
			return err
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

func (x *BuildModulesT) Get(moduleAlias utils.BuildAlias) Module {
	return x.Modules[moduleAlias.String()]
}
func (x *BuildModulesT) ModuleKeys() []string {
	result := utils.Keys(x.Modules)
	sort.Strings(result)
	return result
}
func (x *BuildModulesT) NamespaceKeys() []string {
	result := utils.Keys(x.Namespaces)
	sort.Strings(result)
	return result
}

func (x *BuildModulesT) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Data", "BuildModules")
}
func (x *BuildModulesT) Build(ctx utils.BuildContext) (utils.BuildStamp, error) {
	x.Namespaces = map[string]Namespace{}
	x.Modules = map[string]Module{}
	x.Root = nil
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
		rules := module.GetModule(nil)
		validateModuleRec(x, rules)
	}

	return utils.MakeBuildStamp(moduleRules)
}

var BuildModules = utils.MakeBuildable(func(utils.BuildInit) *BuildModulesT {
	result := &BuildModulesT{
		Source:     utils.CommandEnv.RootFile(),
		Modules:    map[string]Module{},
		Namespaces: map[string]Namespace{},
	}
	return result
})
