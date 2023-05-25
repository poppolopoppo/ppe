package compile

import (
	//lint:ignore ST1001 ignore dot imports warning
	. "build/utils"
	"fmt"
	"sort"
	"strconv"
)

/***************************************
 * Determinist sort of modules by dependencies and name
 ***************************************/

type ModuleDependency struct {
	Module
	Visibility VisibilityType
}

func (x ModuleDependency) String() string {
	return fmt.Sprintf("%v:%v", x.Visibility, x.Module)
}

type ModuleNode struct {
	Level   int
	Ordinal TargetBuildOrder // Module deterministic sort order

	Dependencies []ModuleDependency
	Rules        *ModuleRules
	Unit         *Unit
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
func (x *ModuleNode) sortDependencies(graph *moduleGraph) {
	sort.Slice(x.Dependencies, func(i, j int) bool {
		lhs := x.Dependencies[i].Module
		rhs := x.Dependencies[j].Module
		return (graph.NodeByModule(lhs).Ordinal < graph.NodeByModule(rhs).Ordinal)
	})
}

type ModuleGraph interface {
	SortedModules() []Module
	Module(ModuleAlias) Module
	NodeByModule(Module) *ModuleNode
	NodeByAlias(ModuleAlias) *ModuleNode
	EachNode(func(Module, *ModuleNode) error) error
}

type moduleGraph struct {
	keys    []Module
	modules map[ModuleAlias]Module
	nodes   map[Module]*ModuleNode
}

func (graph *moduleGraph) SortedModules() []Module {
	return graph.keys
}
func (graph *moduleGraph) Module(name ModuleAlias) Module {
	if module, ok := graph.modules[name]; ok {
		return module
	} else {
		LogPanic(LogCompile, "unknown module name '%v'", name)
		return nil
	}
}
func (graph *moduleGraph) NodeByModule(module Module) *ModuleNode {
	if node, ok := graph.nodes[module]; ok {
		return node
	} else {
		LogPanic(LogCompile, "module node not constructed for <%v>", module)
		return nil
	}
}
func (graph *moduleGraph) NodeByAlias(a ModuleAlias) *ModuleNode {
	return graph.NodeByModule(graph.Module(a))
}
func (graph *moduleGraph) EachNode(each func(Module, *ModuleNode) error) error {
	for _, module := range graph.keys {
		if err := each(module, graph.nodes[module]); err != nil {
			return err
		}
	}
	return nil
}

func MakeModuleGraph(env *CompileEnv, targets *BuildModules) (ModuleGraph, error) {
	result := &moduleGraph{
		modules: make(map[ModuleAlias]Module, len(targets.Modules)),
		nodes:   make(map[Module]*ModuleNode, len(targets.Modules)),
	}

	for _, moduleAlias := range targets.Modules {
		if module, err := GetBuildModule(moduleAlias); err == nil {
			result.modules[moduleAlias] = module
		} else {
			return nil, err
		}
	}

	for _, moduleAlias := range targets.Modules {
		result.expandModule(env, result.modules[moduleAlias])
	}

	result.keys = make([]Module, 0, len(result.nodes))
	for m := range result.nodes {
		result.keys = append(result.keys, m)
	}

	sort.SliceStable(result.keys, func(i, j int) bool {
		a := result.nodes[result.keys[i]]
		b := result.nodes[result.keys[j]]
		if a.Level != b.Level {
			return a.Level < b.Level
		} else {
			return a.Rules.ModuleAlias.Compare(b.Rules.ModuleAlias) < 0
		}
	})

	for ord, module := range result.keys {
		node := result.nodes[module]
		node.Ordinal = TargetBuildOrder(ord) // record the enumeration order in the node
		LogTrace(LogCompile, "module %02d#%02d\t|%2d |%2d |%2d |\t%v",
			node.Level,
			node.Ordinal,
			MakeStringer(func() string {
				return strconv.FormatInt(int64(node.Private(func(Module) {})), 10)
			}),
			MakeStringer(func() string {
				return strconv.FormatInt(int64(node.Public(func(Module) {})), 10)
			}),
			MakeStringer(func() string {
				return strconv.FormatInt(int64(node.Runtime(func(Module) {})), 10)
			}),
			MakeStringer(func() string {
				return module.String()
			}))
	}

	for _, node := range result.nodes {
		// sort all dependencies with the new ordinal value
		node.sortDependencies(result)
	}

	return result, nil
}

func (graph *moduleGraph) expandDependencies(env *CompileEnv, deps ...ModuleAlias) []*ModuleNode {
	return Map(func(name ModuleAlias) *ModuleNode {
		if module, ok := graph.modules[name]; ok {
			return graph.expandModule(env, module)
		} else {
			LogPanic(LogCompile, "module graph: can't find module dependency <%v>", name)
			return nil
		}
	}, deps...)
}
func (graph *moduleGraph) expandModule(env *CompileEnv, module Module) (node *ModuleNode) {
	if node, ok := graph.nodes[module]; ok {
		return node
	}

	rules := module.ExpandModule(env)

	private := graph.expandDependencies(env, rules.PrivateDependencies...)
	public := graph.expandDependencies(env, rules.PublicDependencies...)
	runtime := graph.expandDependencies(env, rules.RuntimeDependencies...)

	level := -1
	for _, x := range private {
		if x.Level > level {
			level = x.Level
		}
	}
	for _, x := range public {
		if x.Level > level {
			level = x.Level
		}
	}
	for _, x := range runtime {
		if x.Level > level {
			level = x.Level
		}
	}
	level += 1

	node = &ModuleNode{
		Level:        level,
		Ordinal:      -1,
		Dependencies: []ModuleDependency{},
		Rules:        rules,
	}

	// keys keep track of insertion order
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
		dep.Runtime(node.addRuntime)
	}

	return node
}
