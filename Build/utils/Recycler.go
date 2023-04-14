package utils

import (
	"bytes"
	"sync"
)

type Recycler[T any] interface {
	Allocate() T
	Release(T)
}

type recyclerPool[T any] struct {
	pool       sync.Pool
	onAllocate func(T)
	onRelease  func(T)
}

func NewRecycler[T any](factory func() T, allocate func(T), release func(T)) Recycler[T] {
	result := &recyclerPool[T]{}
	result.pool.New = func() any { return factory() }
	result.onAllocate = release
	result.onRelease = release
	return result
}
func (x *recyclerPool[T]) Allocate() (result T) {
	result = x.pool.Get().(T)
	x.onAllocate(result)
	return
}
func (x *recyclerPool[T]) Release(item T) {
	x.onRelease(item)
	x.pool.Put(item)
}

const TRANSIENT_BYTES_CAPACITY = 64 << 10

// recycle temporary buffers
var TransientBytes = NewRecycler(
	func() []byte { return make([]byte, TRANSIENT_BYTES_CAPACITY) },
	func([]byte) {}, func([]byte) {})

// recycle byte buffers
var TransientBuffer = NewRecycler(
	func() *bytes.Buffer { return &bytes.Buffer{} },
	func(*bytes.Buffer) {},
	func(b *bytes.Buffer) {
		b.Reset()
	})

// recycle channels for Future[T]
var AnyChannels = NewRecycler(
	func() chan any { return make(chan any, 1) },
	func(chan any) {}, func(chan any) {})
