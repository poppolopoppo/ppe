package utils

import (
	"fmt"
	"io"
	"math"
	"reflect"
	"sort"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

/***************************************
 * Public API
 ***************************************/

type BuildDependencyType int32

const (
	DEPENDENCY_ROOT   BuildDependencyType = -1
	DEPENDENCY_OUTPUT BuildDependencyType = iota
	DEPENDENCY_STATIC
	DEPENDENCY_DYNAMIC
)

type BuildDependencyLink struct {
	Alias BuildAlias
	Type  BuildDependencyType
}

type BuildAlias string
type BuildAliases = SetT[BuildAlias]

type BuildAliasable interface {
	Alias() BuildAlias
}

type Buildable interface {
	BuildAliasable
	Serializable
	Build(BuildContext) error
}

type BuildStamp struct {
	ModTime time.Time
	Content Fingerprint
}

type BuildResult struct {
	Buildable
	BuildStamp
}

type BuildStats struct {
	InclusiveStart time.Time
	ExclusiveStart time.Time
	Duration       struct {
		Inclusive time.Duration
		Exclusive time.Duration
	}
	Count int32
}

type BuildDependencies map[BuildAlias]BuildStamp

type BuildNode interface {
	BuildAliasable

	GetBuildStamp() BuildStamp
	GetBuildStats() BuildStats
	GetBuildable() Buildable

	DependsOn(...BuildAlias) bool

	GetStaticDependencies() BuildAliases
	GetDynamicDependencies() BuildAliases
	GetOutputDependencies() BuildAliases

	GetDependencyLinks() []BuildDependencyLink
}

type BuildOptions struct {
	Parent                   *BuildOptions
	Caller                   BuildNode
	OnLaunched               PublicEvent[BuildNode]
	OnBuilt                  PublicEvent[BuildNode]
	Stamp                    *BuildStamp
	Force                    bool
	Recursive                bool
	NoWarningOnMissingOutput bool
}

type BuildOptionFunc func(*BuildOptions)

type BuildGraph interface {
	Aliases() []BuildAlias
	Dirty() bool

	Find(alias BuildAlias) BuildNode
	Create(buildable Buildable, staticDeps BuildAliases, options ...BuildOptionFunc) BuildNode
	Build(alias BuildAliasable, options ...BuildOptionFunc) (BuildNode, Future[BuildResult])
	BuildMany(aliases BuildAliases, options ...BuildOptionFunc) Future[[]BuildResult]
	Join() error

	Load(io.Reader) error
	PostLoad()
	Save(io.Writer) error

	GetStaticDependencies(BuildNode) []BuildNode
	GetDynamicDependencies(BuildNode) []BuildNode
	GetOutputDependencies(BuildNode) []BuildNode

	GetDependencyChain(a, b BuildAlias) ([]BuildDependencyLink, error)
	GetDependencyInputFiles(BuildAlias) (FileSet, error)
	GetDependencyLinks(BuildAlias) ([]BuildDependencyLink, error)

	GetBuildStats() BuildStats
	GetMostExpansiveNodes(n int, inclusive bool) []BuildNode

	OnBuildGraphStart() MutableEvent[BuildGraph]
	OnBuildNodeStart() MutableEvent[BuildNode]
	OnBuildNodeFinished() MutableEvent[BuildNode]
	OnBuildGraphFinished() MutableEvent[BuildGraph]

	Equatable[BuildGraph]
	Serializable
}

type BuildInitializer interface {
	Options() BuildOptions

	DependsOn(...BuildAlias) error

	NeedFactory(BuildFactory) (Buildable, error)
	NeedFactories(...BuildFactory) error
	NeedBuildable(...BuildAliasable) error
	NeedFile(...Filename) error
	NeedDirectory(...Directory) error
}

type BuildContext interface {
	BuildInitializer

	OutputFile(...Filename) error
	OutputNode(...BuildFactory) error

	Timestamp(time.Time)

	OnBuilt(func(BuildNode) error)
}

type BuildFactory interface {
	Create(BuildInitializer) (Buildable, error)
}

/***************************************
 * Build Node
 ***************************************/

type buildState struct {
	stats BuildStats
	sync.RWMutex
}

type buildNode struct {
	BuildAlias BuildAlias
	Buildable  Buildable

	Stamp BuildStamp

	Static      BuildDependencies
	Dynamic     BuildDependencies
	OutputFiles BuildDependencies
	OutputNodes BuildAliases

	state  buildState
	future AtomicFuture[BuildResult]
}

func newBuildNode(alias BuildAlias, builder Buildable) *buildNode {
	Assert(func() bool { return alias.Valid() })
	Assert(func() bool { return builder.Alias().Equals(alias) })
	return &buildNode{
		BuildAlias: alias,
		Buildable:  builder,
		Stamp:      BuildStamp{},

		Static:      BuildDependencies{},
		Dynamic:     BuildDependencies{},
		OutputFiles: BuildDependencies{},
		OutputNodes: BuildAliases{},

		state: buildState{
			stats:   BuildStats{},
			RWMutex: sync.RWMutex{},
		},

		future: AtomicFuture[BuildResult]{},
	}
}
func (node *buildNode) Alias() BuildAlias { return node.BuildAlias }
func (node *buildNode) String() string    { return node.BuildAlias.String() }
func (node *buildNode) GetBuildable() Buildable {
	node.state.RLock()
	defer node.state.RUnlock()
	return node.Buildable
}
func (node *buildNode) GetBuildStamp() BuildStamp {
	node.state.RLock()
	defer node.state.RUnlock()
	return node.Stamp
}
func (node *buildNode) GetBuildStats() BuildStats {
	node.state.RLock()
	defer node.state.RUnlock()
	return node.state.stats
}
func (node *buildNode) GetStaticDependencies() BuildAliases {
	node.state.RLock()
	defer node.state.RUnlock()
	return Keys(node.Static)
}
func (node *buildNode) GetDynamicDependencies() BuildAliases {
	node.state.RLock()
	defer node.state.RUnlock()
	return Keys(node.Dynamic)
}
func (node *buildNode) GetOutputDependencies() BuildAliases {
	node.state.RLock()
	defer node.state.RUnlock()
	return append(Keys(node.OutputFiles), node.OutputNodes...)
}
func (node *buildNode) GetDependencyLinks() []BuildDependencyLink {
	node.state.RLock()
	defer node.state.RUnlock()
	result := make([]BuildDependencyLink, 0, len(node.Static)+len(node.Dynamic)+len(node.OutputFiles)+len(node.OutputNodes))
	for a := range node.Static {
		result = append(result, BuildDependencyLink{Alias: a, Type: DEPENDENCY_STATIC})
	}
	for a := range node.Dynamic {
		result = append(result, BuildDependencyLink{Alias: a, Type: DEPENDENCY_DYNAMIC})
	}
	for a := range node.OutputFiles {
		result = append(result, BuildDependencyLink{Alias: a, Type: DEPENDENCY_OUTPUT})
	}
	for _, a := range node.OutputNodes {
		result = append(result, BuildDependencyLink{Alias: a, Type: DEPENDENCY_OUTPUT})
	}
	return result
}
func (node *buildNode) DependsOn(aliases ...BuildAlias) bool {
	node.state.RLock()
	defer node.state.RUnlock()
	for _, a := range aliases {
		if _, ok := node.Static[a]; ok {
			return true
		}
	}
	for _, a := range aliases {
		if _, ok := node.Dynamic[a]; ok {
			return true
		}
	}
	return false
}
func (node *buildNode) Serialize(ar Archive) {
	ar.Serializable(&node.BuildAlias)
	SerializeExternal(ar, &node.Buildable)

	ar.Serializable(&node.Stamp)

	ar.Serializable(&node.Static)
	ar.Serializable(&node.Dynamic)
	ar.Serializable(&node.OutputFiles)
	SerializeSlice(ar, node.OutputNodes.Ref())
}

func (node *buildNode) makeDirty_assumeLocked() {
	node.Stamp = BuildStamp{}
	node.Dynamic = BuildDependencies{}
	node.OutputFiles = BuildDependencies{}
	node.OutputNodes = BuildAliases{}
}

func (node *buildNode) addStatic_AssumeLocked(a BuildAlias, stamp BuildStamp) {
	AssertMessage(func() bool { return a.Valid() }, "%v: invalid empty alias for static dependency")
	AssertMessage(func() bool { return !node.Alias().Equals(a) }, "%v: can't have a static dependency to self", a)
	AssertMessage(func() bool { _, ok := node.Dynamic[a]; return !ok }, "%v: static dependency is already dynamic <%v>", node, a)
	AssertMessage(func() bool { _, ok := node.OutputFiles[a]; return !ok }, "%v: static dependency is already output <%v>", node, a)
	AssertMessage(func() bool { ok := node.OutputNodes.Contains(a); return !ok }, "%v: static dependency is already output <%v>", node, a)

	node.Static[a] = stamp
}
func (node *buildNode) addDynamic_AssumeLocked(a BuildAlias, stamp BuildStamp) {
	Assert(func() bool { return stamp.Content.Valid() })
	AssertMessage(func() bool { return a.Valid() }, "%v: invalid empty alias for dynamic dependency")
	AssertMessage(func() bool { return !node.Alias().Equals(a) }, "%v: can't have an output dependency to self", a)
	AssertMessage(func() bool { _, ok := node.Static[a]; return !ok }, "%v: dynamic dependency is already static <%v>", node, a)
	AssertMessage(func() bool { _, ok := node.OutputFiles[a]; return !ok }, "%v: dynamic dependency is already output <%v>", node, a)
	AssertMessage(func() bool { ok := node.OutputNodes.Contains(a); return !ok }, "%v: dynamic dependency is already output <%v>", node, a)

	node.Dynamic[a] = stamp
}
func (node *buildNode) addOutputFile_AssumeLocked(a BuildAlias, stamp BuildStamp) {
	Assert(func() bool { return stamp.Content.Valid() })
	AssertMessage(func() bool { return a.Valid() }, "%v: invalid empty alias for output file dependency")
	AssertMessage(func() bool { return !node.Alias().Equals(a) }, "%v: can't have an output dependency to self", a)
	AssertMessage(func() bool { _, ok := node.Static[a]; return !ok }, "%v: output file dependency is already static <%v>", node, a)
	AssertMessage(func() bool { _, ok := node.Dynamic[a]; return !ok }, "%v: output file dependency is already dynamic <%v>", node, a)
	AssertMessage(func() bool { ok := node.OutputNodes.Contains(a); return !ok }, "%v: output file dependency is already output node <%v>", node, a)

	node.OutputFiles[a] = stamp
}
func (node *buildNode) addOutputNode_AssumeLocked(a BuildAlias) {
	AssertMessage(func() bool { return a.Valid() }, "%v: invalid empty alias for output dependency")
	AssertMessage(func() bool { return !node.Alias().Equals(a) }, "%v: can't have an output dependency to self", a)
	AssertMessage(func() bool { _, ok := node.Static[a]; return !ok }, "%v: output node is already static dependency <%v>", node, a)
	AssertMessage(func() bool { _, ok := node.Dynamic[a]; return !ok }, "%v: output node is already dynamic dependency <%v>", node, a)
	AssertMessage(func() bool { _, ok := node.OutputFiles[a]; return !ok }, "%v: output node dependency is already output file dependency <%v>", node, a)

	node.OutputNodes.AppendUniq(a)
}

/***************************************
 * Build Factory
 ***************************************/

type BuildFactoryTyped[T Buildable] func(BuildInitializer) (T, error)

func (x BuildFactoryTyped[T]) Create(context BuildInitializer) (Buildable, error) {
	return x(context)
}
func (x BuildFactoryTyped[T]) Need(context BuildInitializer) (T, error) {
	if buildable, err := context.NeedFactory(x); err == nil {
		return buildable.(T), nil
	} else {
		var none T
		return none, err
	}
}
func (x BuildFactoryTyped[T]) SafeNeed(context BuildInitializer) (result T) {
	if buildable, err := x.Need(context); err == nil {
		result = buildable
	} else {
		LogPanicErr(err)
	}
	return result
}
func (x BuildFactoryTyped[T]) Init(graph BuildGraph, options ...BuildOptionFunc) (result T, err error) {
	var node *buildNode
	node, err = buildInit(graph.(*buildGraph), x, NewBuildOptions(options...))
	if err == nil {
		result = node.Buildable.(T)
	}
	return
}
func (x BuildFactoryTyped[T]) Prepare(graph BuildGraph, options ...BuildOptionFunc) Future[T] {
	bo := NewBuildOptions(options...)
	node, err := buildInit(graph.(*buildGraph), x, bo)
	if err != nil {
		return MakeFutureError[T](err)
	}

	future := graph.(*buildGraph).launchBuild(node, bo)
	return MapFuture(future, func(it BuildResult) T {
		return it.Buildable.(T)
	})
}
func (x BuildFactoryTyped[T]) Build(graph BuildGraph, options ...BuildOptionFunc) Result[T] {
	return x.Prepare(graph, options...).Join()
}

func MakeBuildFactory[T Buildable](factory func(BuildInitializer) (T, error)) BuildFactoryTyped[T] {
	return BuildFactoryTyped[T](factory)
}
func FindGlobalBuildable[T Buildable](aliasable BuildAliasable) (result T, err error) {
	return FindBuildable[T](CommandEnv.buildGraph, aliasable)
}
func FindBuildable[T Buildable](graph BuildGraph, aliasable BuildAliasable) (result T, err error) {
	alias := aliasable.Alias()
	if node := graph.Find(alias); node != nil {
		result = node.GetBuildable().(T)
	} else {
		err = fmt.Errorf("buildable not found: %q", alias)
	}
	return
}

/***************************************
 * Build Initializer
 ***************************************/

type buildInitializer struct {
	graph   *buildGraph
	options BuildOptions

	staticDeps BuildAliases
	sync.Mutex
}

func buildInit(g *buildGraph, factory BuildFactory, options BuildOptions) (*buildNode, error) {
	context := buildInitializer{
		graph:      g,
		options:    options.Recurse(nil),
		staticDeps: BuildAliases{},
		Mutex:      sync.Mutex{},
	}

	buildable, err := factory.Create(&context)
	if err != nil {
		return nil, err
	}
	Assert(func() bool { return !IsNil(buildable) })

	node := g.Create(buildable, context.staticDeps, OptionBuildStruct(options))

	Assert(func() bool { return node.Alias().Equals(buildable.Alias()) })
	return node.(*buildNode), nil
}
func (x *buildInitializer) Options() BuildOptions {
	return x.options
}
func (x *buildInitializer) DependsOn(aliases ...BuildAlias) error {
	x.Lock()
	defer x.Unlock()

	for _, alias := range aliases {
		if node := x.graph.Find(alias); node != nil {
			x.staticDeps.Append(alias)
		} else {
			return fmt.Errorf("static dependency not found: %q", alias)
		}
	}

	return nil
}
func (x *buildInitializer) NeedFactory(factory BuildFactory) (Buildable, error) {
	node, err := buildInit(x.graph, factory, x.options)
	if err != nil {
		return nil, err
	}

	x.Lock()
	defer x.Unlock()

	x.staticDeps.Append(node.Alias())
	return node.GetBuildable(), nil
}
func (x *buildInitializer) NeedFactories(factories ...BuildFactory) error {
	aliases := make(BuildAliases, len(factories))
	for i, factory := range factories {
		node, err := buildInit(x.graph, factory, x.options)
		if err != nil {
			return err
		}
		aliases[i] = node.Alias()
	}

	x.Lock()
	defer x.Unlock()

	x.staticDeps.Append(aliases...)
	return nil
}
func (x *buildInitializer) NeedBuildable(buildables ...BuildAliasable) error {
	aliases := make([]BuildAlias, len(buildables))

	for i, buildable := range buildables {
		aliases[i] = buildable.Alias()

		if node := x.graph.Find(aliases[i]); node == nil {
			return fmt.Errorf("buildgraph: buildable %q not found", aliases[i])
		}
	}

	x.Lock()
	defer x.Unlock()

	x.staticDeps.Append(aliases...)
	return nil
}
func (x *buildInitializer) NeedFile(files ...Filename) error {
	for _, filename := range files {
		if _, err := x.NeedFactory(BuildFile(filename)); err != nil {
			return err
		}
	}
	return nil
}
func (x *buildInitializer) NeedDirectory(directories ...Directory) error {
	for _, directory := range directories {
		if _, err := x.NeedFactory(BuildDirectory(directory)); err != nil {
			return err
		}
	}
	return nil
}

/***************************************
 * Build Execute Context
 ***************************************/

type buildExecuteContext struct {
	graph *buildGraph
	node  *buildNode

	options BuildOptions

	stamp       BuildStamp
	static      BuildDependencies
	dynamic     BuildDependencies
	outputFiles BuildDependencies

	stats     BuildStats
	timestamp time.Time

	barrier sync.Mutex
}

type buildDependencyError struct {
	link  BuildDependencyType
	outer error
}

func (x buildDependencyError) Error() string {
	return x.outer.Error()
	// return fmt.Sprintf("%s dependency failed with: %v", x.link, x.outer)
}

func makeBuildExecuteContext(g *buildGraph, node *buildNode, options BuildOptions) buildExecuteContext {
	Assert(func() bool { return options.Caller == node })

	node.state.Lock()
	defer node.state.Unlock()

	return buildExecuteContext{
		graph:       g,
		node:        node,
		options:     options,
		stamp:       node.Stamp,
		static:      CopyMap(node.Static),
		dynamic:     CopyMap(node.Dynamic),
		outputFiles: CopyMap(node.OutputFiles),
		timestamp:   CommandEnv.BuildTime(),
	}
}

func (x *buildExecuteContext) buildOutputFiles() Future[[]BuildResult] {
	results := make([]BuildResult, 0, len(x.outputFiles))
	for alias := range x.outputFiles {
		node := x.graph.Find(alias)
		if node == nil {
			return MakeFutureError[[]BuildResult](fmt.Errorf("build-graph: can't find buildable file %q", alias))
		}

		file, ok := node.GetBuildable().(*Filename)
		AssertIn(ok, true)

		if stamp, err := file.Digest(); err == nil {
			results = append(results, BuildResult{
				Buildable:  file,
				BuildStamp: stamp,
			})
		} else {
			return MakeFutureError[[]BuildResult](err)
		}
	}
	return MakeFutureLiteral(results)
}
func (x *buildExecuteContext) numDependencies() int {
	return len(x.static) + len(x.dynamic) + len(x.outputFiles)
}
func (x *buildExecuteContext) needToBuild() (bool, BuildDependencyType, error) {
	if x.numDependencies() == 0 {
		return true, DEPENDENCY_ROOT, nil // nodes wihtout dependencies are systematically rebuilt
	}

	bo := OptionBuildStruct(x.options)
	static := x.graph.BuildMany(Keys(x.static), bo)
	dynamic := x.graph.BuildMany(Keys(x.dynamic), bo)

	// output files are an exception: we can't build them without recursing into this node.
	// to avoid avoid looping (or more likely dead-locking) we don't the nodes for those,
	// but instead we compute directly the digest:
	outputFiles := x.buildOutputFiles()
	// outputFiles := g.BuildMany(Keys(node.OutputFiles), bo, OptionBuildTouch(node))

	var lastError error
	rebuild := false
	dependencyType := DEPENDENCY_ROOT

	if results, err := static.Join().Get(); err == nil {
		if x.static.updateBuild(x.node, DEPENDENCY_STATIC, results) {
			rebuild = true
			dependencyType = DEPENDENCY_STATIC
		}
	} else {
		rebuild = false // wont't rebuild if a static dependency failed
		lastError = buildDependencyError{link: DEPENDENCY_STATIC, outer: err}
	}

	if results, err := dynamic.Join().Get(); err == nil {
		if x.dynamic.updateBuild(x.node, DEPENDENCY_DYNAMIC, results) {
			rebuild = true
			dependencyType = DEPENDENCY_DYNAMIC
		}
	} else {
		rebuild = false // wont't rebuild if a dynamic dependency failed
		lastError = buildDependencyError{link: DEPENDENCY_DYNAMIC, outer: err}
	}

	if results, err := outputFiles.Join().Get(); err == nil {
		if x.outputFiles.updateBuild(x.node, DEPENDENCY_OUTPUT, results) {
			rebuild = true
			dependencyType = DEPENDENCY_OUTPUT
		}
	} else {
		if !x.options.NoWarningOnMissingOutput {
			LogWarning("%v: missing output, trigger rebuild", x.node.Alias())
		}
		rebuild = true // output dependencies can be regenerated
		dependencyType = DEPENDENCY_OUTPUT
	}

	if !rebuild && lastError == nil {
		// check if content fingerprint changed
		buildstamp := x.node.GetBuildStamp()
		checksum := MakeBuildFingerprint(x.node.GetBuildable())
		if rebuild = (checksum != buildstamp.Content); rebuild {
			LogTrace("%v: fingerprint mismatch, trigger rebuild\n\told: %v\n\tnew: %v", x.node.Alias(), buildstamp.Content, checksum)
		}
	}

	return rebuild, dependencyType, lastError
}
func (x *buildExecuteContext) Execute() (BuildResult, bool, error) {
	stats := StartBuildStats()

	needToBuild, _, err := x.needToBuild()
	if err != nil || !(needToBuild || x.options.Force) {
		x.node.state.RLock()
		defer x.node.state.RUnlock()
		return BuildResult{
			Buildable:  x.node.Buildable,
			BuildStamp: x.node.Stamp,
		}, false, err
	}

	if needToBuild {
		x.graph.makeDirty() // need to save the build graph since some dependency was invalidated (even if node own stamp could not change)
	}

	// if x.numDependencies() == 0 {
	// 	LogInfo("%v: always rebuild standalone node", x.node.Alias())
	// } else {
	// 	LogClaim("%v: need to rebuild due to %v dependency", x.node.Alias(), dependencyType)
	// }

	x.node.state.Lock()
	defer func() {
		stats.resumeTimer()
		stats.stopTimer()

		x.stats = stats
		x.node.state.stats.add(stats)
		x.node.state.Unlock()

		x.graph.stats.atomic_add(stats)
	}()

	Assert(func() bool { return x.static.validate(x.node, DEPENDENCY_STATIC) })
	x.node.Static = x.static

	// keep static dependencies untouched, clear everything else
	x.node.makeDirty_assumeLocked()

	stats.resumeTimer()
	err = x.node.Buildable.Build(x)
	stats.pauseTimer()

	if err == nil {
		x.node.Stamp = MakeTimedBuildStamp(x.timestamp, x.node.Buildable)
	}

	Assert(func() bool { return x.node.Alias().Equals(x.node.Buildable.Alias()) })
	Assert(func() bool { return x.node.Dynamic.validate(x.node, DEPENDENCY_DYNAMIC) })
	Assert(func() bool { return x.node.OutputFiles.validate(x.node, DEPENDENCY_OUTPUT) })
	return BuildResult{
		Buildable:  x.node.Buildable,
		BuildStamp: x.node.Stamp,
	}, true, err
}
func (x *buildExecuteContext) OnBuilt(e func(BuildNode) error) {
	// add to parent to trigger the event in outer scope
	x.options.Parent.OnBuilt.Add(e)
}
func (x *buildExecuteContext) Alias() BuildAlias {
	return x.node.Alias()
}
func (x *buildExecuteContext) Options() BuildOptions {
	return x.options
}
func (x *buildExecuteContext) Timestamp(timestamp time.Time) {
	x.barrier.Lock()
	defer x.barrier.Unlock()

	x.timestamp = timestamp
}
func (x *buildExecuteContext) lock_for_dependency() {
	x.barrier.Lock()
	x.stats.pauseTimer()
}
func (x *buildExecuteContext) unlock_for_dependency() {
	x.stats.resumeTimer()
	x.barrier.Unlock()
}
func (x *buildExecuteContext) dependsOn_AssumeLocked(aliases ...BuildAlias) error {
	result := x.graph.BuildMany(aliases, OptionBuildStruct(x.options)).Join()
	if err := result.Failure(); err != nil {
		return err
	}

	for _, it := range result.Success() {
		x.node.addDynamic_AssumeLocked(it.Alias(), it.BuildStamp)
	}
	return nil
}
func (x *buildExecuteContext) DependsOn(aliases ...BuildAlias) error {
	x.lock_for_dependency()
	defer x.unlock_for_dependency()

	return x.dependsOn_AssumeLocked(aliases...)
}
func (x *buildExecuteContext) NeedFactory(factory BuildFactory) (Buildable, error) {
	x.lock_for_dependency()
	defer x.unlock_for_dependency()

	node, err := buildInit(x.graph, factory, x.options)
	if err != nil {
		return nil, err
	}

	alias := node.Alias()
	_, future := x.graph.Build(alias, OptionBuildStruct(x.options))

	if result := future.Join(); result.Failure() == nil {
		x.node.addDynamic_AssumeLocked(alias, result.Success().BuildStamp)
		return result.Success().Buildable, nil
	} else {
		return nil, result.Failure()
	}
}
func (x *buildExecuteContext) NeedFactories(factories ...BuildFactory) error {
	x.lock_for_dependency()
	defer x.unlock_for_dependency()

	aliases := make(BuildAliases, len(factories))
	for i, factory := range factories {
		node, err := buildInit(x.graph, factory, x.options)
		if err != nil {
			return err
		}
		aliases[i] = node.Alias()
	}

	future := x.graph.BuildMany(aliases, OptionBuildStruct(x.options))
	return future.Join().Failure()
}
func (x *buildExecuteContext) NeedBuildable(buildables ...BuildAliasable) error {
	x.lock_for_dependency()
	defer x.unlock_for_dependency()

	return x.dependsOn_AssumeLocked(MakeBuildAliases(buildables...)...)
}
func (x *buildExecuteContext) NeedFile(files ...Filename) error {
	x.lock_for_dependency()
	defer x.unlock_for_dependency()

	aliases := make(BuildAliases, len(files))
	for i, file := range files {
		node, err := buildInit(x.graph, BuildFile(file), x.options)
		if err != nil {
			return err
		}
		aliases[i] = node.Alias()
	}
	return x.dependsOn_AssumeLocked(aliases...)
}
func (x *buildExecuteContext) NeedDirectory(directories ...Directory) error {
	x.lock_for_dependency()
	defer x.unlock_for_dependency()

	aliases := make(BuildAliases, len(directories))
	for i, directory := range directories {
		node, err := buildInit(x.graph, BuildDirectory(directory), x.options)
		if err != nil {
			return err
		}
		aliases[i] = node.Alias()
	}
	return x.dependsOn_AssumeLocked(aliases...)
}
func (x *buildExecuteContext) OutputFile(files ...Filename) error {
	// files are treated as an exception: we build them outside of build scope, without using a future
	x.lock_for_dependency()
	defer x.unlock_for_dependency()

	bo := x.options
	bo.Force = true

	for _, it := range files {
		// create output file with a static dependency pointing to its creator (e.g x.node here)
		file, err := BuildFile(it, x.node.BuildAlias).Init(x.graph, OptionBuildStruct(bo))
		if err != nil {
			return err
		}

		// output files are an exception: we need file build stamp to track external modifications
		// in the creator, but we also need to add a dependency from the creator on the file, creating a recursion.
		// to avoid looping (actually more dead-locking) we compute the build stamp of the file without
		// actually building its node:
		if stamp, err := file.Digest(); err == nil {
			x.node.addOutputFile_AssumeLocked(file.Alias(), stamp)
		} else {
			return err
		}
	}

	return nil
}
func (x *buildExecuteContext) OutputNode(factories ...BuildFactory) error {
	x.lock_for_dependency()
	defer x.unlock_for_dependency()

	bo := x.options
	bo.Force = true // force update freshly outputed nodes

	for _, it := range factories {
		factory := it

		node, err := buildInit(x.graph, MakeBuildFactory(func(init BuildInitializer) (Buildable, error) {
			if err := init.DependsOn(x.node.Alias()); err != nil { // add caller node as a static dependency
				return nil, err
			}
			return factory.Create(init)
		}), bo)
		if err != nil {
			return err
		}

		LogDebug("%v: outputs node %q", x.Alias(), node.Alias())
		x.node.addOutputNode_AssumeLocked(node.Alias())
	}

	return nil
}

/***************************************
 * Build Graph
 ***************************************/

type buildEvents struct {
	onBuildGraphStartEvent    ConcurrentEvent[BuildGraph]
	onBuildGraphFinishedEvent ConcurrentEvent[BuildGraph]

	onBuildNodeStartEvent    ConcurrentEvent[BuildNode]
	onBuildNodeFinishedEvent ConcurrentEvent[BuildNode]

	barrier         sync.Mutex
	pbar            atomic.Pointer[pinnedLogProgress]
	numRunningTasks atomic.Int32
}

type buildGraph struct {
	flags    *CommandFlags
	nodes    SharedMapT[BuildAlias, *buildNode]
	options  BuildOptions
	stats    BuildStats
	revision int32

	buildEvents
}

func NewBuildGraph(flags *CommandFlags, options ...BuildOptionFunc) BuildGraph {
	result := &buildGraph{
		flags:    flags,
		nodes:    SharedMapT[BuildAlias, *buildNode]{sync.Map{}},
		options:  NewBuildOptions(options...),
		revision: 0,
	}
	return result
}

func (g *buildGraph) Aliases() []BuildAlias {
	keys := g.nodes.Keys()
	sort.Slice(keys, func(i, j int) bool {
		return keys[i] < keys[j]
	})
	return keys
}
func (g *buildGraph) Dirty() bool {
	return atomic.LoadInt32(&g.revision) > 0
}
func (g *buildGraph) Find(alias BuildAlias) (result BuildNode) {
	if node, _ := g.nodes.Get(alias); node != nil {
		Assert(func() bool { return node.Alias().Equals(alias) })
		AssertMessage(func() bool { return node.Buildable.Alias().Equals(alias) }, "%v: node alias do not match buildable -> %q",
			alias, MakeStringer(func() string {
				return node.Buildable.Alias().String()
			}))

		result = node
	}
	return
}
func (g *buildGraph) Create(buildable Buildable, static BuildAliases, options ...BuildOptionFunc) BuildNode {
	bo := NewBuildOptions(options...)

	var node *buildNode
	var loaded bool

	alias := buildable.Alias()
	AssertMessage(func() bool { return alias.Valid() }, "invalid alias for <%T>", buildable)

	if node, loaded = g.nodes.Get(alias); loaded {
		if !bo.Force {
			return node
		}
	} else {
		// first optimistic Get() to avoid newBuildNode() bellow
		node, loaded = g.nodes.FindOrAdd(alias, newBuildNode(alias, buildable))
	}

	LogDebug("%v: create <%v> node%v", alias,
		MakeStringer(func() string { return reflect.TypeOf(node.GetBuildable()).String() }),
		MakeStringer(func() string { return Blend("", " (forced update)", bo.Force) }))

	node.state.Lock()
	defer node.state.Unlock()

	Assert(func() bool { return alias == node.Alias() })
	AssertSameType(node.Buildable, buildable)

	LogPanicIfFailed(bo.OnLaunched.Invoke(node))

	dirty := !loaded || bo.Force || len(static) != len(node.Static)
	oldStaticDeps := node.Static

	node.Buildable = buildable
	node.Static = make(BuildDependencies, len(static))

	for _, a := range static {
		old, hit := oldStaticDeps[a]
		node.addStatic_AssumeLocked(a, old)
		dirty = dirty || !hit
	}

	if dirty {
		LogPanicIfFailed(bo.OnBuilt.Invoke(node))

		node.makeDirty_assumeLocked()
		g.makeDirty()

		// node just went through a full reset -> forget cached future if any
		node.future.Reset()
	}

	AssertMessage(func() bool { return node.Buildable.Alias().Equals(alias) }, "%v: node alias do not match buildable -> %q",
		alias, MakeStringer(func() string {
			return node.Buildable.Alias().String()
		}))
	return node
}
func (g *buildGraph) Build(it BuildAliasable, options ...BuildOptionFunc) (BuildNode, Future[BuildResult]) {
	a := it.Alias()
	AssertMessage(func() bool { return a.Valid() }, "invalid alias for <%T>", it)

	if node, ok := g.nodes.Get(a); ok {
		Assert(func() bool { return node.Alias().Equals(a) })
		AssertMessage(func() bool { return node.Buildable.Alias().Equals(a) }, "%v: node alias do not match buildable -> %q",
			a, MakeStringer(func() string {
				return node.Buildable.Alias().String()
			}))

		bo := NewBuildOptions(options...)
		return node, g.launchBuild(node, bo)
	} else {
		return nil, MakeFutureError[BuildResult](fmt.Errorf("build: unknown node %q", a))
	}
}
func (g *buildGraph) BuildMany(targets BuildAliases, options ...BuildOptionFunc) (result Future[[]BuildResult]) {
	switch len(targets) {
	case 0:
		return MakeFutureLiteral([]BuildResult{})
	case 1:
		alias := targets[0]
		_, future := g.Build(alias, options...)

		return MapFuture(future, func(it BuildResult) (result []BuildResult) {
			return []BuildResult{it}
		})
	default:
		return MakeFuture(func() (results []BuildResult, err error) {
			results = make([]BuildResult, len(targets))

			bo := NewBuildOptions(options...)
			boStruct := OptionBuildStruct(bo)

			err = ParallelJoin(
				func(i int, it BuildResult) error {
					Assert(func() bool { return it.Content.Valid() })
					results[i] = it
					return nil
				},
				Map(func(alias BuildAlias) Future[BuildResult] {
					_, future := g.Build(alias, boStruct)
					return future
				}, targets...)...)

			return
		}, MakeStringer(func() string {
			return fmt.Sprintf("buildmany: %s", strings.Join(Stringize(targets...), ", "))
		}))
	}
}
func (g *buildGraph) Join() (lastErr error) {
	g.nodes.Range(func(alias BuildAlias, node *buildNode) {
		if node != nil {
			future := node.future.Load()
			if future != nil {
				result := future.Join()
				if err := result.Failure(); err != nil {
					lastErr = err
				}
			}
		}
	})
	return lastErr
}
func (g *buildGraph) PostLoad() {
	if g.flags.Purge.Get() {
		g.revision = 0
		g.nodes.Clear()
		g.makeDirty()
	}
}
func (g *buildGraph) Serialize(ar Archive) {
	var pinned []*buildNode
	serialize := func(node **buildNode) {
		*node = &buildNode{}
		ar.Serializable(*node)
	}
	if !ar.Flags().IsLoading() {
		serialize = func(node **buildNode) {
			ar.Serializable(*node)
		}
		pinned = g.nodes.Values()
		sort.Slice(pinned, func(i, j int) bool {
			return pinned[i].BuildAlias.Compare(pinned[j].BuildAlias) < 0
		})
	}
	SerializeMany(ar, serialize, &pinned)
	if ar.Flags().IsLoading() && ar.Error() == nil {
		g.nodes.Clear()
		for _, node := range pinned {
			g.nodes.Add(node.Alias(), node)
		}
	}
}
func (g *buildGraph) Save(dst io.Writer) error {
	g.revision = 0
	return CompressedArchiveFileWrite(dst, g.Serialize)
}
func (g *buildGraph) Load(src io.Reader) error {
	g.revision = 0
	file, err := CompressedArchiveFileRead(src, g.Serialize)
	LogVeryVerbose("buildgraph: archive version = %v tags = %v", file.Version, file.Tags)
	return err
}
func (g *buildGraph) Equals(other BuildGraph) bool {
	return other.(*buildGraph) == g
}
func (g *buildGraph) GetStaticDependencies(root BuildNode) (result []BuildNode) {
	aliases := root.GetStaticDependencies()
	result = make([]BuildNode, len(aliases))
	for i, alias := range aliases {
		result[i] = g.Find(alias)
	}
	return
}
func (g *buildGraph) GetDynamicDependencies(root BuildNode) (result []BuildNode) {
	aliases := root.GetDynamicDependencies()
	result = make([]BuildNode, len(aliases))
	for i, alias := range aliases {
		result[i] = g.Find(alias)
	}
	return
}
func (g *buildGraph) GetOutputDependencies(root BuildNode) (result []BuildNode) {
	aliases := root.GetOutputDependencies()
	result = make([]BuildNode, len(aliases))
	for i, alias := range aliases {
		result[i] = g.Find(alias)
	}
	return
}
func (g *buildGraph) GetDependencyLinks(a BuildAlias) ([]BuildDependencyLink, error) {
	if node := g.Find(a); node != nil {
		return node.GetDependencyLinks(), nil
	} else {
		return []BuildDependencyLink{}, fmt.Errorf("buildgraph: build node %q not found", a)
	}
}
func (g *buildGraph) GetDependencyInputFiles(alias BuildAlias) (FileSet, error) {
	var files FileSet

	queue := make([]BuildAlias, 0, 32)
	queue = append(queue, alias)

	visiteds := make(map[BuildAlias]int)
	visit := func(node *buildNode) {
		node.state.RLock()
		defer node.state.RUnlock()

		switch file := node.Buildable.(type) {
		case *Filename:
			files.AppendUniq(*file)
		}

		for a := range node.Static {
			if _, ok := visiteds[a]; !ok {
				visiteds[a] = 1
				queue = append(queue, a)
			}
		}
		for a := range node.Dynamic {
			if _, ok := visiteds[a]; !ok {
				visiteds[a] = 1
				queue = append(queue, a)
			}
		}
	}

	for {
		if len(queue) == 0 {
			break
		}

		a := queue[len(queue)-1]
		queue = queue[:len(queue)-1]

		node, _ := g.nodes.Get(a)
		if node == nil {
			return files, fmt.Errorf("buildgraph: build node %q not found", a)
		}

		visit(node)
	}

	return files, nil
}
func (g *buildGraph) GetDependencyChain(src, dst BuildAlias) ([]BuildDependencyLink, error) {
	// https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm#:~:text=in%20some%20topologies.-,Pseudocode,-%5Bedit%5D

	const INDEX_NONE int32 = -1

	vertices := Keys(g.nodes.Pin())
	previous := make([]int32, len(vertices))
	visiteds := make(map[BuildAlias]int32, len(vertices))
	distances := make([]int32, len(vertices))
	linkTypes := make([]BuildDependencyType, len(vertices))

	dstIndex := INDEX_NONE
	for i, a := range vertices {
		visiteds[a] = int32(i)
		distances[i] = math.MaxInt32
		previous[i] = INDEX_NONE
		linkTypes[i] = DEPENDENCY_ROOT

		if a == src {
			distances[i] = 0
		} else if a == dst {
			dstIndex = int32(i)
		}
	}

	for len(visiteds) > 0 {
		min := INDEX_NONE
		for _, i := range visiteds {
			if min < 0 || distances[i] < distances[min] {
				min = int32(i)
			}
		}

		u := vertices[min]
		delete(visiteds, u)

		links, err := g.GetDependencyLinks(u)
		if err != nil {
			return []BuildDependencyLink{}, err
		}

		for _, l := range links {
			v := l.Alias
			if j, ok := visiteds[v]; ok {
				alt := distances[min] + int32(l.Type) // weight by link type, favor output > static > dynamic
				if alt < distances[j] {
					distances[j] = alt
					previous[j] = min
					linkTypes[j] = l.Type
				}
			}
		}
	}

	chain := make([]BuildDependencyLink, distances[dstIndex]+1)
	chain[0] = BuildDependencyLink{
		Alias: dst,
		Type:  DEPENDENCY_ROOT,
	}

	next := dstIndex
	for i := int32(0); i < distances[dstIndex]; i += 1 {
		next = previous[next]
		chain[i+1] = BuildDependencyLink{
			Alias: vertices[next],
			Type:  linkTypes[next],
		}
	}

	return chain, nil
}
func (g *buildGraph) GetBuildStats() BuildStats {
	return g.stats
}
func (g *buildGraph) GetMostExpansiveNodes(n int, inclusive bool) (results []BuildNode) {
	results = make([]BuildNode, 0, n+1)

	predicate := func(i, j int) bool {
		return results[i].GetBuildStats().Duration.Exclusive > results[j].GetBuildStats().Duration.Exclusive
	}
	if inclusive {
		predicate = func(i, j int) bool {
			return results[i].GetBuildStats().Duration.Inclusive > results[j].GetBuildStats().Duration.Inclusive
		}
	}

	for _, node := range g.nodes.Pin() {
		if node.state.stats.Count == 0 {
			continue // unbuilt
		}
		results = append(results, node)
		sort.Slice(results, predicate)
		if len(results) > n {
			results = results[:n]
		}
	}
	return
}

func (g *buildGraph) makeDirty() {
	atomic.AddInt32(&g.revision, 1)
}
func (g *buildGraph) launchBuild(node *buildNode, options BuildOptions) Future[BuildResult] {
	Assert(func() bool {
		relateOutp := strings.Builder{}
		if options.RelatesVerbose(node, 0, &relateOutp) {
			LogPanic("buildgraph: build cyclic dependency in %q\n%s", node, relateOutp.String())
			return false
		}
		return true
	})
	// relateOutp := strings.Builder{}
	// if options.RelatesVerbose(node, 0, &relateOutp) {
	// 	LogPanic("buildgraph: build cyclic dependency in %q\n%s", node, relateOutp.String())
	// }
	// LogTrace("buildgraph: launch build of <%T> %q\n%s", node.Buildable, node.Alias(), relateOutp.String())

	var future = node.future.Load()
	if future != nil && !options.Force {
		return future
	}

	node.state.Lock()
	defer node.state.Unlock()

	if other := node.future.Load(); other != nil && other != future { // check if another thread already launched the node
		future = other
		return future
	} else {
		future = other
	}

	LogDebug("%v: launch <%v> build%v", node.Alias(),
		MakeStringer(func() string { return reflect.TypeOf(node.Buildable).String() }),
		MakeStringer(func() string { return Blend("", " (forced update)", options.Force) }))

	if future != nil {
		future.Join()
	}

	newFuture := MakeFuture(func() (BuildResult, error) {
		Assert(func() bool { return node.Alias().Equals(node.Buildable.Alias()) })

		g.onBuildNodeStart(g, node)
		defer g.onBuildNodeFinished(g, node)

		context := makeBuildExecuteContext(g, node, options.Recurse(node))
		result, built, err := context.Execute()

		if err == nil && built {
			err = options.OnBuilt.Invoke(node)
		}

		if err == nil {
			if built {
				if result.BuildStamp != context.stamp {
					if context.numDependencies() > 0 || IsLogLevelActive(LOG_VERYVERBOSE) {
						LogInfo("%s%s%s %q (%v)",
							Blend("", "force ", options.Force),
							Blend("build", "update", context.stamp.Content.Valid()),
							Blend("", " standalone", context.numDependencies() == 0),
							node.BuildAlias, node.state.stats.Duration.Exclusive)
					}

					LogTrace("%v: new build stamp for [%T]\n\tnew: %v\n\told: %v", node.BuildAlias, result.Buildable, result.BuildStamp, context.stamp)

					g.makeDirty()

				} else {
					LogDebug("unchanged %q (%v)", node.BuildAlias, node.state.stats.Duration.Exclusive)
				}
			} else {
				LogVerbose("up-to-date %q", node.BuildAlias)
			}
		} else {
			switch err.(type) {
			case buildDependencyError:
			default: // failed dependency errors are only printed once
				LogError("%v: %v", node.BuildAlias, err)
			}
		}

		return result, err
	})

	node.future.Store(newFuture)
	LogPanicIfFailed(options.OnLaunched.Invoke(node))

	return newFuture
}

/***************************************
 * Build Events
 ***************************************/

func (g *buildEvents) onBuildGraphStart_ThreadSafe() *pinnedLogProgress {
	g.barrier.Lock()
	defer g.barrier.Unlock()
	if pbar := g.pbar.Load(); pbar == nil {
		pbar := LogSpinner("Build Graph ")
		if concrete, ok := pbar.(*pinnedLogProgress); ok {
			g.pbar.Store(concrete)
			return concrete
		}
	} else {
		return pbar
	}
	return nil
}
func (g *buildEvents) onBuildGraphFinished_ThreadSafe() {
	g.barrier.Lock()
	defer g.barrier.Unlock()
	if pbar := g.pbar.Load(); pbar != nil {
		pbar.Close()
		g.pbar.Store(nil)
	}
}

func (g *buildEvents) onBuildNodeStart(graph *buildGraph, node *buildNode) {
	if g.numRunningTasks.Add(1) == 1 {
		g.onBuildGraphStartEvent.Invoke(graph)
	}

	g.onBuildNodeStartEvent.Invoke(node)

	if enableInteractiveShell {
		pbar := g.pbar.Load()
		if pbar == nil {
			pbar = g.onBuildGraphStart_ThreadSafe()
		}
		// if pbar != nil {
		// 	pbar.Grow(1)
		// }
	}
}
func (g *buildEvents) onBuildNodeFinished(graph *buildGraph, node *buildNode) {
	g.onBuildNodeFinishedEvent.Invoke(node)

	if g.numRunningTasks.Add(-1) == 0 {
		g.onBuildGraphFinishedEvent.Invoke(graph)

		g.onBuildGraphFinished_ThreadSafe()
	}

	if enableInteractiveShell {
		if pbar := g.pbar.Load(); pbar != nil {
			pbar.Inc()
		}
	}
}

func (g *buildEvents) OnBuildGraphStart() MutableEvent[BuildGraph] {
	return &g.onBuildGraphStartEvent
}
func (g *buildEvents) OnBuildGraphFinished() MutableEvent[BuildGraph] {
	return &g.onBuildGraphFinishedEvent
}

func (g *buildEvents) OnBuildNodeStart() MutableEvent[BuildNode] {
	return &g.onBuildNodeStartEvent
}
func (g *buildEvents) OnBuildNodeFinished() MutableEvent[BuildNode] {
	return &g.onBuildNodeStartEvent
}

/***************************************
 * Build Alias
 ***************************************/

func MakeBuildAliases[T BuildAliasable](targets ...T) (result BuildAliases) {
	result = make(BuildAliases, len(targets))
	for i, it := range targets {
		result[i] = it.Alias()
	}
	return result
}

func FindBuildAliases(bg BuildGraph, category string, names ...string) (result BuildAliases) {
	prefix := MakeBuildAlias(category, names...).String()
	for _, a := range bg.Aliases() {
		if strings.HasPrefix(a.String(), prefix) {
			result.Append(a)
		}
	}
	return
}

func MakeBuildAlias(category string, names ...string) BuildAlias {
	sb := strings.Builder{}
	sep := "://"

	capacity := len(category)
	for i, it := range names {
		if i > 0 {
			capacity += 1
		} else {
			capacity += len(sep)
		}
		capacity += len(it)
	}
	sb.Grow(capacity)

	sb.WriteString(category)
	for i, it := range names {
		if i > 0 {
			sb.WriteRune('/')
		} else {
			sb.WriteString(sep)
		}
		BuildSanitizedPath(&sb, it, '/')
	}

	return BuildAlias(sb.String())
}
func (x BuildAlias) Alias() BuildAlias { return x }
func (x BuildAlias) Valid() bool       { return len(x) > 3 /* check for "---" */ }
func (x BuildAlias) Equals(o BuildAlias) bool {
	return string(x) == string(o)
}
func (x BuildAlias) Compare(o BuildAlias) int {
	return strings.Compare(string(x), string(o))
}
func (x BuildAlias) String() string {
	Assert(func() bool { return x.Valid() })
	return string(x)
}
func (x *BuildAlias) Set(in string) error {
	Assert(func() bool { return x.Valid() })
	*x = BuildAlias(in)
	return nil
}
func (x *BuildAlias) Serialize(ar Archive) {
	ar.String((*string)(x))
}
func (x BuildAlias) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *BuildAlias) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

