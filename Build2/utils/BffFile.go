package utils

import (
	"fmt"
	"io"
	"strings"
)

type BffFlags int32

const (
	BFF_NONE   BffFlags = 0
	BFF_MINIFY BffFlags = 1 << 0
)

type BffOp string

func (x BffOp) String() string { return string(x) }

const (
	BFF_ASSIGN BffOp = "="
	BFF_CONCAT BffOp = "+"
)

type BffScope string

func (x BffScope) String() string { return string(x) }

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
func (x BffVar) String() string { return string(x) }

func MakeBffArray[T any](it ...T) (result BffArray) {
	result = make([]interface{}, len(it))
	for i, x := range it {
		result[i] = x
	}
	return result
}

func bffIsDefaultValue(x interface{}) bool {
	if x == nil {
		return true
	}
	switch x.(type) {
	case BoolVar, IntVar, BffVar:
		return false
	case string:
		return x.(string) == ""
	case bool:
		return false
	case int8, int16, int32, int64:
		return false
	case uint8, uint16, uint32, uint64:
		return false
	case float32, float64:
		return false
	case []string:
		return len(x.([]string)) == 0
	case StringSetable:
		return len(x.(StringSetable).StringSet()) == 0
	case BffArray:
		return len(x.(BffArray)) == 0
	case BffMap:
		return len(x.(BffMap)) == 0
	case fmt.Stringer:
		return bffIsDefaultValue(x.(fmt.Stringer).String())
	default:
		UnexpectedValue(x)
	}
	return false
}

type BffFile struct {
	Flags   BffFlags
	aliases StringSet
	io.Writer
}

func NewBffFile(dst io.Writer) *BffFile {
	return &BffFile{
		Flags:  BFF_NONE,
		Writer: dst,
	}
}

func (bff BffFile) Minify() bool {
	return (bff.Flags & BFF_MINIFY) == BFF_MINIFY
}
func (bff *BffFile) Once(key BffVar, closure func()) *BffFile {
	if !bff.aliases.Contains(key.String()) {
		bff.aliases.Append(key.String())
		closure()
	}
	return bff
}
func (bff *BffFile) Include(path Filename) *BffFile {
	fmt.Fprintf(bff, "#include \"%s\"\n", path)
	return bff
}
func (bff *BffFile) Comment(text string, a ...interface{}) *BffFile {
	if !bff.Minify() {
		fmt.Fprintf(bff, "// "+text+"\n", a...)
	}
	return bff
}
func (bff *BffFile) Import(varname ...string) *BffFile {
	for _, x := range varname {
		fmt.Fprintln(bff, "#import "+x)
	}
	return bff
}
func (bff *BffFile) SetVar(name string, value interface{}, operator BffOp, scope BffScope) *BffFile {
	if !bffIsDefaultValue(value) {
		fmt.Fprint(bff, scope.String()+name+operator.String())
		bff.Value(value)
		fmt.Fprintln(bff)
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
	switch x.(type) {
	case BoolVar:
		bff.Value(x.(BoolVar).Get())
	case IntVar:
		bff.Value(x.(IntVar).Get())
	case BffVar:
		fmt.Fprint(bff, "."+x.(BffVar))
	case string:
		fmt.Fprintf(bff, "\"%s\"", strings.ReplaceAll(x.(string), "\"", "^\""))
	case bool:
		if x.(bool) {
			fmt.Fprint(bff, "true")
		} else {
			fmt.Fprint(bff, "false")
		}
	case int8, int16, int32, int64:
		fmt.Fprintf(bff, "%d", x)
	case uint8, uint16, uint32, uint64:
		fmt.Fprintf(bff, "%u", x)
	case float32, float64:
		fmt.Fprintf(bff, "%f", x)
	case []string:
		bff.Value(MakeBffArray(x.([]string)...))
	case StringSetable:
		bff.Value(MakeBffArray(x.(StringSetable).StringSet()...))
	case BffArray:
		fmt.Fprintln(bff, "{")
		for i, x := range x.(BffArray) {
			if i > 0 {
				fmt.Fprintln(bff, ",")
			}
			bff.Value(x)
		}
		fmt.Fprint(bff, "}")
	case BffMap:
		fmt.Fprintln(bff, "[")
		for k, v := range x.(BffMap) {
			bff.Assign(k, v)
		}
		fmt.Fprint(bff, "]")
	case fmt.Stringer:
		bff.Value(x.(fmt.Stringer).String())
	default:
		UnexpectedValue(x)
	}
	return bff
}
func (bff *BffFile) Func(name string, closure func(), args ...string) *BffFile {
	fmt.Fprint(bff, name)
	if len(args) > 0 {
		fmt.Fprint(bff, "(")
		for i, x := range args {
			if i > 0 {
				fmt.Fprint(bff, " ")
			}
			bff.Value(x)
		}
		fmt.Fprint(bff, ")")
	}
	fmt.Fprintln(bff, "{")
	closure()
	fmt.Fprintln(bff, "}")
	return bff
}
func (bff *BffFile) Using(name BffVar) *BffFile {
	fmt.Fprintf(bff, "Using(.%s)\n", name)
	return bff
}
func (bff *BffFile) Struct(name BffVar, closure func()) *BffFile {
	fmt.Fprintf(bff, ".%s=[\n", name)
	closure()
	fmt.Fprintln(bff, "]")
	return bff
}
