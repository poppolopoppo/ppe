package utils

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"errors"
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
	Serialize(io.Writer) error
	Deserialize(io.Reader) error
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

func (x BuildAlias) String() string {
	return string(x)
}

type buildNode struct {
	Stamp   BuildStamp
	Static  BuildDependencies
	Dynamic BuildDependencies
	Output  BuildDependencies
	Buildable
}

func newBuildNode(builder Buildable) *buildNode {
	return &buildNode{
		Buildable: builder,
		Static:    BuildDependencies{},
		Dynamic:   BuildDependencies{},
		Output:    BuildDependencies{},
	}
}
func (node *buildNode) Alias() BuildAlias         { return node.Buildable.Alias() }
func (node *buildNode) GetBuildStamp() BuildStamp { return node.Stamp }
func (node *buildNode) GetBuildable() Buildable   { return node.Buildable }
func (node *buildNode) AddStatic(g BuildGraph, a BuildAlias) (Buildable, error) {
	dep, fut := g.Build(a)
	if ret := fut.Join(); ret.Failure() == nil {
		node.Static[a] = ret.Success()
		return dep.GetBuildable(), nil
	} else {
		return nil, ret.Failure()
	}
}
func (node *buildNode) AddDynamic(g BuildGraph, it ...BuildAliasable) error {
	var futures = make(map[BuildAlias]Future[BuildStamp], len(it))
	for _, x := range it {
		a := x.Alias()
		_, fut := g.Build(a)
		futures[a] = fut
	}
	for a, fut := range futures {
		if ret := fut.Join(); ret.Failure() == nil {
			node.Dynamic[a] = ret.Success()
		} else {
			return ret.Failure()
		}
	}
	return nil
}
func (node *buildNode) AddOutput(g BuildGraph, f Filename) (Buildable, error) {
	dep, fut := g.ForceBuild(g.Create(f).GetBuildable())
	if ret := fut.Join(); ret.Failure() == nil {
		node.Output[f.Alias()] = ret.Success()
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
	sync.Mutex
	*buildGraph
	*buildNode
}

func (ctx *buildExecuteContext) DependsOn(it ...BuildAliasable) {
	ctx.Mutex.Lock()
	defer ctx.Mutex.Unlock()
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
	ctx.Mutex.Lock()
	defer ctx.Mutex.Unlock()
	for _, f := range files {
		ctx.AddOutput(ctx.buildGraph, f)
	}
}

type buildGraph struct {
	barrier  sync.Mutex
	flags    *CommandFlagsT
	nodes    *SharedMapT[BuildAlias, *buildNode]
	futures  *SharedMapT[BuildAlias, Future[BuildStamp]]
	revision int32
}
type buildGraphData map[BuildAlias]*buildNode

func NewBuildGraph(flags *CommandFlagsT) BuildGraph {
	return &buildGraph{
		flags:    flags,
		nodes:    NewSharedMapT[BuildAlias, *buildNode](),
		futures:  NewSharedMapT[BuildAlias, Future[BuildStamp]](),
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
		if fut, ok := g.futures.Get(a); ok {
			return node, fut
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

func (g *buildGraph) Serialize(dst io.Writer) error {
	g.barrier.Lock()
	defer g.barrier.Unlock()

	g.revision = 0
	exported := g.nodes.Values()

	enc := gob.NewEncoder(dst)
	return enc.Encode(&exported)
}
func (g *buildGraph) Deserialize(src io.Reader) error {
	g.barrier.Lock()
	defer g.barrier.Unlock()

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

func (deps BuildDependencies) needToBuild(g *buildGraph, pbar PinnedProgress) (bool, error) {
	futures := make(map[BuildAlias]Future[BuildStamp], len(deps))
	for a := range deps {
		_, fut := g.Build(a)
		futures[a] = fut
	}
	var errs []error
	rebuild := false
	for a, oldStamp := range deps {
		fut := futures[a].Join()
		if pbar != nil {
			pbar.Inc()
		}
		if err := fut.Failure(); err == nil {
			if newStamp := fut.Success(); newStamp != oldStamp {
				LogVeryVerbose("need to rebuild <%v>:\n\tnew: %v\n\told: %v", a, newStamp, oldStamp)
				deps[a] = newStamp
				rebuild = true
			}
		} else {
			errs = append(errs, err)
		}
	}
	var err error
	if len(errs) > 0 {
		text := strings.Builder{}
		for _, x := range errs {
			fmt.Fprint(&text, x)
		}
		err = errors.New(text.String())
	}
	return rebuild, err
}
func (node *buildNode) needToBuild(g *buildGraph) (bool, error) {
	n := len(node.Static) + len(node.Dynamic) + len(node.Output)
	if n > 0 {
		pbar := LogProgress(0, n, node.Alias().String())
		defer pbar.Close()
		if rebuild, err := node.Static.needToBuild(g, pbar); rebuild || err != nil {
			return rebuild, err
		}
		if rebuild, err := node.Dynamic.needToBuild(g, pbar); rebuild || err != nil {
			return rebuild, err
		}
		if rebuild, err := node.Output.needToBuild(g, pbar); rebuild || err != nil {
			if err != nil {
				LogWarning("%v: %v", node.Alias(), err)
			}
			return true, nil
		}
		return false, nil
	} else {
		return true, nil // always build nodes with empty dependencies
	}
}
func (g *buildGraph) launchBuild(node *buildNode, a BuildAlias, needUpdate bool) Future[BuildStamp] {
	g.barrier.Lock()
	defer g.barrier.Unlock()

	if fut, ok := g.futures.Get(a); !ok || needUpdate {
		if needUpdate && ok {
			fut.Join()
		}
		fut = MakeFuture(func() (BuildStamp, error) {
			// LogVeryVerbose("check '%v'", a)

			// benchmark := LogBenchmark(a.String())
			// defer benchmark.Close()

			var err error
			var rebuild bool
			if rebuild, err = node.needToBuild(g); (rebuild || g.flags.Force) && err == nil {
				// LogVeryVerbose("need to build '%v'", a)

				node.Dynamic = BuildDependencies{}
				node.Output = BuildDependencies{}
				// node.Static.needToBuild(g, nil)

				ctx := buildExecuteContext{sync.Mutex{}, g, node}

				var stamp BuildStamp
				if stamp, err = node.Build(&ctx); err == nil {
					if node.Stamp != stamp {
						LogInfo("updated '%v'", a)
						node.Stamp = stamp
						g.makeDirty()
					} else {
						LogVerbose("up-to-date '%v'", a)
					}

					return node.Stamp, nil
				} else {
					return node.Stamp, err
				}
			} else if err != nil {
				LogError("can't build '%v': %v", a, err)
			} else {
				LogVerbose("up-to-date '%v'", a)
			}
			return node.Stamp, err
		})
		g.futures.Add(a, fut)
		return fut
	} else {
		return fut
	}
}

type BuilderFactory[T Buildable] func(BuildGraph) T

func (f BuilderFactory[T]) Create(g BuildGraph) T {
	return f(g)
}
func (f BuilderFactory[T]) Prepare(g BuildGraph) (T, Future[BuildStamp]) {
	node, future := g.Build(f.Create(g))
	return node.GetBuildable().(T), future
}
func (f BuilderFactory[T]) Build(g BuildGraph) (T, error) {
	builder, future := f.Prepare(g)
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
	return MemoizePod(func(g BuildGraph) T {
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

func MakeBuildStamp(content ...interface{}) (BuildStamp, error) {
	return MakeTimedBuildStamp(time.Time{}, content...)
}
func MakeTimedBuildStamp(modTime time.Time, content ...interface{}) (BuildStamp, error) {
	buf := bytes.Buffer{}
	buf.Grow(2 * 8 * len(content))
	for i, x := range content {
		buf.WriteByte(byte(i))
		switch x.(type) {
		case bool:
			if x.(bool) {
				buf.WriteByte(0xFF)
			} else {
				buf.WriteByte(0x00)
			}
		case int:
			tmp := [binary.MaxVarintLen64]byte{}
			len := binary.PutVarint(tmp[:], int64(x.(int)))
			buf.Write(tmp[:len])
		case uint:
			tmp := [binary.MaxVarintLen64]byte{}
			len := binary.PutUvarint(tmp[:], uint64(x.(int)))
			buf.Write(tmp[:len])
		case string:
			buf.WriteString(x.(string))
		case Digestable:
			x.(Digestable).GetDigestable(&buf)
		default:
			UnexpectedValue(x)
		}
	}
	stamp := BuildStamp{
		ModTime: modTime,
		Content: MakeDigest(RawBytes(buf.Bytes())),
	}
	return stamp, nil
}

func (x Directory) Alias() BuildAlias {
	return BuildAlias(x.String())
}
func (x Directory) Build(BuildContext) (BuildStamp, error) {
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
	if info, err := x.Info(); err == nil {
		return BuildStamp{
			ModTime: info.ModTime(),
			Content: MakeDigest(IntVar(info.Size()), IntVar(info.Mode())),
		}, nil
	} else {
		return BuildStamp{}, err
	}
}