/***************************************
 * Build Stamp
 ***************************************/

func MakeBuildFingerprint(buildable Buildable) (result Fingerprint) {
	result = SerializeFingerpint(buildable, GetProcessSeed())
	if !result.Valid() {
		LogPanic("buildgraph: invalid buildstamp for %q", buildable.Alias())
	}
	return
}
func MakeTimedBuildStamp(modTime time.Time, buildable Buildable) BuildStamp {
	stamp := BuildStamp{
		// round up timestamp to millisecond, see ArchiveBinaryReader/Writer.Time()
		ModTime: time.UnixMilli(modTime.UnixMilli()),
		Content: MakeBuildFingerprint(buildable),
	}
	Assert(func() bool { return stamp.Content.Valid() })
	return stamp
}

func (x BuildStamp) String() string {
	return fmt.Sprintf("[%v] %v", x.Content.ShortString(), x.ModTime.Local().Format(time.Stamp))
}
func (x *BuildStamp) Serialize(ar Archive) {
	ar.Time(&x.ModTime)
	ar.Serializable(&x.Content)
}

/***************************************
 * Build Options
 ***************************************/

func NewBuildOptions(options ...BuildOptionFunc) (result BuildOptions) {
	result.Init(options...)
	return
}
func (x *BuildOptions) Init(options ...BuildOptionFunc) {
	for _, it := range options {
		it(x)
	}
}
func (x *BuildOptions) Recurse(node BuildNode) (result BuildOptions) {
	AssertMessage(func() bool { return x.Caller == nil || x.Caller != node }, "build graph: invalid build recursion on %q\n%v", node, x)
	AssertMessage(func() bool { return node == nil || node.Alias().Valid() }, "build graph: invalid build alias on %q\n%v", node, x)

	result.Parent = x
	result.Caller = node
	result.OnBuilt = x.OnBuilt
	result.OnLaunched = x.OnLaunched
	result.NoWarningOnMissingOutput = x.NoWarningOnMissingOutput

	if x.Recursive {
		result.Force = x.Force
		result.Recursive = x.Recursive
	}

	return
}
func (x *BuildOptions) Touch(parent BuildNode) (result BuildOptions) {
	Assert(func() bool { return x.Caller == parent })
	x.Stamp = &x.Caller.(*buildNode).Stamp
	return
}
func (x BuildOptions) DependencyChain() (result []BuildNode) {
	result = []BuildNode{x.Caller}
	if x.Parent != nil {
		result = append(result, x.Parent.DependencyChain()...)
	}
	return
}
func (x BuildOptions) String() string {
	sb := strings.Builder{}
	x.RelatesVerbose(nil, 0, &sb)
	return sb.String()
}
func (x BuildOptions) HasOuter(node BuildNode) *BuildStamp {
	if x.Caller == node {
		return x.Stamp
	} else if x.Parent != nil {
		return x.Parent.HasOuter(node)
	}
	return nil
}
func (x BuildOptions) RelatesVerbose(node BuildNode, depth int, outp *strings.Builder) bool {
	indent := "  "
	if depth == 0 && node != nil {
		fmt.Fprintf(outp, "%s%d) %s\n", strings.Repeat(indent, depth), depth, node.Alias())
		depth += 1
	}
	if x.Caller != nil {
		if x.Stamp == nil {
			fmt.Fprintf(outp, "%s%d) %s\n", strings.Repeat(indent, depth), depth, x.Caller.Alias())
		} else {
			fmt.Fprintf(outp, "%s%d) %s - [OUTER:%s]\n", strings.Repeat(indent, depth), depth, x.Caller.Alias(), x.Stamp.String())
		}
	}
	if depth > 20 {
		LogPanic("buildgraph: node stack too deep!\n%v", outp)
	}

	var result bool
	if x.Caller == node && x.Stamp == nil /* if Caller has a Stamp then it is the outer of 'node' */ {
		result = true
	} else if x.Parent != nil {
		result = x.Parent.RelatesVerbose(node, depth+1, outp)
	} else {
		result = false
	}

	return result
}

