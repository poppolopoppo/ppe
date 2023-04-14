package utils

import (
	"fmt"
	"io"
)

// https://graphviz.org/doc/info/lang.html

type GraphVizFile struct {
	*StructuredFile
	Directed bool
}

type GraphVizShape string //
var GRAPHVIZ_Box GraphVizShape = "box"
var GRAPHVIZ_Box3d GraphVizShape = "box3d"
var GRAPHVIZ_Cds GraphVizShape = "cds"
var GRAPHVIZ_Circle GraphVizShape = "circle"
var GRAPHVIZ_Component GraphVizShape = "component"
var GRAPHVIZ_Ellipse GraphVizShape = "ellipse"
var GRAPHVIZ_Folder GraphVizShape = "folder"
var GRAPHVIZ_Mcircle GraphVizShape = "Mcircle"
var GRAPHVIZ_Mdiamond GraphVizShape = "Mdiamond"
var GRAPHVIZ_Msquare GraphVizShape = "Msquare"
var GRAPHVIZ_Note GraphVizShape = "note"
var GRAPHVIZ_Octagon GraphVizShape = "doubleoctagon"
var GRAPHVIZ_Plain GraphVizShape = "plain"
var GRAPHVIZ_Triangle GraphVizShape = "triangle"

type GraphVizStyle string // https://graphviz.org/docs/attr-types/style/
var GRAPHVIZ_Dashed GraphVizStyle = "dashed"
var GRAPHVIZ_Dotted GraphVizStyle = "dotted"
var GRAPHVIZ_Filled GraphVizStyle = "filled"
var GRAPHVIZ_Solid GraphVizStyle = "solid"
var GRAPHVIZ_Bold GraphVizStyle = "bold"

type GraphVizAttributes struct {
	BgColor    string
	Color      string
	FillColor  string
	Label      string
	FontColor  string
	FontName   string
	FontSize   InheritableInt
	Shape      GraphVizShape
	Style      GraphVizStyle
	Scale      InheritableInt
	Constraint InheritableBool
	Weight     InheritableInt
}

type GraphVizOptions struct {
	GraphVizAttributes
}

type GraphVizOptionFunc func(*GraphVizOptions)

func NewGraphVizOptions(options ...GraphVizOptionFunc) (result GraphVizOptions) {
	result.Init(options...)
	return
}

func (gvo *GraphVizOptions) Init(options ...GraphVizOptionFunc) {
	for _, it := range options {
		it(gvo)
	}
}

func OptionGraphVizOptions(options *GraphVizOptions) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		*gvo = *options
	}
}
func OptionGraphVizAttributes(value GraphVizAttributes) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.GraphVizAttributes = value
	}
}
func OptionGraphVizBgColor(value string) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.BgColor = value
	}
}
func OptionGraphVizColor(value string) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.Color = value
	}
}
func OptionGraphVizFillColor(value string) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.FillColor = value
	}
}
func OptionGraphVizFontColor(value string) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.FontColor = value
	}
}
func OptionGraphVizFontName(value string) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.FontName = value
	}
}
func OptionGraphVizFontSize(value int) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.FontSize.Assign(value)
	}
}
func OptionGraphVizLabel(format string, args ...interface{}) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.Label = fmt.Sprintf(format, args...)
	}
}
func OptionGraphVizScale(value int) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.Scale.Assign(value)
	}
}
func OptionGraphVizShape(value GraphVizShape) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.Shape = value
	}
}
func OptionGraphVizStyle(value GraphVizStyle) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.Style = value
	}
}
func OptionGraphVizConstraint(value bool) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.Constraint.Assign(value)
	}
}
func OptionGraphVizWeight(value int) GraphVizOptionFunc {
	return func(gvo *GraphVizOptions) {
		gvo.Weight.Assign(value)
	}
}

func NewGraphVizFile(dst io.Writer) (result GraphVizFile) {
	result = GraphVizFile{
		StructuredFile: NewStructuredFile(dst, "  ", false),
		Directed:       false,
	}
	return
}

func (gvz *GraphVizFile) Digraph(id string, inner func(), options ...GraphVizOptionFunc) {
	gvz.Directed = true
	gvz.Print("digraph %s ", id)
	gvz.Scope(func() {
		gvz.Style(options...)
		gvz.Println("")
		inner()
	})
}
func (gvz *GraphVizFile) Graph(id string, inner func(), options ...GraphVizOptionFunc) {
	gvz.Directed = false
	gvz.Print("graph %s ", id)
	gvz.Scope(func() {
		gvz.Style(options...)
		gvz.Println("")
		inner()
	})
}
func (gvz *GraphVizFile) NodeStyle(options ...GraphVizOptionFunc) {
	gvz.Print("node [")
	gvz.Style(options...)
	gvz.Println("]")
}
func (gvz *GraphVizFile) EdgeStyle(options ...GraphVizOptionFunc) {
	gvz.Print("edge [")
	gvz.Style(options...)
	gvz.Println("]")
}
func (gvz *GraphVizFile) Node(id string, options ...GraphVizOptionFunc) {
	gvz.Print("%q [", id)
	gvz.Style(options...)
	gvz.Println("]")
}
func (gvz *GraphVizFile) Edge(a, b string, options ...GraphVizOptionFunc) {
	edge := "--"
	if gvz.Directed {
		edge = "->"
	}
	gvz.Print("%q %s %q [", a, edge, b)
	gvz.Style(options...)
	gvz.Println("]")
}

func (gvz *GraphVizFile) Comment(format string, args ...interface{}) {
	if gvz.Minify() {
		gvz.Println("// "+format, args...)
	}
}
func (gvz *GraphVizFile) Scope(inner func()) {
	gvz.Println("{")
	gvz.BeginIndent()
	inner()
	gvz.EndIndent()
	gvz.Println("}")
}
func (gvz *GraphVizFile) Style(options ...GraphVizOptionFunc) {
	gvo := NewGraphVizOptions(options...)

	if len(gvo.BgColor) > 0 {
		gvz.Print(" bgcolor=%q", gvo.BgColor)
	}
	if len(gvo.Color) > 0 {
		gvz.Print(" color=%q", gvo.Color)
	}
	if len(gvo.FillColor) > 0 {
		gvz.Print(" fillcolor=%q", gvo.FillColor)
	}
	if len(gvo.FontColor) > 0 {
		gvz.Print(" fontcolor=%q", gvo.FontColor)
	}
	if len(gvo.FontName) > 0 {
		gvz.Print(" fontname=%q", gvo.FontName)
	}
	if !gvo.FontSize.IsInheritable() {
		gvz.Print(" fontsize=%d", gvo.FontSize)
	}
	if len(gvo.Label) > 0 {
		gvz.Print(" label=%q", gvo.Label)
	}
	if !gvo.Scale.IsInheritable() {
		gvz.Print(" scale=%d", gvo.Scale)
	}
	if len(gvo.Shape) > 0 {
		gvz.Print(" shape=%q", gvo.Shape)
	}
	if len(gvo.Style) > 0 {
		gvz.Print(" style=%q", gvo.Style)
	}
	if !gvo.Constraint.IsInheritable() {
		gvz.Print(" constraint=%v", gvo.Constraint)
	}
	if !gvo.Weight.IsInheritable() {
		gvz.Print(" weight=%v", gvo.Weight)
	}
}
