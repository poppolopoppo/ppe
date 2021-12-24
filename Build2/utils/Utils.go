package utils

import (
	"encoding/gob"
	"fmt"
	"regexp"
	"sync"
)

func InitUtils() {
	setupCloseHandler()

	// register type for serialization
	gob.Register(Filename{})
	gob.Register(Directory{})
	gob.Register(&SourceControlModifiedFilesT{})
	gob.Register(&SourceControlStatus{})
}

var re_nonAlphaNumeric = regexp.MustCompile(`[^a-zA-Z0-9]+`)

func SanitizeIdentifier(in string) string {
	return re_nonAlphaNumeric.ReplaceAllString(in, "_")
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

func IndexOf[T comparable](match T, values ...T) (int, bool) {
	for i, x := range values {
		if x == match {
			return i, true
		}
	}
	return -1, false
}

func IndexIf[T any](pred func(T) bool, values ...T) (int, bool) {
	for i, x := range values {
		if pred(x) {
			return i, true
		}
	}
	return -1, false
}

func Contains[T comparable](arr []T, values ...T) bool {
	for _, x := range values {
		if _, ok := IndexOf(x, arr...); !ok {
			return false
		}
	}
	return true
}

func AppendUniq[T comparable](src []T, elts ...T) (result []T) {
	result = src
	for _, x := range elts {
		if _, ok := IndexOf(x, result...); !ok {
			result = append(result, x)
		}
	}
	return result
}

func Remove[T comparable](src []T, elts ...T) (result []T) {
	result = src
	for _, x := range elts {
		if i, ok := IndexOf(x, result...); !ok {
			result = append(result[:i], result[i+1:]...)
		}
	}
	return result
}

func RemoveIf[T any](pred func(T) bool, src ...T) (result []T) {
	result = src
	for i, x := range src {
		if pred(x) {
			result = append(result[:i], result[i+1:]...)
		}
	}
	return result
}

func Range[T any](transform func(int) T, n int) (dst []T) {
	dst = make([]T, n)
	for i := 0; i < n; i += 1 {
		dst[i] = transform(i)
	}
	return dst
}

func Map[T, R any](transform func(T) R, src ...T) (dst []R) {
	dst = make([]R, len(src))
	for i, x := range src {
		dst[i] = transform(x)
	}
	return dst
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
func MemoizeArg[A comparable, T any](fn func(A) T) func(A) T {
	var memoizedArg A
	var memoizedValue *T
	barrier := sync.Mutex{}
	return func(arg A) T {
		if memoizedValue == nil || memoizedArg != arg {
			barrier.Lock()
			defer barrier.Unlock()
			if memoizedValue == nil || arg != memoizedArg {
				tmp := fn(arg)
				memoizedArg = arg
				memoizedValue = &tmp
			}
		}
		return *memoizedValue
	}
}

func Join[T fmt.Stringer](delim string, it ...T) (result string) {
	var notFirst bool
	for _, x := range it {
		if notFirst {
			result += delim
		}
		result += x.String()
		notFirst = true
	}
	return result
}
