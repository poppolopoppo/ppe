//go:build !debug
// +build !debug

package utils

import (
	"fmt"
	"reflect"
	"sync/atomic"
)

var LogAssert = NewLogCategory("Assert")

var enableDiagnostics bool = false

func EnableDiagnostics() bool {
	return enableDiagnostics
}
func SetEnableDiagnostics(enabled bool) {
	enableDiagnostics = enabled
}

func AssertMessage(pred func() bool, msg string, args ...interface{}) {}

func Assert(pred func() bool)                    {}
func AssertSameType[T any](T, T)                 {}
func AssertIn[T comparable](T, ...T)             {}
func AssertNotIn[T comparable](T, ...T)          {}
func AssertInStrings[T fmt.Stringer](T, ...T)    {}
func AssertNotInStrings[T fmt.Stringer](T, ...T) {}

func NotImplemented(string, ...interface{})    { LogPanic(LogAssert, "not implemented") }
func UnreachableCode()                         { LogPanic(LogAssert, "unreachable code") }
func UnexpectedValue(interface{})              { LogPanic(LogAssert, "unexpected value") }
func UnexpectedType(reflect.Type, interface{}) { LogPanic(LogAssert, "unexpected type") }

func AppendSlice_CheckUniq[T any](src []T, elts []T, equals func(T, T) bool) (result []T) {
	return append(src, elts...)
}
func PrependSlice_CheckUniq[T any](src []T, elts []T, equals func(T, T) bool) (result []T) {
	return append(elts, src...)
}

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

type AtomicFuture[T any] struct {
	atomic.Pointer[async_future[T]]
}

func (x *AtomicFuture[T]) Reset() {
	x.Pointer.Store(nil)
}
func (x *AtomicFuture[T]) Store(future Future[T]) {
	x.Pointer.Store(future.(*async_future[T]))
}

func MakeFuture[T any](f func() (T, error), debug ...fmt.Stringer) Future[T] {
	return make_async_future(f)
}

func ParallelJoin[T any](each func(int, T) error, futures ...Future[T]) error {
	return ParallelJoin_Async(each, futures...)
}
func ParallelMap[IN any, OUT any](each func(IN) (OUT, error), in ...IN) ([]OUT, error) {
	return ParallelMap_Async(each, in...)
}
func ParallelRange[IN any](each func(IN) error, in ...IN) error {
	return ParallelRange_Async(each, in...)
}