func OptionBuildCaller(node BuildNode) BuildOptionFunc {
	return func(opts *BuildOptions) {
		opts.Caller = node
	}
}
func OptionBuildTouch(node BuildNode) BuildOptionFunc {
	return func(opts *BuildOptions) {
		opts.Touch(node)
	}
}
func OptionBuildForce(opts *BuildOptions) {
	opts.Force = true
}
func OptionBuildForceIf(force bool) BuildOptionFunc {
	return func(opts *BuildOptions) {
		opts.Force = force
	}
}
func OptionBuildForceRecursive(opts *BuildOptions) {
	opts.Force = true
	opts.Recursive = true
}
func OptionNoWarningOnMissingOutput(opts *BuildOptions) {
	opts.NoWarningOnMissingOutput = true
}
func OptionWarningOnMissingOutputIf(warn bool) BuildOptionFunc {
	return func(bo *BuildOptions) {
		bo.NoWarningOnMissingOutput = !warn
	}
}
func OptionBuildOnLaunched(event func(BuildNode) error) BuildOptionFunc {
	return func(opts *BuildOptions) {
		opts.OnLaunched.Add(event)
	}
}
func OptionBuildOnBuilt(event func(BuildNode) error) BuildOptionFunc {
	return func(opts *BuildOptions) {
		opts.OnBuilt.Add(event)
	}
}
func OptionBuildParent(parent *BuildOptions) BuildOptionFunc {
	return func(opts *BuildOptions) {
		opts.Parent = parent
	}
}
func OptionBuildStruct(value BuildOptions) BuildOptionFunc {
	return func(opts *BuildOptions) {
		*opts = value
	}
}

