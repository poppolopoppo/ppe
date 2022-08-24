package utils

import (
	"container/list"
	"time"
)

type Recycler[T any] interface {
	Allocate() T
	Release(T)
}

type recyclerPoolT[T any] struct {
	get, give chan T
}

type recyclerQueueT[T any] struct {
	when  time.Time
	slice T
}

func NewRecycler[T any](allocator func() T, lifetime time.Duration) Recycler[T] {
	get := make(chan T)
	give := make(chan T)
	go func() {
		q := new(list.List)
		for {
			if q.Len() == 0 {
				q.PushFront(recyclerQueueT[T]{when: time.Now(), slice: allocator()})
			}

			e := q.Front()

			timeout := time.NewTimer(lifetime)
			select {
			case b := <-give:
				timeout.Stop()
				q.PushFront(recyclerQueueT[T]{when: time.Now(), slice: b})

			case get <- e.Value.(recyclerQueueT[T]).slice:
				timeout.Stop()
				q.Remove(e)

			case <-timeout.C:
				e := q.Front()
				for e != nil {
					n := e.Next()
					if time.Since(e.Value.(recyclerQueueT[T]).when) > lifetime {
						q.Remove(e)
						e.Value = nil
					}
					e = n
				}
			}
		}

	}()
	return &recyclerPoolT[T]{get, give}
}
func (re *recyclerPoolT[T]) Allocate() T {
	return <-re.get
}
func (re *recyclerPoolT[T]) Release(block T) {
	re.give <- block
}

const TRANSIENT_BYTES_CAPACITY = 64 << 10

var TransientBytes = NewRecycler(
	func() []byte { return make([]byte, TRANSIENT_BYTES_CAPACITY) },
	time.Minute)
