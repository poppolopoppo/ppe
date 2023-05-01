package utils

import (
	"bytes"
	"io"
	"sync"
)

type Recycler[T any] interface {
	Allocate() T
	Release(T)
}

type recyclerPool[T any] struct {
	pool      sync.Pool
	onRelease func(T)
}

func NewRecycler[T any](factory func() T, release func(T)) Recycler[T] {
	result := &recyclerPool[T]{}
	result.pool.New = func() any { return factory() }
	result.onRelease = release
	return result
}
func (x *recyclerPool[T]) Allocate() (result T) {
	result = x.pool.Get().(T)
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
	func([]byte) {})

// recycle byte buffers
var TransientBuffer = NewRecycler(
	func() *bytes.Buffer { return &bytes.Buffer{} },
	func(b *bytes.Buffer) {
		b.Reset()
	})

// recycle channels for Future[T]
var AnyChannels = NewRecycler(
	func() chan any { return make(chan any, 1) },
	func(chan any) {})

const useTransientIoCopyOverIoCopy = true

// io copy with transient bytes to replace io.Copy()
func TransientIoCopy(dst io.Writer, src io.Reader) (err error) {
	if useTransientIoCopyOverIoCopy {
		// From io.Copy(), but with TransientBytes recycler:
		/*if wt, ok := src.(io.WriterTo); ok {
			// If the reader has a WriteTo method, use it to do the copy.
			// Avoids an allocation and a copy.
			_, err = wt.WriteTo(dst)
		} else if rt, ok := dst.(io.ReaderFrom); ok {
			// Similarly, if the writer has a ReadFrom method, use it to do the copy.
			_, err = rt.ReadFrom(src)
		} else*/{
			// io.Copy() will make a temporary allocation, and we have a recycler for this
			buf := TransientBytes.Allocate()
			defer TransientBytes.Release(buf)

			for {
				nr, er := src.Read(buf)
				if nr > 0 {
					nw, ew := dst.Write(buf[0:nr])
					if nw < 0 || nr < nw {
						nw = 0
						if ew == nil {
							ew = io.ErrShortWrite
						}
					}
					if ew != nil {
						err = ew
						break
					}
					if nr != nw {
						err = io.ErrShortWrite
						break
					}
				}
				if er != nil {
					if er != io.EOF {
						err = er
					}
					break
				}
			}
		}
	} else {
		_, err = io.Copy(dst, src)
	}

	if err == io.EOF {
		err = nil
	}
	return
}

type writerWithProgress struct {
	wr   io.Writer
	pbar PinnedProgress
}

func (x writerWithProgress) Write(p []byte) (int, error) {
	n, err := x.wr.Write(p)
	x.pbar.Add(n)
	return n, err
}

func TransientIoCopyWithProgress(context string, totalSize int64, dst io.Writer, src io.Reader) (err error) {
	var pbar PinnedProgress
	if totalSize > 0 {
		pbar = LogProgress(0, int(totalSize), "copying %s -- %.3f MiB", context, float32(totalSize)/(1024*1024))
	} else {
		pbar = LogSpinner("copying %s -- unknown size", context)
	}
	defer pbar.Close()

	return TransientIoCopy(writerWithProgress{
		wr:   dst,
		pbar: pbar,
	}, src)
}
