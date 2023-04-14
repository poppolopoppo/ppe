package utils

import "sync"

/***************************************
 * Event
 ***************************************/

type DelegateHandle = int32

type AnyDelegate func() error

func (x AnyDelegate) Invoke() error {
	return x()
}

type EventDelegate[T any] func(T) error

func (x EventDelegate[T]) Invoke(arg T) error {
	return x(arg)
}

type Event[T any] interface {
	Invoke(T) error
}

type MutableEvent[T any] interface {
	Add(EventDelegate[T]) DelegateHandle
	Remove(DelegateHandle) bool
	Clear()
	Event[T]
}

/***************************************
 * ConcurrentEvent
 ***************************************/

type ConcurrentEvent[T any] struct {
	PublicEvent[T]
	barrier sync.RWMutex
}

func (x *ConcurrentEvent[T]) Add(e EventDelegate[T]) DelegateHandle {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	return x.PublicEvent.Add(e)
}
func (x *ConcurrentEvent[T]) Remove(h DelegateHandle) bool {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	return x.PublicEvent.Remove(h)
}
func (x *ConcurrentEvent[T]) Clear() {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	x.PublicEvent.Clear()
}
func (x *ConcurrentEvent[T]) Invoke(arg T) error {
	x.barrier.RLock()
	defer x.barrier.RUnlock()
	return x.PublicEvent.Invoke(arg)
}

/***************************************
 * AnyEvent
 ***************************************/

type AnyEvent struct {
	delegates []struct {
		Handle   int32
		Delegate AnyDelegate
	}
	nextHandle DelegateHandle
}

func (x *AnyEvent) Add(e AnyDelegate) DelegateHandle {
	x.nextHandle += 1
	x.delegates = append(x.delegates, struct {
		Handle   int32
		Delegate AnyDelegate
	}{
		Handle:   x.nextHandle,
		Delegate: e,
	})
	return x.nextHandle
}
func (x *AnyEvent) Remove(handle DelegateHandle) bool {
	for i, it := range x.delegates {
		if it.Handle == handle {
			x.delegates = append(x.delegates[:i], x.delegates[i+1:]...)
			return true
		}
	}
	return false
}
func (x *AnyEvent) Invoke() error {
	for _, it := range x.delegates {
		if err := it.Delegate.Invoke(); err != nil {
			return err
		}
	}
	return nil
}
func (x *AnyEvent) Clear() {
	*x = AnyEvent{}
}

/***************************************
 * PublicEvent
 ***************************************/

type PublicEvent[T any] struct {
	delegates []struct {
		Handle   int32
		Delegate EventDelegate[T]
	}
	nextHandle DelegateHandle
}

func (x *PublicEvent[T]) Add(e EventDelegate[T]) DelegateHandle {
	x.nextHandle += 1
	x.delegates = append(x.delegates, struct {
		Handle   int32
		Delegate EventDelegate[T]
	}{
		Handle:   x.nextHandle,
		Delegate: e,
	})
	return x.nextHandle
}
func (x *PublicEvent[T]) Remove(handle DelegateHandle) bool {
	for i, it := range x.delegates {
		if it.Handle == handle {
			x.delegates = append(x.delegates[:i], x.delegates[i+1:]...)
			return true
		}
	}
	return false
}
func (x *PublicEvent[T]) Invoke(arg T) error {
	for _, it := range x.delegates {
		if err := it.Delegate.Invoke(arg); err != nil {
			return err
		}
	}
	return nil
}
func (x *PublicEvent[T]) Clear() {
	*x = PublicEvent[T]{}
}
