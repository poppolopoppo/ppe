package compile

import (
	"build/utils"
	"fmt"
	"math"
	"sort"
)

/***************************************
 * Determinist sort of modules by dependencies and name
 ***************************************/

type ModuleDependency struct {
	Module
	Visibility VisibilityType
}
type ModuleNode struct {
	Ordinal int // Module deterministic sort order

	Dependencies []ModuleDependency

	Files     func() utils.FileSet
	TotalSize func() uint32
	Unity     func(int) UnityType
}

func (x *ModuleNode) Private(each func(Module)) int {
	return x.Range(func(m Module, _ VisibilityType) {
		each(m)
	}, MakeVisibilityMask(PRIVATE))
}
func (x *ModuleNode) Public(each func(Module)) int {
	return x.Range(func(m Module, _ VisibilityType) {
		each(m)
	}, MakeVisibilityMask(PUBLIC))
}
func (x *ModuleNode) Runtime(each func(Module)) int {
	return x.Range(func(m Module, _ VisibilityType) {
		each(m)
	}, MakeVisibilityMask(RUNTIME))
}
func (x *ModuleNode) Range(each func(Module, VisibilityType), mask VisibilityMask) (count int) {
	for _, dep := range x.Dependencies {
		if mask.Has(dep.Visibility) {
			count += 1
			each(dep.Module, dep.Visibility)
		}
	}
	return count
}

func (x *ModuleNode) addPrivate(module Module) {
	x.append(module, PRIVATE)
}
func (x *ModuleNode) addPublic(module Module) {
	x.append(module, PUBLIC)
}
func (x *ModuleNode) addRuntime(module Module) {
	x.append(module, RUNTIME)
}
func (x *ModuleNode) append(module Module, vis VisibilityType) bool {
	for i, it := range x.Dependencies {
		if it.Module == module {
			if it.Visibility < vis {
				x.Dependencies[i].Visibility = vis
			}
			return false
		}
	}
	x.Dependencies = append(x.Dependencies, ModuleDependency{
		Module:     module,
		Visibility: vis,
	})
	return true
}
func (x *ModuleNode) SortDependencies(graph *ModuleGraph) {
	sort.Slice(x.Dependencies, func(i, j int) bool {
		lhs := x.Dependencies[i].Module
		rhs := x.Dependencies[j].Module
		return (graph.Get(lhs).Ordinal < graph.Get(rhs).Ordinal)
	})
}

type ModuleGraph struct {
	keys    []Module
	modules map[string]Module
	nodes   map[Module]*ModuleNode
}

func (graph *ModuleGraph) Keys() (keys []Module) {
	return graph.keys
}
func (graph *ModuleGraph) Module(name utils.BuildAlias) Module {
	if module, ok := graph.modules[name.String()]; ok {
		return module
	} else {
		panic(fmt.Errorf("unknown module name '%v'", name))
	}
}
func (graph *ModuleGraph) Get(module Module) *ModuleNode {
	if node, ok := graph.nodes[module]; ok {
		return node
	} else {
		panic(fmt.Errorf("module node not constructed for <%v>", module))
	}
}

func (graph *ModuleGraph) expandDependencies(deps ...string) []*ModuleNode {
	return utils.Map(func(name string) *ModuleNode {
		if module, ok := graph.modules[name]; ok {
			return graph.expandModule(module)
		} else {
			panic(fmt.Errorf("module graph: can't find module dependency <%v>", name))
		}
	}, deps...)
}
func (graph *ModuleGraph) expandModule(module Module) *ModuleNode {
	if node, ok := graph.nodes[module]; ok {
		return node
	} else {
		rules := module.GetModule()

		private := graph.expandDependencies(rules.PrivateDependencies...)
		public := graph.expandDependencies(rules.PublicDependencies...)
		runtime := graph.expandDependencies(rules.RuntimeDependencies...)

		node = &ModuleNode{
			Ordinal:      len(graph.nodes), // only works if the node is created *after* its dependencies
			Dependencies: []ModuleDependency{},
			Files:        utils.Memoize(rules.Source.GetFileSet),
			TotalSize: utils.Memoize(func() uint32 {
				return uint32(node.Files().TotalSize())
			}),
			Unity: utils.MemoizeArg(func(sizePerUnity int) UnityType {
				totalSize := float64(node.TotalSize())
				numUnityFiles := totalSize / float64(sizePerUnity)
				result := UnityType(int32(math.Ceil(numUnityFiles)))
				utils.LogTrace("%v: %d unity files (%.2f KiB)", module.GetModule(), result, totalSize/1024)
				return result
			}),
		}

		// keys keep track of insertion order
		graph.keys = append(graph.keys, module)
		graph.nodes[module] = node

		// public and runtime dependencies are viral
		for i, dep := range private {
			node.addPrivate(graph.modules[rules.PrivateDependencies[i]])
			dep.Public(node.addPrivate)
			dep.Runtime(node.addRuntime)
		}
		for i, dep := range public {
			node.addPublic(graph.modules[rules.PublicDependencies[i]])
			dep.Public(node.addPublic)
			dep.Runtime(node.addRuntime)
		}
		for i, dep := range runtime {
			node.addRuntime(graph.modules[rules.RuntimeDependencies[i]])
			dep.Public(node.addRuntime)
			dep.Runtime(node.addRuntime)
		}

		// keep all dependencies in deterministic order using freshly computed Ordinal
		node.SortDependencies(graph)
		return node
	}
}

var GetModuleGraph = utils.MemoizeArg(func(targets *BuildModulesT) *ModuleGraph {
	result := &ModuleGraph{
		keys:    []Module{},
		modules: make(map[string]Module, len(targets.Modules)),
		nodes:   make(map[Module]*ModuleNode, len(targets.Modules)),
	}

	for _, module := range targets.Modules {
		result.modules[module.GetModule().String()] = module
	}

	for _, module := range targets.Modules {
		result.expandModule(module)
	}

	for _, module := range result.keys {
		node, _ := result.nodes[module]
		utils.LogTrace("module #%02d\t|%2d |%2d |%2d |\t%v",
			node.Ordinal,
			node.Private(func(Module) {}),
			node.Public(func(Module) {}),
			node.Runtime(func(Module) {}),
			module.GetModule().String())
	}

	return result
})
