package utils

import (
	"context"
	"fmt"
)

type Result[S any] interface {
	Success() S
	Failure() error
}

type result[S any] struct {
	success S
	failure error
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
	Cancel()
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
func (future *sync_future[T]) Cancel() {}
func (future *sync_future[T]) Join() Result[T] {
	return future.memoized()
}

type async_future[T any] struct {
	result    *result[T]
	completed bool
	wait      chan bool
	ctx       context.Context
	cancel    func()
}

func make_async_future[T any](f func() (T, error)) Future[T] {
	future := &async_future[T]{
		wait: make(chan bool),
	}
	future.ctx, future.cancel = context.WithCancel(context.Background())
	go func() {
		defer close(future.wait)
		success, failure := f()
		future.result = &result[T]{success, failure}
		future.completed = true
		future.wait <- true
	}()
	return future
}
func (future *async_future[T]) Cancel() {
	future.cancel()
}
func (future *async_future[T]) Join() Result[T] {
	if future.completed {
		return future.result
	} else {
		select {
		case <-future.wait:
			return future.result
		case <-future.ctx.Done():
			return &result[T]{
				failure: future.ctx.Err(),
			}
		}
	}
}

type futureLiteral[T any] struct {
	immediate result[T]
}

func (x futureLiteral[T]) Cancel() {}
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
