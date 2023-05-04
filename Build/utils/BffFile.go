package utils

import (
	"fmt"
	"io"
	"strings"
)

type BffOp string

func (x BffOp) String() string { return (string)(x) }

const (
	BFF_ASSIGN BffOp = "="
	BFF_CONCAT BffOp = "+"
)

type BffScope string

func (x BffScope) String() string { return (string)(x) }

const (
	BFF_LOCAL  BffScope = "."
	BFF_GLOBAL BffScope = "^"
)

type BffVar string
type BffArray []interface{}
type BffMap map[string]interface{}

func MakeBffVar(key string) BffVar {
	return BffVar(SanitizeIdentifier(key))
}

func (x BffVar) Valid() bool    { return x != "" }
func (x BffVar) String() string { return (string)(x) }

func MakeBffArray[T any](it ...T) (result BffArray) {
	result = make(BffArray, len(it))
	for i, x := range it {
		result[i] = x
	}
	return
}

func bffIsDefaultValue(x interface{}) bool {
	if x == nil {
		return true
	}
	switch value := x.(type) {
	case BoolVar, IntVar, BffVar:
		return false
	case string:
		return value == ""
	case bool:
		return false
	case int8, int16, int32, int64:
		return false
	case uint8, uint16, uint32, uint64:
		return false
	case float32, float64:
		return false
	case []string:
		return len(value) == 0
	case StringSetable:
		return len(value.StringSet()) == 0
	case BffArray:
		return len(value) == 0
	case BffMap:
		return len(value) == 0
	case fmt.Stringer:
		return bffIsDefaultValue(value.String())
	default:
		UnexpectedValue(x)
	}
	return false
}

type BffFile struct {
	aliases StringSet
	*StructuredFile
}

func NewBffFile(dst io.Writer, minify bool) *BffFile {
	return &BffFile{
		StructuredFile: NewStructuredFile(dst, STRUCTUREDFILE_DEFAULT_TAB, minify),
	}
}

func (bff *BffFile) Once(key BffVar, closure func()) *BffFile {
	if !bff.aliases.Contains(key.String()) {
		bff.aliases.Append(key.String())
		closure()
	}
	return bff
}
func (bff *BffFile) Include(path Filename) *BffFile {
	bff.Println(`#include "%s"`, path)
	return bff
}
func (bff *BffFile) Comment(text string, a ...interface{}) *BffFile {
	if !bff.Minify() {
		bff.Println("// "+text, a...)
	}
	return bff
}
func (bff *BffFile) Import(varname ...string) *BffFile {
	for _, x := range varname {
		bff.Println("#import " + x)
	}
	return bff
}
func (bff *BffFile) SetVar(name string, value interface{}, operator BffOp, scope BffScope) *BffFile {
	if !bffIsDefaultValue(value) {
		if bff.Minify() {
			bff.Print(scope.String() + name + operator.String())
		} else {
			bff.Print(scope.String() + name + " " + operator.String() + " ")
		}
		bff.Value(value)
		bff.LineBreak()
	}
	return bff
}
func (bff *BffFile) Assign(name string, value interface{}) *BffFile {
	return bff.SetVar(name, value, BFF_ASSIGN, BFF_LOCAL)
}
func (bff *BffFile) Append(name string, value interface{}) *BffFile {
	return bff.SetVar(name, value, BFF_CONCAT, BFF_LOCAL)
}
func (bff *BffFile) Value(x interface{}) *BffFile {
	switch value := x.(type) {
	case BoolVar:
		bff.Value(value.Get())
	case IntVar:
		bff.Value(value.Get())
	case BffVar:
		bff.Print("." + value.String())
	case string:
		bff.Print(`"%s"`, strings.ReplaceAll(value, "\"", "^\""))
	case bool:
		if value {
			bff.Print("true")
		} else {
			bff.Print("false")
		}
	case int8, int16, int32, int64:
		bff.Print("%d", value)
	case uint8, uint16, uint32, uint64:
		bff.Print("%u", value)
	case float32, float64:
		bff.Print("%f", value)
	case []string:
		bff.Value(MakeBffArray(value...))
	case StringSetable:
		bff.Value(MakeBffArray(value.StringSet()...))
	case BffArray:
		bff.Print("{")
		bff.ScopeIndent(func() {
			for i, x := range value {
				if i > 0 {
					if bff.Minify() {
						bff.Print(",")
					} else {
						bff.Println(",")
					}
				}
				bff.Value(x)
			}
		})
		bff.Println("}")
	case BffMap:
		bff.Print("[")
		bff.ScopeIndent(func() {
			for k, v := range value {
				bff.Assign(k, v)
			}
		})
		bff.Println("]")
	case fmt.Stringer:
		bff.Value(x.(fmt.Stringer).String())
	default:
		UnexpectedValue(x)
	}
	return bff
}
func (bff *BffFile) Func(name string, closure func(), args ...string) *BffFile {
	bff.Print(name)
	if len(args) > 0 {
		bff.Print("(")
		if !bff.Minify() {
			bff.Print(" ")
		}
		for i, x := range args {
			if i > 0 {
				bff.Print(" ")
			}
			bff.Value(x)
		}
		if !bff.Minify() {
			bff.Print(" ")
		}
		bff.Print(")")
	}
	if bff.Minify() {
		bff.Print("{")
	} else {
		bff.Print(" {")
	}
	bff.ScopeIndent(closure)
	bff.Println("}")
	return bff
}
func (bff *BffFile) Using(name BffVar) *BffFile {
	if bff.Minify() {
		bff.Println("Using(.%s)", name)
	} else {
		bff.Println("Using( .%s )", name)
	}
	return bff
}
func (bff *BffFile) Struct(name BffVar, closure func()) *BffFile {
	if bff.Minify() {
		bff.Print(".%s=[", name)
	} else {
		bff.Print(".%s = [", name)
	}
	bff.ScopeIndent(closure)
	bff.Println("]")
	return bff
}
