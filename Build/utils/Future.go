package utils

import (
	"fmt"
	"strings"
	"sync"
	"sync/atomic"
	"time"
	"unsafe"
)

type Future[T any] interface {
	Join() Result[T]
}

/***************************************
 * Result[T]
 ***************************************/

type Result[S any] interface {
	Success() S
	Failure() error
	Get() (S, error)
}

type result[S any] struct {
	success S
	failure error
}

func (r result[S]) Get() (S, error) {
	return r.success, r.failure
}
func (r result[S]) Success() S {
	if r.failure != nil {
		LogPanic("future: %v", r.failure)
	}
	return r.success
}
func (r result[S]) Failure() error {
	return r.failure
}
func (r result[S]) String() string {
	if r.failure != nil {
		return fmt.Sprint(r.failure)
	} else {
		return fmt.Sprint(r.success)
	}
}

/***************************************
 * Sync Future (for debug)
 ***************************************/

type sync_future[T any] struct {
	await  func() (T, error)
	result *result[T]
	debug  []fmt.Stringer
}

func make_sync_future[T any](f func() (T, error), debug ...fmt.Stringer) Future[T] {
	AssertMessage(func() bool { return f != nil }, "invalid future!\n%s", strings.Join(Stringize(debug...), "\n"))
	return &sync_future[T]{
		await:  f,
		result: nil,
		debug:  debug,
	}
}
func (future *sync_future[T]) Join() Result[T] {
	if future.result == nil {
		await := future.await
		AssertMessage(func() bool { return await != nil }, "future reentrancy!\n%s", strings.Join(Stringize(future.debug...), "\n"))
		future.await = nil
		value, err := await()
		future.result = &result[T]{
			success: value,
			failure: err,
		}
	}
	return future.result
}

/***************************************
 * Async Future (using goroutine)
 ***************************************/

type async_future[T any] struct {
	wait   chan any
	once   sync.Once
	result result[T]
}

func MakeGlobalWorkerFuture[T any](f func() (T, error)) Future[T] {
	future := &async_future[T]{wait: AnyChannels.Allocate()}
	GetGlobalWorkerPool().Queue(func() {
		future.invoke(f)
	})
	return future
}

func make_async_future[T any](f func() (T, error)) Future[T] {
	return (&async_future[T]{wait: AnyChannels.Allocate()}).run_in_background(f)
}

func (future *async_future[T]) run_in_background(f func() (T, error)) *async_future[T] {
	go future.invoke(f)
	return future
}
func (future *async_future[T]) invoke(f func() (T, error)) {
	success, failure := f()
	future.wait <- result[T]{success, failure}
}
func (future *async_future[T]) Join() Result[T] {
	future.once.Do(func() {
		future.result = (<-future.wait).(result[T])
		AnyChannels.Release(future.wait)
		future.wait = nil
	})
	return future.result
}

/***************************************
 * Timeout Async Future (using goroutine)
 ***************************************/

type timeout_async_future[T any] struct {
	inner async_future[T]
}

func make_timeout_async_future[T any](f func() (T, error)) Future[T] {
	result := &timeout_async_future[T]{}
	result.inner.run_in_background(f)
	return result
}
func (future *timeout_async_future[T]) Join() Result[T] {
	future.inner.once.Do(func() {
		defer AnyChannels.Release(future.inner.wait)
		select { // with timeout
		case r := <-future.inner.wait:
			future.inner.result = r.(result[T])
		case <-time.After(30 * time.Second):
			panic(fmt.Errorf("future timeout"))
		}
		future.inner.wait = nil
	})
	return future.inner.result
}

/***************************************
 * Map Future
 ***************************************/

type map_future_result[OUT, IN any] struct {
	inner     Result[IN]
	transform func(IN) OUT
}

func (x map_future_result[OUT, IN]) Success() OUT {
	return x.transform(x.inner.Success())
}
func (x map_future_result[OUT, IN]) Failure() error {
	return x.inner.Failure()
}
func (x map_future_result[OUT, IN]) Get() (OUT, error) {
	if result, err := x.inner.Get(); err == nil {
		return x.transform(result), nil
	} else {
		var none OUT
		return none, err
	}
}

