// + build windows

package utils

import (
	"os"
	"path/filepath"
	"strings"
	"syscall"
	"time"
	"unsafe"
)

/***************************************
 * Avoid UFS.MTime() when we already opened an *os.File, this is much faster:
 * https://github.com/loov/hrtime/blob/master/now_windows.go
 ***************************************/

func SetMTime(file *os.File, mtime time.Time) (err error) {
	mtime = mtime.Local()
	wtime := syscall.NsecToFiletime(mtime.UnixNano())
	err = syscall.SetFileTime(syscall.Handle(file.Fd()), nil, nil, &wtime)
	if err == nil {
		Assert(func() bool {
			var info os.FileInfo
			if info, err = file.Stat(); err == nil {
				if info.ModTime() != mtime {
					LogPanic("SetMTime: timestamp mismatch for %q\n\tfound:\t\t%v\n\texpected:\t\t%v", file.Name(), info.ModTime(), mtime)
				}
			}
			return true
		})
	}
	return err
}

/***************************************
 * Cleaning path to get the correct case is terribly expansive on Windows
 ***************************************/

var cleanPathCache SharedMapT[string, Directory]

func CleanPath(in string) Directory {
	AssertMessage(func() bool { return filepath.IsAbs(in) }, "ufs: need absolute path -> %q", in)

	// Maximize cache usage by always convert to lower-case on Windows
	in = strings.ToLower(in)

	// Those checks are cheap compared to the followings
	in = filepath.Clean(in)

	// /!\ EvalSymlinks() is **SUPER** expansive !
	// Try to mitigate with an ad-hoc concurrent cache
	if cleaned, ok := cleanPathCache.Get(in); ok {
		return cleaned // cache-hit: already processed
	}

	if cleaned, err := filepath.Abs(in); err == nil {
		in = cleaned
	} else {
		LogPanicErr(err)
	}

	cleaned, err := filepath.EvalSymlinks(in)
	if err != nil {
		cleaned = in
		err = nil // if path does not exist yet
	}

	// Split path and store prebuilt directory to re-use this slice in the future
	result := SplitPath(cleaned)

	// Store cleaned path for future occurrences (expects many for directories)
	cleanPathCache.Add(in, result)
	return result
}

/***************************************
 * Use perf counter, which give more precision than time.Now() on Windows
 * https://github.com/loov/hrtime/blob/master/now_windows.go
 ***************************************/

// precision timing
var (
	modkernel32 = syscall.NewLazyDLL("kernel32.dll")
	procFreq    = modkernel32.NewProc("QueryPerformanceFrequency")
	procCounter = modkernel32.NewProc("QueryPerformanceCounter")

	qpcFrequency = getFrequency()
	qpcBase      = getCount()
)

// getFrequency returns frequency in ticks per second.
func getFrequency() int64 {
	var freq int64
	r1, _, _ := syscall.SyscallN(procFreq.Addr(), uintptr(unsafe.Pointer(&freq)))
	if r1 == 0 {
		panic("call failed")
	}
	return freq
}

// getCount returns counter ticks.
func getCount() int64 {
	var qpc int64
	syscall.SyscallN(procCounter.Addr(), uintptr(unsafe.Pointer(&qpc)))
	return qpc
}

// Now returns current time.Duration with best possible precision.
//
// Now returns time offset from a specific time.
// The values aren't comparable between computer restarts or between computers.
func Elapsed() time.Duration {
	return time.Duration(getCount()-qpcBase) * time.Second / (time.Duration(qpcFrequency) * time.Nanosecond)
}

// NowPrecision returns maximum possible precision for Now in nanoseconds.
func NowPrecision() float64 {
	return float64(time.Second) / (float64(qpcFrequency) * float64(time.Nanosecond))
}
