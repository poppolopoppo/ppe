package utils

import (
	"runtime"
	"sync"
	"time"
)

type TaskFunc func()

type WorkerPool interface {
	Name() string
	Arity() int
	Queue(TaskFunc)
	Join()
	Resize(int)
}

var allWorkerPools = []WorkerPool{}

func JoinAllWorkerPools() {
	for _, pool := range allWorkerPools {
		pool.Join()
	}
}

var GetGlobalWorkerPool = Memoize(func() (result WorkerPool) {
	result = NewFixedSizeWorkerPool("global", runtime.NumCPU()-1)
	allWorkerPools = append([]WorkerPool{result}, allWorkerPools...)
	return
})
var GetLoggerWorkerPool = Memoize(func() (result WorkerPool) {
	result = NewFixedSizeWorkerPoolEx("logger", 1, func(fswp *fixedSizeWorkerPool, i int) {
		onWorkerThreadStart(fswp, i)
		defer onWorkerThreadStop(fswp, i)

		for quit := false; !quit; {
			if pinnedLogRefresh != nil {
				select {
				case task := (<-fswp.give):
					if task != nil {
						task()
					} else {
						quit = true
					}
				// refresh pinned logs if no message output after a while
				case <-time.After(33 * time.Millisecond):
					pinnedLogRefresh()
				}
			} else {
				if task := (<-fswp.give); task != nil {
					task()
				} else {
					quit = true
				}
			}
		}
	})
	allWorkerPools = append(allWorkerPools, result)
	return
})

type fixedSizeWorkerPool struct {
	give       chan TaskFunc
	name       string
	numWorkers int

	mutex sync.Mutex
	cond  *sync.Cond
}

func NewFixedSizeWorkerPool(name string, numWorkers int) WorkerPool {
	return NewFixedSizeWorkerPoolEx(name, numWorkers, func(fswp *fixedSizeWorkerPool, i int) {
		fswp.workerLoop(i)
	})
}
func NewFixedSizeWorkerPoolEx(name string, numWorkers int, loop func(*fixedSizeWorkerPool, int)) WorkerPool {
	pool := &fixedSizeWorkerPool{
		give:       make(chan TaskFunc, 8192),
		name:       name,
		numWorkers: numWorkers,
		mutex:      sync.Mutex{},
	}
	pool.cond = sync.NewCond(&pool.mutex)
	for i := 0; i < pool.numWorkers; i += 1 {
		workerIndex := i
		go loop(pool, workerIndex)
	}
	return pool
}
func (x *fixedSizeWorkerPool) Name() string { return x.name }
func (x *fixedSizeWorkerPool) Arity() int   { return x.numWorkers }
func (x *fixedSizeWorkerPool) Queue(task TaskFunc) {
	Assert(func() bool { return task != nil })
	x.give <- task
}
func (x *fixedSizeWorkerPool) Join() {
	x.cond.L.Lock()
	defer x.cond.L.Unlock()

	pbar := LogProgress(0, x.numWorkers, "join %s worker pool", x.name)
	defer pbar.Close()

	for i := 0; i < x.numWorkers; i += 1 {
		x.Queue(func() {
			pbar.Inc()

			x.cond.L.Lock()
			defer x.cond.L.Unlock()

			x.cond.Broadcast()
		})
	}

	x.cond.Wait()
}
func (x *fixedSizeWorkerPool) Resize(n int) {
	Assert(func() bool { return n > 0 })
	x.mutex.Lock()
	defer x.mutex.Unlock()

	if n == x.numWorkers {
		return
	}

	LogTrace("workerpool: resizing %q pool from %d to %d worker threads", x.name, x.numWorkers, n)

	delta := n - x.numWorkers
	if delta > 0 {
		for i := 0; i < delta; i += 1 {
			workerIndex := x.numWorkers + i // create a new worker
			go x.workerLoop(workerIndex)
		}
	} else {
		for i := 0; i < -delta; i += 1 {
			x.give <- nil // push a nil task to kill the future
		}
	}
	x.numWorkers += delta
}
func onWorkerThreadStart(pool WorkerPool, workerIndex int) {
	// LockOSThread wires the calling goroutine to its current operating system thread.
	// The calling goroutine will always execute in that thread,
	// and no other goroutine will execute in it,
	// until the calling goroutine has made as many calls to
	// UnlockOSThread as to LockOSThread.
	// If the calling goroutine exits without unlocking the thread,
	// the thread will be terminated.
	runtime.LockOSThread()
}
func onWorkerThreadStop(pool WorkerPool, workerIndex int) {
	//runtime.UnlockOSThread() // let acquired thread die with the pool
}
func (x *fixedSizeWorkerPool) workerLoop(workerIndex int) {
	onWorkerThreadStart(x, workerIndex)
	defer onWorkerThreadStop(x, workerIndex)

	for {
		if task := (<-x.give); task != nil {
			task()
		} else {
			break
		}
	}
}