type map_future[OUT, IN any] struct {
	inner     Future[IN]
	transform func(IN) OUT
}

func MapFuture[OUT, IN any](future Future[IN], transform func(IN) OUT) Future[OUT] {
	return map_future[OUT, IN]{inner: future, transform: transform}
}
func (x map_future[OUT, IN]) Join() Result[OUT] {
	return map_future_result[OUT, IN]{
		inner:     x.inner.Join(),
		transform: x.transform,
	}
}

/***************************************
 * Future Literal
 ***************************************/

type future_literal[T any] struct {
	immediate result[T]
}

func (x future_literal[T]) Join() Result[T] {
	return &x.immediate
}

func MakeFutureLiteral[T any](value T) Future[T] {
	return future_literal[T]{
		immediate: result[T]{
			success: value,
			failure: nil,
		},
	}
}

/***************************************
 * Future Error
 ***************************************/

func MakeFutureError[T any](err error) Future[T] {
	return future_literal[T]{
		immediate: result[T]{
			failure: err,
		},
	}
}

/***************************************
 * Parallel Helpers
 ***************************************/

func ParallelJoin_Sync[T any](each func(int, T) error, futures ...Future[T]) error {
	for i, future := range futures {
		if value, err := future.Join().Get(); err == nil {
			each(i, value)
		} else {
			return err
		}
	}
	return nil
}

const enableParallelJoin_Async = false // TODO: ParallelJoin_Async() is spawning too much goroutines, find a way to select mutilple futures?

func ParallelJoin_Async[T any](each func(int, T) error, futures ...Future[T]) error {
	if len(futures) == 1 || !enableParallelJoin_Async {
		return ParallelJoin_Sync(each, futures...)
	}

	recv := AnyChannels.Allocate()
	defer AnyChannels.Release(recv)

	for i, future := range futures {
		go func(id int, future Future[T]) {
			future.Join()
			recv <- id
		}(i, future)
	}

	var lastErr error
	for i := 0; i < len(futures); i += 1 {
		id := (<-recv).(int)
		if value, err := futures[id].Join().Get(); err == nil {
			each(id, value)
		} else {
			lastErr = err
		}
	}

	return lastErr
}

func ParallelRange_Sync[IN any](each func(IN) error, in ...IN) error {
	for _, it := range in {
		if err := each(it); err != nil {
			return err
		}
	}
	return nil
}

func ParallelRange_Async[IN any](each func(IN) error, in ...IN) error {
	var firstErr unsafe.Pointer

	wg := sync.WaitGroup{}
	wg.Add(len(in))

	for _, it := range in {
		go func(input IN) {
			defer wg.Done()
			if err := each(input); err != nil {
				atomic.CompareAndSwapPointer(&firstErr, nil, unsafe.Pointer(&err))
			}
		}(it)
	}

	wg.Wait()
	if firstErr == nil {
		return nil
	} else {
		return *(*error)(firstErr)
	}
}

func ParallelMap_Sync[IN any, OUT any](each func(IN) (OUT, error), in ...IN) ([]OUT, error) {
	results := Map(func(x IN) OUT {
		result, err := each(x)
		if err != nil {
			LogPanicErr(err)
		}
		return result
	}, in...)
	return results, nil
}

func ParallelMap_Async[IN any, OUT any](each func(IN) (OUT, error), in ...IN) ([]OUT, error) {
	var firstErr unsafe.Pointer

	wg := sync.WaitGroup{}
	wg.Add(len(in))

	results := make([]OUT, len(in))
	for i, it := range in {
		go func(id int, input IN) {
			if value, err := each(input); err == nil {
				results[id] = value
			} else {
				atomic.CompareAndSwapPointer(&firstErr, nil, unsafe.Pointer(&err))
			}
			wg.Done()
		}(i, it)
	}

	wg.Wait()
	if firstErr == nil {
		return results, nil
	} else {
		return results, *(*error)(firstErr)
	}
}
