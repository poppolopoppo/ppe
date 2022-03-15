//go:build debug
// +build debug

package utils

import (
	"fmt"
	"log"
	"reflect"
	"runtime/debug"
)

var enableDiagnostics bool = true

func EnableDiagnostics() bool {
	return enableDiagnostics
}
func SetEnableDiagnostics(enabled bool) {
	enableDiagnostics = enabled
}

var enableAssertions bool = true

func Assert(pred func() bool) {
	if !pred() {
		panic("failed assertion")
	}
}

func AssertSameType[T any](a T, b T) {
	ta := reflect.TypeOf(a)
	tb := reflect.TypeOf(b)
	if ta != tb {
		panic(fmt.Errorf("expected type <%v> but got <%v>", ta, tb))
	}
}

func AssertIn[T comparable](elt T, values ...T) {
	for _, x := range values {
		if x == elt {
			return
		}
	}
	panic(fmt.Errorf("element <%v> is not in the slice", elt))
}
func AssertNotIn[T comparable](elt T, values ...T) {
	for _, x := range values {
		if x == elt {
			panic(fmt.Errorf("element <%v> is already in the slice", elt))
		}
	}
}

func AssertInStrings[T fmt.Stringer](elt T, values ...T) {
	for _, x := range values {
		if x.String() == elt.String() {
			return
		}
	}
	panic(fmt.Errorf("element <%v> is not in the slice", elt))
}
func AssertNotInStrings[T fmt.Stringer](elt T, values ...T) {
	for _, x := range values {
		if x.String() == elt.String() {
			panic(fmt.Errorf("element <%v> is already in the slice", elt))
		}
	}
}

func NotImplemented(m string, a ...interface{}) {
	LogWarning("not implemented: "+m, a...)
}
func UnreachableCode() {
	panic(fmt.Errorf("unreachable code"))
}
func UnexpectedValue(x interface{}) {
	panic(fmt.Errorf("unexpected value: <%T> %v", x, x))
}
func UnexpectedType(expected reflect.Type, given interface{}) {
	if reflect.TypeOf(given) != expected {
		panic(fmt.Errorf("expected <%v>, given %v <%T>", expected, given, given))
	}
}

func AppendComparable_CheckUniq[T comparable](src []T, elts ...T) (result []T) {
	result = src
	for _, x := range elts {
		if !Contains(src, x) {
			result = append(result, x)
		} else {
			panic(fmt.Errorf("element already in set: %v (%v)", x, elts))
		}
	}
	return result
}
func PrependComparable_CheckUniq[T comparable](src []T, elts ...T) (result []T) {
	result = src
	for _, x := range elts {
		if !Contains(src, x) {
			result = append([]T{x}, result...)
		} else {
			panic(fmt.Errorf("element already in set: %v (%v)", x, elts))
		}
	}
	return result
}

func AppendEquatable_CheckUniq[T Equatable[T]](src []T, elts ...T) (result []T) {
	result = src
	for _, x := range elts {
		for _, y := range src {
			if x.Equals(y) {
				panic(fmt.Errorf("element already in set: %v (%v)", x, elts))
			}
		}
		result = append(result, x)
	}
	return result
}
func PrependEquatable_CheckUniq[T Equatable[T]](src []T, elts ...T) (result []T) {
	result = src
	for _, x := range elts {
		for _, y := range src {
			if x.Equals(y) {
				panic(fmt.Errorf("element already in set: %v (%v)", x, elts))
			}
		}
		result = append([]T{x}, result...)
	}
	return result
}

func MakeFuture[T any](f func() (T, error)) Future[T] {
	return make_sync_future(f)
}

func make_logQueue() logQueue {
	return make_logQueue_immediate()
}

func log_callstack() {
	log.Println(string(debug.Stack()))
}
