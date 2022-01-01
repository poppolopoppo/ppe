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

func Range[T any](transform func(int) T, n int) (dst []T) {
	dst = make([]T, n)
	for i := 0; i < n; i += 1 {
		dst[i] = transform(i)
	}
	return dst
}