/***************************************
 * Build Dependencies
 ***************************************/

func (x *BuildDependencyType) Combine(y BuildDependencyType) {
	if *x > y || *x == DEPENDENCY_ROOT {
		*x = y
	}
}

func (x BuildDependencyType) String() string {
	switch x {
	case DEPENDENCY_STATIC:
		return "STATIC"
	case DEPENDENCY_DYNAMIC:
		return "DYNAMIC"
	case DEPENDENCY_OUTPUT:
		return "OUTPUT"
	case DEPENDENCY_ROOT:
		return "ROOT"
	default:
		UnexpectedValuePanic(x, x)
		return ""
	}
}
func (x *BuildDependencyType) Set(in string) error {
	switch strings.ToUpper(in) {
	case DEPENDENCY_STATIC.String():
		*x = DEPENDENCY_STATIC
	case DEPENDENCY_DYNAMIC.String():
		*x = DEPENDENCY_DYNAMIC
	case DEPENDENCY_OUTPUT.String():
		*x = DEPENDENCY_OUTPUT
	case DEPENDENCY_ROOT.String():
		*x = DEPENDENCY_ROOT
	default:
		return MakeUnexpectedValueError(x, in)
	}
	return nil
}

func (deps *BuildDependencies) Serialize(ar Archive) {
	SerializeMap(ar, (*map[BuildAlias]BuildStamp)(deps))
}
func (deps BuildDependencies) validate(owner BuildNode, depType BuildDependencyType) bool {
	valid := true
	for a, stamp := range deps {
		if !stamp.Content.Valid() {
			valid = false
			LogError("%v: %s dependency <%v> has an invalid build stamp (%v)", depType, owner.Alias(), a, stamp)
		}
	}
	return valid
}
func (deps *BuildDependencies) updateBuild(owner BuildNode, depType BuildDependencyType, results []BuildResult) (rebuild bool) {
	Assert(func() bool { return len(results) == len(*deps) })

	for _, result := range results {
		alias := result.Alias()
		Assert(func() bool { return result.Content.Valid() })

		oldStamp, ok := (*deps)[alias]
		AssertIn(ok, true)

		if oldStamp != result.BuildStamp {
			LogTrace("%v: %v dependency <%v> has been updated:\n\tnew: %v\n\told: %v", owner.Alias(), depType, alias, result.BuildStamp, oldStamp)

			(*deps)[alias] = result.BuildStamp
			rebuild = true
		} // else // LogDebug("%v: %v %v dependency is up-to-date", owner.Alias(), alias, depType)
	}

	return rebuild
}

