package compile

import (
	. "build/utils"
	"fmt"
	"io"
	"path"
	"strings"
)

const NAMESPACEDESC_EXT = "-namespace.json"
const MODULEDESC_EXT = "-module.json"

const PCH_DEFAULT_HEADER = "stdafx.h"
const PCH_DEFAULT_SOURCE = "stdafx.cpp"

/***************************************
 * Module Arche Type
 ***************************************/

var AllArchetypes SharedMapT[string, ModuleArchetype]

type ModuleArchetype func(*ModuleRules)

func RegisterArchetype(archtype string, fn ModuleArchetype) ModuleArchetype {
	archtype = strings.ToUpper(archtype)
	AllArchetypes.Add(archtype, fn)
	return fn
}

/***************************************
 * Module Extension Description
 ***************************************/

type ExtensionDesc struct {
	Archetypes      StringSet
	AllowedPlaforms SetT[PlatformAlias]
	HAL             map[HostId]*ModuleDesc
	TAG             map[TagFlags]*ModuleDesc
}

func (src ExtensionDesc) ApplyAllowedPlatforms(name fmt.Stringer) bool {
	if len(src.AllowedPlaforms) > 0 {
		localPlatform := GetLocalHostPlatformAlias()
		if src.AllowedPlaforms.Contains(localPlatform) {
			LogTrace("%v: not allowed on <%v> platform", name, localPlatform)
			return false
		}
	}
	return true
}

