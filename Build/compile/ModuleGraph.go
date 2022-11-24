package compile

import (
	"build/utils"
	"fmt"
	"math"
	"sort"
	"strconv"
	"sync"
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
	Ordinal int // Module deterministic sort order

	Dependencies []ModuleDependency

	Files     func() utils.FileSet
	TotalSize func() uint32
	Unity     func(int) UnityType

	Rules *ModuleRules
	Unit  *Unit
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
	CompileUnits()
	SortedKeys() []Module
	Module(utils.BuildAlias) Module
	NodeByModule(Module) *ModuleNode
	NodeByAlias(utils.BuildAlias) *ModuleNode
	EachNode(func(Module, *ModuleNode))
}

type moduleGraph struct {
	env     *CompileEnv
	keys    []Module
	modules map[string]Module
	nodes   map[Module]*ModuleNode
}

func (graph *moduleGraph) SortedKeys() []Module {
	return graph.keys
}
func (graph *moduleGraph) Module(name utils.BuildAlias) Module {
	if module, ok := graph.modules[name.String()]; ok {
		return module
	} else {
		utils.LogPanic("unknown module name '%v'", name)
		return nil
	}
}
func (graph *moduleGraph) NodeByModule(module Module) *ModuleNode {
	if node, ok := graph.nodes[module]; ok {
		return node
	} else {
		utils.LogPanic("module node not constructed for <%v>", module)
		return nil
	}
}
func (graph *moduleGraph) NodeByAlias(a utils.BuildAlias) *ModuleNode {
	return graph.NodeByModule(graph.Module(a))
}
func (graph *moduleGraph) EachNode(each func(Module, *ModuleNode)) {
	for _, module := range graph.keys {
		each(module, graph.nodes[module])
	}
}

func NewModuleGraph(env *CompileEnv, targets *BuildModulesT) ModuleGraph {
	result := &moduleGraph{
		env:     env,
		modules: make(map[string]Module, len(targets.Modules)),
		nodes:   make(map[Module]*ModuleNode, len(targets.Modules)),
	}

	for _, module := range targets.Modules {
		result.modules[module.String()] = module
	}

	for _, module := range targets.Modules {
		result.expandModule(env, module)
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
			return result.keys[i].ModuleAlias().Compare(result.keys[j].ModuleAlias()) < 0
		}
	})

	for ord, module := range result.keys {
		node := result.nodes[module]
		node.Ordinal = ord // record the enumeration order in the node
		utils.LogTrace("module %02d#%02d\t|%2d |%2d |%2d |\t%v",
			node.Level,
			node.Ordinal,
			utils.MakeStringer(func() string {
				return strconv.FormatInt(int64(node.Private(func(Module) {})), 10)
			}),
			utils.MakeStringer(func() string {
				return strconv.FormatInt(int64(node.Public(func(Module) {})), 10)
			}),
			utils.MakeStringer(func() string {
				return strconv.FormatInt(int64(node.Runtime(func(Module) {})), 10)
			}),
			module.String())
	}

	for _, node := range result.nodes {
		// sort all dependencies with the new ordinal value
		node.sortDependencies(result)
	}

	return result
}

func (graph *moduleGraph) CompileUnits() {
	utils.Assert(func() bool { return graph.env != nil })
	pbar := utils.LogProgress(0, 0, "%v/Compile", graph.env.EnvironmentAlias())

	wg := sync.WaitGroup{}
	wg.Add(len(graph.keys))

	for _, m := range graph.keys {
		go func(module Module) {
			defer pbar.Inc()
			defer wg.Done()

			node := graph.NodeByModule(module)
			node.Unit = graph.env.Compile(node.Rules)
		}(m)
	}

	wg.Wait()
	pbar.Close()
}

func (graph *moduleGraph) expandDependencies(env *CompileEnv, deps ...ModuleAlias) []*ModuleNode {
	return utils.Map(func(name ModuleAlias) *ModuleNode {
		if module, ok := graph.modules[name.String()]; ok {
			return graph.expandModule(env, module)
		} else {
			utils.LogPanic("module graph: can't find module dependency <%v>", name)
			return nil
		}
	}, deps...)
}
func (graph *moduleGraph) expandModule(env *CompileEnv, module Module) (node *ModuleNode) {
	if node, ok := graph.nodes[module]; ok {
		return node
	}

	rules := module.GetModule(env)

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
		Files:        utils.Memoize(rules.Source.GetFileSet),
		TotalSize: utils.Memoize(func() uint32 {
			return uint32(node.Files().TotalSize())
		}),
		Unity: utils.MemoizePod(func(sizePerUnity int) UnityType {
			totalSize := float64(node.TotalSize())
			numUnityFiles := totalSize / float64(sizePerUnity)
			result := UnityType(int32(math.Ceil(numUnityFiles)))
			utils.LogTrace("%v: %d unity files (%.2f KiB)", module.GetModule(nil), result, totalSize/1024)
			return result
		}),
		Rules: rules,
	}

	// keys keep track of insertion order
	graph.nodes[module] = node

	// public and runtime dependencies are viral
	for i, dep := range private {
		node.addPrivate(graph.modules[rules.PrivateDependencies[i].String()])
		dep.Public(node.addPrivate)
		dep.Runtime(node.addRuntime)
	}
	for i, dep := range public {
		node.addPublic(graph.modules[rules.PublicDependencies[i].String()])
		dep.Public(node.addPublic)
		dep.Runtime(node.addRuntime)
	}
	for i, dep := range runtime {
		node.addRuntime(graph.modules[rules.RuntimeDependencies[i].String()])
		dep.Runtime(node.addRuntime)
	}

	return node
}
