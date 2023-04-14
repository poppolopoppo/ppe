package cmd

import (
	. "build/utils"
	"fmt"
	"io"
	"reflect"
)

var CommandGraphviz = NewCommand(
	"Export",
	"export-gvz",
	"dump build node to graphviz .dot",
	OptionCommandCompletionArgs(),
	OptionCommandRun(func(cc CommandContext) error {
		bg := CommandEnv.BuildGraph()
		aliases := bg.Aliases()
		args := GetCompletionArgs()

		completion := make(map[string]BuildAlias, len(aliases))
		for _, a := range aliases {
			completion[a.String()] = a
		}

		results := make(map[BuildAlias]BuildNode, 8)
		mapCompletion(args, func(s string) {
			alias := completion[s]
			results[alias] = bg.Find(alias)
		}, completion)

		return openCompletion(args, func(w io.Writer) error {
			gvz := newBuildGraphViz(bg, w)
			gvz.Digraph("G", func() {
				for _, node := range results {
					gvz.Visit(node, OptionGraphVizFontSize(36), OptionGraphVizFillColor("red"), OptionGraphVizFontColor("yellow"), OptionGraphVizScale(2))
				}
			}, OptionGraphVizScale(10), OptionGraphVizFontName("Helvetica,Arial,sans-serif"), OptionGraphVizFontSize(9))
			return nil
		})
	}))

type buildGraphViz struct {
	GraphVizFile
	graph   BuildGraph
	visited map[BuildNode]string
}

func newBuildGraphViz(graph BuildGraph, w io.Writer) buildGraphViz {
	return buildGraphViz{
		graph:        graph,
		GraphVizFile: NewGraphVizFile(w),
		visited:      make(map[BuildNode]string),
	}
}
func (x *buildGraphViz) Visit(node BuildNode, userOptions ...GraphVizOptionFunc) string {
	if id, ok := x.visited[node]; ok {
		return id
	}

	id := node.Alias().String()
	x.visited[node] = id

	options := GraphVizOptions{}
	switch buildable := node.GetBuildable().(type) {
	case *Filename:
		options.Color = "#AAE4B580"
		options.Shape = GRAPHVIZ_Note
		options.FontSize = 7
	case *Directory:
		options.Color = "#AAFACD80"
		options.Shape = GRAPHVIZ_Folder
		options.FontSize = 7
	case *DirectoryCreator, *DirectoryGlob, *DirectoryList:
		options.Color = "#7B68EE50"
		options.Shape = GRAPHVIZ_Component
		options.FontSize = 7
	default:
		ty := reflect.TypeOf(buildable)
		digest := StringFingeprint(ty.String())

		hsl := HSL{
			H: float64(digest[len(digest)-1]) / 0xFF,
			S: 0.7,
			L: 0.8,
		}

		options.Color = fmt.Sprintf("#%s70", hsl.ToHTML())
		options.Style = GRAPHVIZ_Filled
		options.Shape = GRAPHVIZ_Cds
	}

	options.Init(userOptions...)
	x.Node(id, OptionGraphVizOptions(&options))

	for _, dep := range x.graph.GetStaticDependencies(node) {
		x.Edge(id, x.Visit(dep), OptionGraphVizColor("#1E90FF80"), OptionGraphVizStyle(GRAPHVIZ_Dashed), OptionGraphVizWeight(2))
	}
	for _, dep := range x.graph.GetDynamicDependencies(node) {
		x.Edge(id, x.Visit(dep), OptionGraphVizColor("#00FA9A80"), OptionGraphVizStyle(GRAPHVIZ_Dashed), OptionGraphVizWeight(1))
	}
	for _, dep := range x.graph.GetOutputDependencies(node) {
		x.Edge(id, x.Visit(dep), OptionGraphVizColor("#F4A46090"), OptionGraphVizWeight(3))
	}

	return id
}
