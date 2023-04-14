package utils

import (
	"fmt"
	"io"
	"strings"
)

const STRUCTUREDFILE_DEFAULT_TAB = "  "

type StructuredFileFlags int32

const (
	STRUCTUREDFILE_NONE   StructuredFileFlags = 0
	STRUCTUREDFILE_MINIFY StructuredFileFlags = 1 << 0
)

type FileSite struct {
	Line   int
	Column int
}

func (si *FileSite) LineBreak() {
	si.Line += 1
	si.Column = 1
}

type StructuredFile struct {
	indent string
	tab    string
	flags  StructuredFileFlags
	site   FileSite
	writer io.Writer
}

func NewStructuredFile(writer io.Writer, tab string, minify bool) *StructuredFile {
	flags := STRUCTUREDFILE_NONE
	if minify {
		flags |= STRUCTUREDFILE_MINIFY
	}
	return &StructuredFile{
		indent: "",
		tab:    tab,
		flags:  flags,
		site: FileSite{
			Line:   1,
			Column: 1,
		},
		writer: writer,
	}
}

func (sf *StructuredFile) Minify() bool {
	return (sf.flags & STRUCTUREDFILE_MINIFY) == STRUCTUREDFILE_MINIFY
}
func (sf *StructuredFile) Site() FileSite { return sf.site }

func (sf *StructuredFile) IndentIFN() {
	if sf.site.Column == 1 {
		sf.site.Column += len(sf.indent)
		fmt.Fprint(sf.writer, sf.indent)
	}
}
func (sf *StructuredFile) BeginIndent() {
	sf.indent += sf.tab
}
func (sf *StructuredFile) EndIndent() {
	sf.indent = sf.indent[:len(sf.indent)-len(sf.tab)]
}
func (sf *StructuredFile) ScopeIndent(infix func()) {
	if infix != nil {
		sf.LineBreak()
		sf.BeginIndent()
		infix()
		sf.LineBreak()
		sf.EndIndent()
	}
}

func (sf *StructuredFile) Print(format string, args ...interface{}) {
	sf.IndentIFN()
	sf.Print_NoIndent(format, args...)
}
func (sf *StructuredFile) Println(format string, args ...interface{}) {
	sf.IndentIFN()
	sf.Println_NoIndent(format, args...)
}
func (sf *StructuredFile) LineBreak() {
	if sf.site.Column > 1 {
		sf.site.LineBreak()
		fmt.Fprintln(sf.writer)
	}
}
func (sf *StructuredFile) Align(column int) {
	sf.Pad(column, " ")
}
func (sf *StructuredFile) Pad(column int, in string) {
	if sf.site.Column < column {
		if len(in) == 1 {
			fmt.Fprint(sf.writer, strings.Repeat(in[:1], column-sf.site.Column))
		} else {
			runes := Map(func(r rune) string { return string(r) }, []rune(in)...)
			for i := sf.site.Column; i < column; i += 1 {
				fmt.Fprint(sf.writer, runes[i%len(runes)])
			}
		}

		sf.site.Column = column
	}
}

func (sf *StructuredFile) Print_NoIndent(format string, args ...interface{}) {
	txt := format
	if len(args) > 0 {
		txt = fmt.Sprintf(format, args...)
	}
	sf.site.Column += ansi_escaped_len(txt)
	fmt.Fprint(sf.writer, txt)
}
func (sf *StructuredFile) Println_NoIndent(format string, args ...interface{}) {
	txt := format
	if len(args) > 0 {
		txt = fmt.Sprintf(format, args...)
	}
	sf.site.LineBreak()
	fmt.Fprintln(sf.writer, txt)
}
