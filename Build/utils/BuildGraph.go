package utils

import (
	"encoding/gob"
	"fmt"
	"io"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

type BuildAlias string
type BuildAliases struct {
	SetT[BuildAlias]
}

func MakeBuildAlias(category string, name string) BuildAlias {
	return BuildAlias(category + "://" + name)
}
func (x BuildAlias) Alias() BuildAlias { return x }
func (x BuildAlias) Equals(o BuildAlias) bool {
	return string(x) == string(o)
}
func (x BuildAlias) Compare(o BuildAlias) int {
	return strings.Compare(string(x), string(o))
}
func (x BuildAlias) String() string {
	return string(x)
}

type BuildStamp struct {
	ModTime time.Time
	Content Digest
}

func (x BuildStamp) String() string {
	return fmt.Sprintf("[%X] %v", x.Content[:8], x.ModTime.Local().Format(time.Stamp))
}

type BuildDependencies map[BuildAlias]BuildStamp

type BuildNode interface {
	BuildAliasable
	GetBuildStamp() BuildStamp
	GetBuildable() Buildable
}
type BuildGraph interface {
	Aliases() []BuildAlias
	Dirty() bool
	Node(BuildAliasable) BuildNode
	Create(Buildable, ...BuildAlias) BuildNode
	Build(BuildAliasable) (BuildNode, Future[BuildStamp])
	ForceBuild(Buildable) (BuildNode, Future[BuildStamp])
	Join()
	PostLoad()
	Serialize(io.Writer) error
	Deserialize(io.Reader) error
	Equatable[BuildGraph]
}

type BuildInit interface {
	DependsOn(...BuildAliasable)
	NeedFile(...Filename)
	NeedFolder(...Directory)
}
type BuildContext interface {
	DependsOn(...BuildAliasable)
	NeedFile(...Filename)
	NeedFolder(...Directory)
	OutputFile(...Filename)
}

type BuildAliasable interface {
	Alias() BuildAlias
}
type Buildable interface {
	BuildAliasable
	Build(BuildContext) (BuildStamp, error)
}

type buildState struct {
	launch sync.Mutex
	edit   sync.Mutex
	future Future[BuildStamp]
}

type buildNode struct {
	Stamp   BuildStamp
	Static  BuildDependencies
	Dynamic BuildDependencies
	Output  BuildDependencies
	Buildable

	state buildState
}

func newBuildNode(builder Buildable) *buildNode {
	return &buildNode{
		Buildable: builder,
		Static:    BuildDependencies{},
		Dynamic:   BuildDependencies{},
		Output:    BuildDependencies{},
		state: buildState{
			launch: sync.Mutex{},
			edit:   sync.Mutex{},
			future: nil,
		},
	}
}
func (node *buildNode) Alias() BuildAlias         { return node.Buildable.Alias() }
func (node *buildNode) String() string            { return node.Alias().String() }
func (node *buildNode) GetBuildStamp() BuildStamp { return node.Stamp }
func (node *buildNode) GetBuildable() Buildable   { return node.Buildable }
func (node *buildNode) Join() Result[BuildStamp] {
	node.state.launch.Lock()
	defer node.state.launch.Unlock()
	return node.state.future.Join()
}
func (node *buildNode) AddStatic(g BuildGraph, a BuildAlias) (Buildable, error) {
	LogDebug("buildgraph: <%v> has static dependency on '%v'", node, a)
	dep, fut := g.Build(a)
	if ret := fut.Join(); ret.Failure() == nil {
		node.state.edit.Lock()
		defer node.state.edit.Unlock()
		stamp := ret.Success()
		Assert(func() bool { return stamp.Content.Valid() })
		node.Static[a] = stamp
		return dep.GetBuildable(), nil
	} else {
		return nil, ret.Failure()
	}
}
func (node *buildNode) AddDynamic(g BuildGraph, it ...BuildAliasable) error {
	futures := make(map[BuildAlias]Future[BuildStamp], len(it))
	for _, x := range it {
		a := x.Alias()
		LogDebug("buildgraph: <%v> has dynamic dependency on '%v'", node, a)
		_, fut := g.Build(a)
		futures[a] = fut
	}
	node.state.edit.Lock()
	defer node.state.edit.Unlock()
	for a, fut := range futures {
		if ret := fut.Join(); ret.Failure() == nil {
			stamp := ret.Success()
			Assert(func() bool { return stamp.Content.Valid() })
			node.Dynamic[a] = stamp
		} else {
			return ret.Failure()
		}
	}
	return nil
}
func (node *buildNode) AddOutput(g BuildGraph, f Filename) (Buildable, error) {
	LogDebug("buildgraph: <%v> outputs file '%v'", node, f)
	dep, fut := g.ForceBuild(g.Create(f).GetBuildable())
	if ret := fut.Join(); ret.Failure() == nil {
		node.state.edit.Lock()
		defer node.state.edit.Unlock()
		stamp := ret.Success()
		Assert(func() bool { return stamp.Content.Valid() })
		node.Output[f.Alias()] = stamp
		return dep.GetBuildable(), nil
	} else {
		return nil, ret.Failure()
	}
}

type buildInitContext struct {
	BuildGraph
	BuildAliases
}

func (ctx buildInitContext) DependsOn(it ...BuildAliasable) {
	for _, x := range it {
		ctx.Append(ctx.Node(x).Alias())
	}
}
func (ctx buildInitContext) NeedFile(files ...Filename) {
	ctx.DependsOn(Map(func(f Filename) BuildAliasable {
		return ctx.Create(f)
	}, files...)...)
}
func (ctx buildInitContext) NeedFolder(folders ...Directory) {
	ctx.DependsOn(Map(func(d Directory) BuildAliasable {
		return ctx.Create(d)
	}, folders...)...)
}

type buildExecuteContext struct {
	*buildGraph
	*buildNode
}

func (ctx *buildExecuteContext) DependsOn(it ...BuildAliasable) {
	if err := ctx.AddDynamic(ctx.buildGraph, it...); err != nil {
		panic(err)
	}
}
func (ctx *buildExecuteContext) NeedFile(files ...Filename) {
	ctx.DependsOn(Map(func(f Filename) BuildAliasable {
		return ctx.Create(f)
	}, files...)...)
}
func (ctx *buildExecuteContext) NeedFolder(folders ...Directory) {
	ctx.DependsOn(Map(func(d Directory) BuildAliasable {
		return ctx.Create(d)
	}, folders...)...)
}
func (ctx *buildExecuteContext) OutputFile(files ...Filename) {
	for _, f := range files {
		ctx.AddOutput(ctx.buildGraph, f)
	}
}

type buildGraph struct {
	flags    *CommandFlagsT
	nodes    *SharedMapT[BuildAlias, *buildNode]
	revision int32
}
type buildGraphData map[BuildAlias]*buildNode

func NewBuildGraph(flags *CommandFlagsT) BuildGraph {
	return &buildGraph{
		flags:    flags,
		nodes:    NewSharedMapT[BuildAlias, *buildNode](),
		revision: 0,
	}
}
func (g *buildGraph) makeDirty() {
	atomic.AddInt32(&g.revision, 1)
}
func (g *buildGraph) Aliases() []BuildAlias {
	return g.nodes.Keys()
}
func (g *buildGraph) Dirty() bool {
	return atomic.LoadInt32(&g.revision) > 0
}
func (g *buildGraph) Node(a BuildAliasable) BuildNode {
	if node, ok := g.nodes.Get(a.Alias()); ok {
		return node
	} else {
		UnreachableCode()
		return nil
	}
}
func (g *buildGraph) Create(b Buildable, static ...BuildAlias) BuildNode {
	node, loaded := g.nodes.FindOrAdd(b.Alias(), newBuildNode(b))
	AssertSameType(node.Buildable, b)
	deps := BuildDependencies{}
	dirty := !loaded || len(static) != len(node.Static)
	for _, a := range static {
		old, hit := node.Static[a]
		deps[a] = old
		if !hit {
			dirty = true
		}
	}
	node.Static = deps
	if dirty {
		g.makeDirty()
	}
	return node
}
func (g *buildGraph) Build(it BuildAliasable) (BuildNode, Future[BuildStamp]) {
	a := it.Alias()
	if node, ok := g.nodes.Get(a); ok {
		if node.state.future != nil {
			return node, node.state.future
		} else {
			return node, g.launchBuild(node, a, false)
		}
	} else {
		return nil, MakeFutureError[BuildStamp](fmt.Errorf("build: unknown node <%v>", a))
	}
}
func (g *buildGraph) ForceBuild(it Buildable) (BuildNode, Future[BuildStamp]) {
	a := it.Alias()
	if node, ok := g.nodes.Get(a); ok {
		return node, g.launchBuild(node, a, true)
	} else {
		return g.Build(g.Create(it))
	}
}
func (g *buildGraph) Join() {
	g.nodes.Range(func(alias BuildAlias, node *buildNode) {
		if node != nil {
			node.state.launch.Lock()
			defer node.state.launch.Unlock()

			fut := node.state.future
			if fut != nil && fut.Available() == false {
				LogWarning("builder <%v> wasn't available and needed to be joined", alias)
				fut.Join()
			}
		}
	})
}
func (g *buildGraph) PostLoad() {
	if g.flags.Purge {
		g.revision = 0
		g.nodes.Clear()
		g.makeDirty()
	}
}
func (g *buildGraph) Serialize(dst io.Writer) error {
	g.revision = 0
	exported := g.nodes.Values()

	enc := gob.NewEncoder(dst)
	return enc.Encode(&exported)
}
func (g *buildGraph) Deserialize(src io.Reader) error {
	imported := []*buildNode{}
	dec := gob.NewDecoder(src)
	if err := dec.Decode(&imported); err != nil {
		return err
	}

	g.revision = 0
	g.nodes.Clear()
	for _, node := range imported {
		g.nodes.Add(node.Alias(), node)
	}
	return nil
}
func (g *buildGraph) Equals(other BuildGraph) bool {
	return other.(*buildGraph) == g
}

type buildBatch struct {
	Graph    *buildGraph
	Caller   *buildNode
	Prepared map[BuildAlias]*buildNode
	Received map[BuildAlias]Result[BuildStamp]
}

func (batch *buildBatch) Join(pbar PinnedProgress) {
	// first pass: join all dependencies
	for _, fut := range batch.Prepared {
		fut.Join()
		if pbar != nil {
			pbar.Inc()
		}
	}

	// second-pass: collect results only *after* join
	batch.Received = make(map[BuildAlias]Result[BuildStamp], len(batch.Prepared))
	for a, fut := range batch.Prepared {
		batch.Received[a] = fut.Join()
	}
}

func (deps BuildDependencies) prepareBuild(batch *buildBatch) {
	for a := range deps {
		Assert(func() bool { return batch.Prepared[a] == nil })
		node, _ := batch.Graph.Build(a) // trigger async build, don't wait for the result here
		batch.Prepared[a] = node.(*buildNode)
	}
}
func (deps BuildDependencies) joinBuild(batch *buildBatch, depType string) (bool, error) {
	var err error
	rebuild := false

	for a, oldStamp := range deps {
		result := batch.Received[a]

		if depErr := result.Failure(); depErr == nil {
			if newStamp := result.Success(); newStamp != oldStamp {
				LogVeryVerbose("%v: need to rebuild %v dependency <%v>:\n\tnew: %v\n\told: %v", batch.Caller.Alias(), depType, a, newStamp, oldStamp)
				deps[a] = newStamp
				rebuild = true
			} else {
				LogDebug("%v: %v %s dependency is up-to-date", batch.Caller.Alias(), a, depType)
			}
		} else {
			LogWarning("%v: %s dependency failed with %v", batch.Caller.Alias(), depType, depErr)
			err = depErr
		}
	}

	return rebuild, err
}

func (node *buildNode) needToBuild(g *buildGraph) (bool, error) {
	n := len(node.Static) + len(node.Dynamic) + len(node.Output)
	if n <= 0 {
		return true, nil
	}

	pbar := LogProgress(0, n, node.Alias().String())
	defer pbar.Close()

	batch := buildBatch{
		Graph:    g,
		Caller:   node,
		Prepared: make(map[BuildAlias]*buildNode, n),
	}

	node.Static.prepareBuild(&batch)
	node.Dynamic.prepareBuild(&batch)
	node.Output.prepareBuild(&batch)

	batch.Join(pbar)

	var err error
	rebuild := false

	if rebuildStatic, errStatic := node.Static.joinBuild(&batch, "static"); rebuildStatic || errStatic != nil {
		rebuild = rebuild || rebuildStatic
		if errStatic != nil {
			LogWarning("%v: %v", node.Alias(), errStatic)
			rebuild = true
		}
	}
	if rebuildDynamic, errDynamic := node.Dynamic.joinBuild(&batch, "dynamic"); rebuildDynamic || errDynamic != nil {
		rebuild = rebuild || rebuildDynamic
		if errDynamic != nil {
			LogWarning("%v: %v", node.Alias(), errDynamic)
			rebuild = true
		}
	}
	if rebuildOutput, errOutput := node.Output.joinBuild(&batch, "output"); rebuildOutput || errOutput != nil {
		rebuild = rebuild || rebuildOutput
		if errOutput != nil {
			LogError("%v: %v", node.Alias(), errOutput)
			rebuild = true
		}
	}

	if !rebuild {
		switch node.Buildable.(type) {
		case Digestable: // rebuild if digest was invalidated by code
			checksum := MakeDigest(node.Buildable.(Digestable))
			return checksum != node.Stamp.Content, nil
		default:
			return false, nil
		}
	} else {
		return rebuild, err
	}
}
func (g *buildGraph) launchBuild(node *buildNode, a BuildAlias, needUpdate bool) Future[BuildStamp] {
	node.state.launch.Lock()
	defer node.state.launch.Unlock()

	if node.state.future == nil || needUpdate {
		if node.state.future != nil {
			node.state.future.Join()
		}
		node.state.future = MakeFuture(func() (BuildStamp, error) {
			var err error
			var rebuild bool
			if rebuild, err = node.needToBuild(g); (rebuild || g.flags.Force) && err == nil {
				node.Dynamic = BuildDependencies{}
				node.Output = BuildDependencies{}

				ctx := buildExecuteContext{g, node}

				var stamp BuildStamp
				if stamp, err = node.Build(&ctx); err == nil {
					Assert(func() bool { return stamp.Content.Valid() })

					if node.Stamp != stamp {
						LogInfo("updated '%v'", a)

						node.Stamp = stamp
						g.makeDirty()
						rebuild = true
					}
				}
			}
			if err != nil {
				LogError("can't build '%v': %v", a, err)
			} else if !rebuild {
				LogVerbose("up-to-date '%v'", a)
			}
			return node.Stamp, err
		})
	}
	return node.state.future
}

type BuilderFactory[T Buildable] func(BuildGraph) T

func (f BuilderFactory[T]) Create(g BuildGraph) T {
	return f(g)
}
func (f BuilderFactory[T]) Prepare(g BuildGraph, force bool) (T, Future[BuildStamp]) {
	var node BuildNode
	var future Future[BuildStamp]
	if force {
		node, future = g.ForceBuild(f.Create(g))
	} else {
		node, future = g.Build(f.Create(g))
	}
	return node.GetBuildable().(T), future
}
func (f BuilderFactory[T]) Build(g BuildGraph) (T, error) {
	builder, future := f.Prepare(g, false)
	_, err := future.Join().Get()
	return builder, err
}
func (f BuilderFactory[T]) Get(bc BuildContext) (builder T, err error) {
	if x, err := f.Build(CommandEnv.BuildGraph()); err == nil {
		bc.DependsOn(x)
		return x, nil
	} else {
		return x, err
	}
}
func (f BuilderFactory[T]) Need(bc BuildContext) T {
	if builder, err := f.Get(bc); err == nil {
		return builder
	} else {
		panic(err)
	}
}

func GetBuildable[T Buildable](g BuildGraph, a BuildAliasable) T {
	return g.Node(a).GetBuildable().(T)
}
func MakeBuildable[T Buildable](factory func(BuildInit) T) BuilderFactory[T] {
	return MemoizeArg(func(g BuildGraph) T {
		ctx := buildInitContext{BuildGraph: g}
		builder := factory(&ctx)
		return g.Create(builder, ctx.BuildAliases.Slice()...).GetBuildable().(T)
	})
}
func MemoizeBuildable[T Buildable](builder T, static ...BuildAlias) BuilderFactory[T] {
	return MakeBuildable(func(init BuildInit) T {
		for _, a := range static {
			init.DependsOn(a)
		}
		return builder
	})
}

func MakeBuildStamp(content Digestable) (BuildStamp, error) {
	return MakeTimedBuildStamp(time.Time{}, content)
}
func MakeTimedBuildStamp(modTime time.Time, content Digestable) (BuildStamp, error) {
	stamp := BuildStamp{
		ModTime: modTime,
		Content: MakeDigest(content),
	}
	Assert(func() bool { return stamp.Content.Valid() })
	return stamp, nil
}

func (x Directory) Alias() BuildAlias {
	return BuildAlias(x.String())
}
func (x Directory) Build(BuildContext) (BuildStamp, error) {
	x.Invalidate()
	if info, err := x.Info(); err == nil {
		return BuildStamp{
			ModTime: info.ModTime(),
			Content: MakeDigest(IntVar(info.Size()), IntVar(info.Mode())),
		}, nil
	} else {
		return BuildStamp{}, err
	}
}

func (x Filename) Alias() BuildAlias {
	return BuildAlias(x.String())
}
func (x Filename) Build(BuildContext) (BuildStamp, error) {
	x.Invalidate()
	if info, err := x.Info(); err == nil {
		return BuildStamp{
			ModTime: info.ModTime(),
			Content: MakeDigest(IntVar(info.Size()), IntVar(info.Mode())),
		}, nil
	} else {
		return BuildStamp{}, err
	}
}
