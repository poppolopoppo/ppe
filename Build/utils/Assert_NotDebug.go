//go:build !debug
// +build !debug

package utils

import (
	"fmt"
	"log"
	"reflect"
)

var enableDiagnostics bool = false

func EnableDiagnostics() bool {
	return enableDiagnostics
}
func SetEnableDiagnostics(enabled bool) {
	enableDiagnostics = enabled
}

var enableAssertions bool = false

func Assert(pred func() bool)                    {}
func AssertSameType[T any](T, T)                 {}
func AssertIn[T comparable](T, ...T)             {}
func AssertNotIn[T comparable](T, ...T)          {}
func AssertInStrings[T fmt.Stringer](T, ...T)    {}
func AssertNotInStrings[T fmt.Stringer](T, ...T) {}

func NotImplemented(string, ...interface{})    { log.Panic("not implemented") }
func UnreachableCode()                         { log.Panic("unreachable code") }
func UnexpectedValue(interface{})              { log.Panic("unexpected value") }
func UnexpectedType(reflect.Type, interface{}) { log.Panic("unexpected type") }

func AppendComparable_CheckUniq[T comparable](src []T, elts ...T) []T {
	return append(src, elts...)
}
func PrependComparable_CheckUniq[T comparable](src []T, elts ...T) []T {
	return append(elts, src...)
}

func AppendEquatable_CheckUniq[T Equatable[T]](src []T, elts ...T) (result []T) {
	return append(src, elts...)
}
func PrependEquatable_CheckUniq[T Equatable[T]](src []T, elts ...T) (result []T) {
	return append(elts, src...)
}

func MakeFuture[T any](f func() (T, error)) Future[T] {
	return make_async_future(f)
}

func make_logQueue() logQueue {
	return make_logQueue_deferred()
}

func log_callstack() {}