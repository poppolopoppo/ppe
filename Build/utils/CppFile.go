package utils

import (
	"io"
	"strings"
)

type CppFile struct {
	ifdef string
	*StructuredFile
}

func NewCppFile(dst io.Writer, minify bool) *CppFile {
	return &CppFile{
		StructuredFile: NewStructuredFile(dst, STRUCTUREDFILE_DEFAULT_TAB, minify),
	}
}

func (cpp *CppFile) Comment(format string, args ...interface{}) {
	if !cpp.Minify() {
		cpp.Println("// "+format, args...)
	}
}

func (cpp *CppFile) Pragma(format string, args ...interface{}) {
	cpp.Println("#pragma "+format, args...)
}
func (cpp *CppFile) Define(name, value string) {
	cpp.Println("#define %s %s", name, value)
}
func (cpp *CppFile) Include(path string) {
	cpp.Println(`#include "%s"`, path)
}
func (cpp *CppFile) IfMacro(test string, inner func()) {
	cpp.Print_NoIndent("#if " + test)
	cpp.LineBreak()
	inner()
	cpp.Println_NoIndent("#endif")
}
func (cpp *CppFile) LazyIfMacro(test string, inner func(), flush bool) {
	if len(test) > 0 {
		if cpp.ifdef != test {
			if len(cpp.ifdef) > 0 {
				cpp.Println("#endif")
			}
			cpp.Println("#if " + test)
			cpp.ifdef = test
		}
	} else if len(cpp.ifdef) > 0 {
		cpp.Println("#endif")
		cpp.ifdef = ""
	}
	inner()
	if flush && len(cpp.ifdef) > 0 {
		cpp.Println("#endif")
		cpp.ifdef = ""
	}
}
func (cpp *CppFile) LazyIfDef(symbol string, inner func(), flush bool) {
	if len(symbol) > 0 {
		cpp.LazyIfMacro("defined("+symbol+")", inner, flush)
	} else {
		if flush && len(cpp.ifdef) > 0 {
			cpp.Println("#endif")
			cpp.ifdef = ""
		}
		inner()
	}
}
func (cpp *CppFile) IfDef(symbol string, inner func()) {
	if len(symbol) > 0 {
		cpp.IfMacro("defined("+symbol+")", inner)
	} else {
		inner()
	}
}
func (cpp *CppFile) IfnDef(symbol string, inner func()) {
	if len(symbol) > 0 {
		cpp.IfMacro("!defined("+symbol+")", inner)
	} else {
		inner()
	}
}

func (cpp *CppFile) Declare(name, result string, value func()) {
	cpp.Print("%s %s{", result, name)
	cpp.ScopeIndent(value)
	cpp.Println("};")
}
func (cpp *CppFile) Statement(format string, args ...interface{}) {
	cpp.Println(format+";", args...)
}

func (cpp *CppFile) Closure(inner func(), suff ...string) {
	if inner != nil {
		if cpp.Minify() {
			cpp.Print("{")
		} else {
			cpp.Print(" {")
		}
		cpp.ScopeIndent(inner)
		if !cpp.Minify() {
			cpp.LineBreak()
		}
		cpp.Println(strings.Join(append([]string{"}"}, suff...), ""))
	} else {
		cpp.Println(";")
	}
}

func (cpp *CppFile) If(condition string, inner func()) {
	cpp.Print("if (" + condition + ")")
	cpp.Closure(inner)
}
func (cpp *CppFile) ElseIf(condition string, inner func()) {
	cpp.Print("else if (" + condition + ")")
	cpp.Closure(inner)
}
func (cpp *CppFile) Else(inner func()) {
	cpp.Print("else ")
	cpp.Closure(inner)
}

func (cpp *CppFile) Namespace(name string, inner func()) {
	cpp.Println("namespace " + name + " {")
	inner()
	cpp.Println("} //!namespace " + name)
}
func (cpp *CppFile) EnumC99(name string, underlying string, inner func()) {
	cpp.Print("enum %s : %s", name, underlying)
	cpp.Closure(inner, ";")
}
func (cpp *CppFile) EnumClass(name string, underlying string, inner func()) {
	cpp.EnumC99("class "+name, underlying, inner)
}
func (cpp *CppFile) Class(name string, inner func()) {
	cpp.Print("class " + name)
	cpp.Closure(inner, ";")
}
func (cpp *CppFile) Struct(name string, inner func()) {
	cpp.Print("struct " + name)
	cpp.Closure(inner, ";")
}

func (cpp *CppFile) Func(name, result string, args []string, suffix string, inner func()) {
	if len(suffix) > 0 && suffix[0] != ' ' {
		suffix = " " + suffix
	}
	cpp.Print("%s %s(%s)%s", result, name, strings.Join(args, ", "), suffix)
	cpp.Closure(inner)
}
func (cpp *CppFile) Switch(value string, inner func()) {
	cpp.Println("switch(" + value + ") {")
	inner()
	cpp.Println("}")
}