/***************************************
 * Build Stats
 ***************************************/

func StartBuildStats() (result BuildStats) {
	result.startTimer()
	return
}
func (x *BuildStats) Append(other BuildStats) {
	other.stopTimer()
	x.atomic_add(other)
}

func (x *BuildStats) atomic_add(other BuildStats) {
	if expected := x.Count + other.Count; atomic.AddInt32(&x.Count, other.Count) == expected {
		x.InclusiveStart = other.InclusiveStart
		x.ExclusiveStart = other.ExclusiveStart
	}

	atomic.AddInt64((*int64)(&x.Duration.Inclusive), int64(other.Duration.Inclusive))
	atomic.AddInt64((*int64)(&x.Duration.Exclusive), int64(other.Duration.Exclusive))
}
func (x *BuildStats) add(other BuildStats) {
	if x.Count == 0 {
		x.InclusiveStart = other.InclusiveStart
		x.ExclusiveStart = other.ExclusiveStart
	}

	x.Count += other.Count
	x.Duration.Inclusive += other.Duration.Inclusive
	x.Duration.Exclusive += other.Duration.Exclusive
}
func (x *BuildStats) startTimer() {
	x.Count += 1
	x.InclusiveStart = time.Now()
	x.ExclusiveStart = x.InclusiveStart
}
func (x *BuildStats) stopTimer() {
	x.Duration.Inclusive += time.Since(x.InclusiveStart)
	x.Duration.Exclusive += time.Since(x.ExclusiveStart)
}
func (x *BuildStats) pauseTimer() {
	x.Duration.Exclusive += time.Since(x.ExclusiveStart)
}
func (x *BuildStats) resumeTimer() {
	x.ExclusiveStart = time.Now()
}
