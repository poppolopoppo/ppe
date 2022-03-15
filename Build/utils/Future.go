package utils

import (
	"fmt"
	"sync"
)

type Result[S any] interface {
	Success() S
	Failure() error
	Get() (S, error)
}

type result[S any] struct {
	success S
	failure error
}

func (r *result[S]) Get() (S, error) {
	return r.success, r.failure
}
func (r *result[S]) Success() S {
	if r.failure != nil {
		LogPanic("future: %v", r.failure)
	}
	return r.success
}
func (r *result[S]) Failure() error {
	return r.failure
}
func (r *result[S]) String() string {
	if r.failure != nil {
		return fmt.Sprint(r.failure)
	} else {
		return fmt.Sprint(r.success)
	}
}

type Future[T any] interface {
	Join() Result[T]
}

type sync_future[T any] struct {
	memoized func() Result[T]
}

func make_sync_future[T any](f func() (T, error)) Future[T] {
	return &sync_future[T]{
		Memoize(func() Result[T] {
			value, err := f()
			return &result[T]{
				success: value,
				failure: err,
			}
		}),
	}
}
func (future *sync_future[T]) Join() Result[T] {
	return future.memoized()
}

type async_future[T any] struct {
	result *result[T]
	wait   <-chan bool
}

func make_async_future[T any](f func() (T, error)) Future[T] {
	wait := make(chan bool)
	future := &async_future[T]{wait: wait}
	go func() {
		defer close(wait)
		success, failure := f()
		future.result = &result[T]{success, failure}
		wait <- true
	}()
	return future
}
func (future *async_future[T]) Join() Result[T] {
	if future.result != nil {
		return future.result
	} else {
		select {
		case <-future.wait:
			return future.result
		}
	}
}

type futureLiteral[T any] struct {
	immediate result[T]
}

func (x futureLiteral[T]) Join() Result[T] {
	return &x.immediate
}

func MakeFutureError[T any](err error) Future[T] {
	return futureLiteral[T]{
		immediate: result[T]{
			failure: err,
		},
	}
}

func ParallelMap[IN any, OUT any](each func(IN) (OUT, error), in ...IN) ([]OUT, error) {
	futures := make([]Future[OUT], len(in))
	for i := range in {
		var ref = in[i]
		futures[i] = MakeFuture(func() (OUT, error) {
			return each(ref)
		})
	}
	var err error
	result := Map(func(fut Future[OUT]) OUT {
		x := fut.Join()
		if x.Failure() != nil {
			return x.Success()
		} else {
			err = x.Failure()
		}
		return fut.Join().Success()
	}, futures...)
	return result, err
}
func ParallelRange[T any](each func(T) error, it ...T) error {
	return ParallelWhile(each, nil, it...)
}
func ParallelWhile[T any](each func(T) error, while func() error, it ...T) error {
	var firstErr error
	wg := sync.WaitGroup{}
	wg.Add(len(it))
	for i := range it {
		go func(ref T) {
			defer wg.Done()
			if err := each(ref); err != nil {
				firstErr = err
			}
		}(it[i])
	}
	if while != nil {
		if err := while(); err != nil {
			firstErr = err
		}
	}
	wg.Wait()
	return firstErr
}
