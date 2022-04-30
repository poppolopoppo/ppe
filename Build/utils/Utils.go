package utils

import (
	"bufio"
	"encoding/gob"
	"fmt"
	"io"
	"reflect"
	"regexp"
	"sync"
	"time"
)

var programStartedAt = time.Now()

func InitUtils() {
	setupCloseHandler()

	// register type for serialization
	gob.Register(Filename{})
	gob.Register(Directory{})
	gob.Register(&Downloader{})
	gob.Register(&SourceControlStatus{})
	gob.Register(&ZipExtractor{})
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

var re_nonAlphaNumeric = regexp.MustCompile(`[^\w\d]+`)

func SanitizeIdentifier(in string) string {
	return re_nonAlphaNumeric.ReplaceAllString(in, "_")
}

type jointStringer[T fmt.Stringer] struct {
	it    []T
	delim string
}

func (join jointStringer[T]) String() (result string) {
	var notFirst bool
	for _, x := range join.it {
		if notFirst {
			result += join.delim
		}
		result += x.String()
		notFirst = true
	}
	return result
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

func Stringize[T fmt.Stringer](it ...T) []string {
	result := make([]string, len(it))
	for i, x := range it {
		result[i] = x.String()
	}
	return result
}

func Memoize[T any](fn func() T) func() T {
	var memoizedValue *T
	barrier := sync.Mutex{}
	return func() T {
		if memoizedValue == nil {
			barrier.Lock()
			defer barrier.Unlock()
			if memoizedValue == nil {
				tmp := fn()
				memoizedValue = &tmp
			}
		}
		return *memoizedValue
	}
}
func MemoizePod[A comparable, T any](fn func(A) T) func(A) T {
	var memoizedArg A
	var memoizedValue *T
	barrier := sync.Mutex{}
	return func(arg A) T {
		if memoizedValue == nil || memoizedArg != arg {
			barrier.Lock()
			defer barrier.Unlock()
			if memoizedValue == nil || memoizedArg != arg {
				tmp := fn(arg)
				memoizedArg = arg
				memoizedValue = &tmp
			}
		}
		return *memoizedValue
	}
}
func MemoizeArg[A Equatable[A], T any](fn func(A) T) func(A) T {
	var memoizedArg A
	var memoizedValue *T
	barrier := sync.Mutex{}
	return func(arg A) T {
		if memoizedValue == nil || !memoizedArg.Equals(arg) {
			barrier.Lock()
			defer barrier.Unlock()
			if memoizedValue == nil || !memoizedArg.Equals(arg) {
				tmp := fn(arg)
				memoizedArg = arg
				memoizedValue = &tmp
			}
		}
		return *memoizedValue
	}
}
func MemoizeArgs2[A Equatable[A], B Equatable[B], T any](fn func(A, B) T) func(A, B) T {
	var memoizedArgA A
	var memoizedArgB B
	var memoizedValue *T
	barrier := sync.Mutex{}
	return func(argA A, argB B) T {
		if memoizedValue == nil || !memoizedArgA.Equals(argA) || !memoizedArgB.Equals(argB) {
			barrier.Lock()
			defer barrier.Unlock()
			if memoizedValue == nil || !memoizedArgA.Equals(argA) || !memoizedArgB.Equals(argB) {
				tmp := fn(argA, argB)
				memoizedArgA = argA
				memoizedArgB = argB
				memoizedValue = &tmp
			}
		}
		return *memoizedValue
	}
}
func MemoizeArgs3[A Equatable[A], B Equatable[B], C Equatable[C], T any](fn func(A, B, C) T) func(A, B, C) T {
	var memoizedArgA A
	var memoizedArgB B
	var memoizedArgC C
	var memoizedValue *T
	barrier := sync.Mutex{}
	return func(argA A, argB B, argC C) T {
		if memoizedValue == nil || !memoizedArgA.Equals(argA) || !memoizedArgB.Equals(argB) || !memoizedArgC.Equals(argC) {
			barrier.Lock()
			defer barrier.Unlock()
			if memoizedValue == nil || !memoizedArgA.Equals(argA) || !memoizedArgB.Equals(argB) || !memoizedArgC.Equals(argC) {
				tmp := fn(argA, argB, argC)
				memoizedArgA = argA
				memoizedArgB = argB
				memoizedValue = &tmp
			}
		}
		return *memoizedValue
	}
}