func (src ExtensionDesc) ApplyArchetypes(dst *ModuleDesc, name ModuleAlias) {
	src.Archetypes.Range(func(id string) {
		id = strings.ToUpper(id)
		if decorator, ok := AllArchetypes.Get(id); ok {
			LogTrace("%v: inherit module archtype <%v>", name, id)
			decorator(dst.rules)
		} else {
			LogFatal("%v: invalid module archtype <%v>", name, id)
		}
	})
}
func (src ExtensionDesc) ApplyHAL(dst *ModuleDesc, name ModuleAlias) {
	hostId := CurrentHost().Id
	for id, desc := range src.HAL {
		var hal HostId
		if err := hal.Set(id.String()); err == nil && hal == hostId {
			LogTrace("%v: inherit platform facet [%v]", name, id)
			dst.Archetypes.Prepend(desc.Archetypes...)
			dst.rules.Prepend(desc.rules)
		} else if err != nil {
			LogError("%v: invalid platform id [%v], %v", name, id, err)
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

/***************************************
 * Module Namespace Description
 ***************************************/

type NamespaceDesc struct {
	Name   string
	Source Filename

	Modules StringSet

	Parent   *NamespaceDesc
	Children StringSet

	Facet
	ExtensionDesc

	rules *NamespaceRules
}

func (x *NamespaceDesc) GetAbsoluteName() string {
	if x.Parent != nil {
		return path.Join(x.Parent.GetAbsoluteName(), x.Name)
	} else {
		return x.Name
	}
}

func (x *NamespaceDesc) Serialize(dst io.Writer) error {
	return JsonSerialize(x, dst)
}
func (x *NamespaceDesc) Deserialize(src io.Reader) error {
	x.Facet = NewFacet()
	return JsonDeserialize(x, src)
}

/***************************************
 * Module Description
 ***************************************/

type ModuleDesc struct {
	Name   string
	Source Filename

	Type ModuleType

	SourceDirs    StringSet
	SourceGlobs   StringSet
	ExcludedGlobs StringSet
	SourceFiles   StringSet
	ExcludedFiles StringSet
	ForceIncludes StringSet
	IsolatedFiles StringSet
	ExtraFiles    StringSet
	ExtraDirs     StringSet

	PrecompiledHeader *string
	PrecompiledSource *string

	PrivateDependencies ModuleAliases
	PublicDependencies  ModuleAliases
	RuntimeDependencies ModuleAliases

	Facet
	CppRules
	ExtensionDesc

	rules *ModuleRules
}

func (x *ModuleDesc) Serialize(dst io.Writer) error {
	return JsonSerialize(x, dst)
}
func (x *ModuleDesc) Deserialize(src io.Reader) error {
	x.Facet = NewFacet()
	return JsonDeserialize(x, src)
}
func (x *ModuleDesc) CreateRules(src Filename, namespace *NamespaceDesc, moduleBasename string) error {
	moduleAlias := ModuleAlias{
		NamespaceAlias: namespace.rules.NamespaceAlias,
		ModuleName:     moduleBasename,
	}
	LogVerbose("create rules for module: '%v'", moduleAlias)

	rootDir := src.Dirname
	x.rules = &ModuleRules{
		ModuleAlias: moduleAlias,
		ModuleDir:   rootDir,
		ModuleType:  x.Type,
		CppRules:    x.CppRules,
		Source: ModuleSource{
			SourceGlobs:   x.SourceGlobs,
			ExcludedGlobs: x.ExcludedGlobs,
			SourceDirs:    x.SourceDirs.ToDirSet(rootDir).Normalize(),
			SourceFiles:   x.SourceFiles.ToFileSet(rootDir).Normalize(),
			ExcludedFiles: x.ExcludedFiles.ToFileSet(rootDir).Normalize(),
			IsolatedFiles: x.IsolatedFiles.ToFileSet(rootDir).Normalize(),
			ExtraFiles:    x.ExtraFiles.ToFileSet(rootDir).Normalize(),
			ExtraDirs:     x.ExtraDirs.ToDirSet(rootDir).Normalize(),
		},
		PrivateDependencies: x.PrivateDependencies,
		PublicDependencies:  x.PublicDependencies,
		RuntimeDependencies: x.RuntimeDependencies,
		Facet:               x.Facet,
		PerTags:             map[TagFlags]ModuleRules{},
	}

	for key, desc := range x.HAL {
		if err := desc.CreateRules(src, namespace, path.Join(moduleBasename, key.String())); err != nil {
			return err
		}
	}

	for key, desc := range x.TAG {
		if err := desc.CreateRules(src, namespace, path.Join(moduleBasename, key.String())); err != nil {
			return err
		}
	}

	x.ApplyHAL(x, x.rules.ModuleAlias)

	for tags, desc := range x.TAG {
		x.rules.PerTags[tags] = *desc.rules
	}

	return nil
}

/***************************************
 * Build Modules Deserializer
 ***************************************/

type buildModulesDeserializer struct {
	namespaces []*NamespaceDesc
	modules    []*ModuleDesc
}

func (x *buildModulesDeserializer) loadModuleDesc(src Filename, namespace *NamespaceDesc) (*ModuleDesc, error) {
	LogTrace("loading data-driven module from '%v'", src)

	result := &ModuleDesc{
		Name:   strings.TrimSuffix(src.Basename, MODULEDESC_EXT),
		Source: src,
	}

	if err := UFS.OpenBuffered(src, result.Deserialize); err != nil {
		return nil, fmt.Errorf("%v: %v", src, err)
	}
	rootDir := src.Dirname

	if err := result.CreateRules(src, namespace, result.Name); err != nil {
		return nil, fmt.Errorf("%v: %v", src, err)
	}
	result.ApplyExtensions(&namespace.ExtensionDesc)

	result.rules.ForceIncludes.Append(result.ForceIncludes.ToFileSet(rootDir)...)
	result.rules.Source.ExtraFiles.AppendUniq(src)

	result.ApplyArchetypes(result, result.rules.ModuleAlias)

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

	if !result.ApplyAllowedPlatforms(result.rules.ModuleAlias) {
		return nil, nil
	}

	x.modules = append(x.modules, result)
	return result, nil
}

func (x *buildModulesDeserializer) loadNamespaceDesc(src Filename, parent *NamespaceDesc) (*NamespaceDesc, error) {
	LogTrace("loading data-driven namespace from '%v'", src)

	src = src.Normalize()

	result := &NamespaceDesc{
		Source: src,
	}

	if err := UFS.OpenBuffered(src, result.Deserialize); err != nil {
		return nil, fmt.Errorf("%v: %v", src, err)
	}

	if result.Name == "" {
		result.Name = strings.TrimSuffix(src.Basename, NAMESPACEDESC_EXT)
	}

	result.rules = &NamespaceRules{
		NamespaceAlias:    NamespaceAlias{NamespaceName: result.GetAbsoluteName()},
		NamespaceChildren: NewStringSet(),
		NamespaceDir:      src.Dirname,
		NamespaceModules:  ModuleAliases{},
		Facet:             result.Facet,
	}

	if parent != nil {
		result.rules.NamespaceParent = parent.rules.NamespaceAlias
		result.ApplyExtensions(&parent.ExtensionDesc)
	}

	if !result.ApplyAllowedPlatforms(result.rules.NamespaceAlias) {
		return nil, nil
	}

	x.namespaces = append(x.namespaces, result)

	for _, it := range result.Modules {
		f := result.rules.NamespaceDir.Folder(it).File(it + MODULEDESC_EXT).Normalize()
		if module, err := x.loadModuleDesc(f, result); err == nil {
			if module != nil { // check if module is allowed
				result.rules.NamespaceModules.Append(module.rules.ModuleAlias)
			}
		} else {
			return nil, err
		}
	}

	for _, it := range result.Children {
		f := result.rules.NamespaceDir.Folder(it).File(it + NAMESPACEDESC_EXT).Normalize()
		if namespace, err := x.loadNamespaceDesc(f, result); err == nil {
			if namespace != nil { // check if namespace is allowed
				result.rules.NamespaceChildren.Append(namespace.rules.String())
			}
		} else {
			return nil, err
		}
	}

	return result, nil
}

/***************************************
 * Build Modules
 ***************************************/

type BuildModules struct {
	Source     Filename
	Programs   ModuleAliases
	Modules    ModuleAliases
	Namespaces NamespaceAliases
	Root       Namespace
}

func (x BuildModules) Alias() BuildAlias {
	return MakeBuildAlias("Modules", x.Source.String())
}
func (x *BuildModules) Build(bc BuildContext) error {
	x.Programs = make(ModuleAliases, 0, len(x.Programs))
	x.Namespaces = make(NamespaceAliases, 0, len(x.Namespaces))
	x.Modules = make(ModuleAliases, 0, len(x.Modules))
	x.Root = nil

	deserializer := buildModulesDeserializer{}
	root, err := deserializer.loadNamespaceDesc(x.Source, nil)
	if err != nil {
		return err
	}

	x.Root = root.rules

	for _, it := range deserializer.namespaces {
		x.Namespaces.Append(it.rules.NamespaceAlias)

		if !x.Source.Equals(it.Source) {
			if err := bc.NeedFile(it.Source); err != nil {
				return err
			}
		}

		err := bc.OutputNode(WrapBuildFactory(func(bi BuildInitializer) (Namespace, error) {
			if err := bi.NeedFile(it.Source); err != nil {
				return nil, err
			}
			if it.Parent != nil {
				if err := bi.NeedBuildable(it.Parent.rules); err != nil {
					return nil, err
				}
			}
			return it.rules, nil
		}))
		if err != nil {
			return err
		}
	}

	for _, it := range deserializer.modules {
		x.Modules.Append(it.rules.ModuleAlias)

		if it.rules.ModuleType == MODULE_PROGRAM {
			x.Programs.Append(it.rules.ModuleAlias)
		}

		if err := bc.NeedFile(it.Source); err != nil {
			return err
		}

		err := bc.OutputNode(WrapBuildFactory(func(bi BuildInitializer) (Module, error) {
			if err := bi.NeedFile(it.Source); err != nil {
				return nil, err
			}
			if err := bi.NeedBuildable(it.rules.GetNamespace()); err != nil {
				return nil, err
			}
			return it.rules, nil
		}))
		if err != nil {
			return err
		}
	}

	// need a separated pass for validation, or not every module will be declared!
	for _, it := range deserializer.modules {
		LogPanicIfFailed(validateModuleRec(x, it.rules))
	}

	return nil
}
func (x *BuildModules) Serialize(ar Archive) {
	ar.Serializable(&x.Source)
	SerializeSlice(ar, x.Programs.Ref())
	SerializeSlice(ar, x.Modules.Ref())
	SerializeSlice(ar, x.Namespaces.Ref())
	SerializeExternal(ar, &x.Root)
}

func validateModuleDep(x *BuildModules, src ...ModuleAlias) error {
	for _, id := range src {
		if !x.Modules.Contains(id) {
			return fmt.Errorf("missing module: '%v'", id)
		}
	}
	return nil
}
func validateModuleRec(x *BuildModules, rules *ModuleRules) error {
	if err := validateModuleDep(x, rules.PrivateDependencies...); err != nil {
		return fmt.Errorf("private dep for '%v': %v", rules.String(), err)
	}
	if err := validateModuleDep(x, rules.PublicDependencies...); err != nil {
		return fmt.Errorf("public dep for '%v': %v", rules.String(), err)
	}
	if err := validateModuleDep(x, rules.RuntimeDependencies...); err != nil {
		return fmt.Errorf("runtime dep for '%v': %v", rules.String(), err)
	}

	for _, tagged := range rules.PerTags {
		if err := validateModuleRec(x, &tagged); err != nil {
			return err
		}
	}

	return nil
}

func GetBuildModules() BuildFactoryTyped[*BuildModules] {
	return MakeBuildFactory(func(bi BuildInitializer) (BuildModules, error) {
		source := CommandEnv.RootFile()
		return BuildModules{Source: source}, bi.NeedFile(source)
	})
}
