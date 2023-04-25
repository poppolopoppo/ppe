package utils

import (
	"bufio"
	"fmt"
	"io"
	"reflect"
	"regexp"
	"strings"
	"sync"
	"time"
)

var programStartedAt = time.Now()

func InitUtils() {
	setupCloseHandler()

	// register type for serialization
	RegisterSerializable(&buildNode{})
	RegisterSerializable(&CompressedArchiveExtractor{})
	RegisterSerializable(&Directory{})
	RegisterSerializable(&DirectoryCreator{})
	RegisterSerializable(&DirectoryGlob{})
	RegisterSerializable(&DirectoryList{})
	RegisterSerializable(&Downloader{})
	RegisterSerializable(&Filename{})
	RegisterSerializable(&SourceControlModifiedFiles{})
	RegisterSerializable(&SourceControlStatus{})
}

/***************************************
 * https://mangatmodi.medium.com/go-check-nil-interface-the-right-way-d142776edef1
 ***************************************/

func IsNil(v interface{}) bool {
	if v == nil {
		return true
	}
	val := reflect.ValueOf(v)
	switch val.Kind() {
	case reflect.Chan, reflect.Func, reflect.Map, reflect.Pointer, reflect.UnsafePointer, reflect.Interface, reflect.Slice:
		return val.IsNil()
	}
	return false
}

/***************************************
 * Higher order programming
 ***************************************/

type Closable interface {
	Close()
}

type Equatable[T any] interface {
	Equals(other T) bool
}
type Comparable[T any] interface {
	Compare(other T) int
}
type OrderedComparable[T any] interface {
	Comparable[T]
	comparable
}

type jointStringer[T fmt.Stringer] struct {
	it    []T
	delim string
}

func (join jointStringer[T]) String() string {
	var notFirst bool
	sb := strings.Builder{}
	for _, x := range join.it {
		if notFirst {
			sb.WriteString(join.delim)
		}
		sb.WriteString(x.String())
		notFirst = true
	}
	return sb.String()
}

func Join[T fmt.Stringer](delim string, it ...T) fmt.Stringer {
	return jointStringer[T]{delim: delim, it: it}
}
func JoinString[T fmt.Stringer](delim string, it ...T) string {
	return Join(delim, it...).String()
}

func Map[T, R any](transform func(T) R, src ...T) (dst []R) {
	dst = make([]R, len(src))
	for i, x := range src {
		dst[i] = transform(x)
	}
	return dst
}

func Range[T any](transform func(int) T, n int) (dst []T) {
	dst = make([]T, n)
	for i := 0; i < n; i += 1 {
		dst[i] = transform(i)
	}
	return dst
}

func Blend[T any](ifFalse, ifTrue T, selector bool) T {
	if selector {
		return ifTrue
	} else {
		return ifFalse
	}
}

func Recover(scope func() error) (result error) {
	defer func() {
		if err := recover(); err != nil {
			result = err.(error)
		}
	}()
	result = scope()
	return
}

/***************************************
 * String helpers
 ***************************************/

var re_nonAlphaNumeric = regexp.MustCompile(`[^\w\d]+`)

func SanitizeIdentifier(in string) string {
	return re_nonAlphaNumeric.ReplaceAllString(in, "_")
}

var re_whiteSpace = regexp.MustCompile(`\s+`)

func IsWhiteSpaceStr(s string) bool {
	return re_whiteSpace.MatchString(s)
}
func IsWhiteSpaceRune(ch ...rune) bool {
	return re_whiteSpace.MatchString(string(ch))
}

func SplitWords(in string) []string {
	return re_whiteSpace.Split(in, -1)
}

func Inspect[T any](it ...T) []string {
	result := make([]string, len(it))
	for i, x := range it {
		result[i] = fmt.Sprint(reflect.TypeOf(x), "->", reflect.ValueOf(x))
	}
	return result
}

func SplitRegex(re *regexp.Regexp, capacity int) bufio.SplitFunc {
	return func(data []byte, atEOF bool) (advance int, token []byte, err error) {
		if atEOF && len(data) == 0 {
			return 0, nil, nil
		}
		if loc := re.FindIndex(data); loc != nil {
			return loc[1] + 1, data[loc[0]:loc[1]], nil
		}
		if atEOF {
			return 0, nil, io.EOF
		}
		if len(data) >= capacity {
			return len(data) - capacity, nil, nil
		}
		return 0, nil, nil
	}
}

func MakeString(x any) string {
	switch it := x.(type) {
	case string:
		return it
	case fmt.Stringer:
		return it.String()
	case []byte:
		return string(it)
	default:
		return fmt.Sprint(x)
	}
}

func Stringize[T fmt.Stringer](it ...T) []string {
	result := make([]string, len(it))
	for i, x := range it {
		result[i] = x.String()
	}
	return result
}

/***************************************
 * FourCC
 ***************************************/

type FourCC uint32

func MakeFourCC(a, b, c, d rune) FourCC {
	return FourCC(uint32(a) | (uint32(b) << 8) | (uint32(c) << 16) | (uint32(d) << 24))
}
func (x FourCC) Valid() bool { return x != 0 }
func (x FourCC) Bytes() (result [4]byte) {
	result[0] = byte((uint32(x) >> 0) & 0xFF)
	result[1] = byte((uint32(x) >> 8) & 0xFF)
	result[2] = byte((uint32(x) >> 16) & 0xFF)
	result[3] = byte((uint32(x) >> 24) & 0xFF)
	return
}
func (x FourCC) String() string {
	raw := x.Bytes()
	return string(raw[:])
}
func (x *FourCC) Serialize(ar Archive) {
	ar.UInt32((*uint32)(x))
}

/***************************************
 * Memoize
 ***************************************/

func Memoize[T any](fn func() T) func() T {
	var memoized T
	once := sync.Once{}
	return func() T {
		once.Do(func() { memoized = fn() })
		return memoized
	}
}

func MemoizeComparable[T any, ARG comparable](fn func(ARG) T) func(ARG) T {
	memoized := make(map[ARG]T)
	mutex := sync.Mutex{}
	return func(a ARG) T {
		mutex.Lock()
		result, ok := memoized[a]
		if !ok {
			result = fn(a)
			memoized[a] = result
		}
		mutex.Unlock()
		return result
	}
}

type memoized_equatable_arg[T any, ARG Equatable[ARG]] struct {
	key   ARG
	value T
}

func MemoizeEquatable[T any, ARG Equatable[ARG]](fn func(ARG) T) func(ARG) T {
	memoized := make([]memoized_equatable_arg[T, ARG], 0, 1)
	mutex := sync.Mutex{}
	return func(a ARG) T {
		mutex.Lock()
		defer mutex.Unlock()

		for _, it := range memoized {
			if it.key.Equals(a) {
				return it.value
			}
		}

		result := fn(a)
		memoized = append(memoized, memoized_equatable_arg[T, ARG]{key: a, value: result})
		return result
	}
}
