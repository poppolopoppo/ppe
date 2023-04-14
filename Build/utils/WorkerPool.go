package utils

import (
	"runtime"
	"sync"
)

type TaskFunc func()

type WorkerPool interface {
	Arity() int
	Queue(TaskFunc)
	Join()
}

var GetGlobalWorkerPool = Memoize(func() WorkerPool {
	return NewFixedSizeWorkerPool("global", runtime.NumCPU()-1)
})

var GetBackgroundWorkerPool = Memoize(func() WorkerPool {
	return NewFixedSizeWorkerPool("background", 1)
})

type fixedSizeWorkerPool struct {
	give       chan TaskFunc
	name       string
	numWorkers int

	mutex sync.Mutex
	cond  *sync.Cond
}

func NewFixedSizeWorkerPool(name string, numWorkers int) WorkerPool {
	pool := &fixedSizeWorkerPool{
		give:       make(chan TaskFunc, 8192),
		name:       name,
		numWorkers: numWorkers,
		mutex:      sync.Mutex{},
	}
	pool.cond = sync.NewCond(&pool.mutex)
	for i := 0; i < pool.numWorkers; i += 1 {
		workerIndex := i
		go pool.workerLoop(workerIndex)
	}
	return pool
}
func (x *fixedSizeWorkerPool) Arity() int {
	return x.numWorkers
}
func (x *fixedSizeWorkerPool) Queue(task TaskFunc) {
	Assert(func() bool { return task != nil })
	x.give <- task
}
func (x *fixedSizeWorkerPool) Join() {
	wg := sync.WaitGroup{}
	wg.Add(x.numWorkers)
	for i := 0; i < x.numWorkers; i += 1 {
		x.Queue(func() {
			wg.Done()

			x.mutex.Lock()
			x.cond.Wait()
			x.mutex.Unlock()
		})
	}
	wg.Wait()
	x.cond.Broadcast()
}
func (x *fixedSizeWorkerPool) workerLoop(workerIndex int) {
	for {
		(<-x.give)()
	}
}
