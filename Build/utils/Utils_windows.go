// + build windows

package utils

import (
	"os"
	"path/filepath"
	"strings"
	"syscall"
	"time"
	"unicode"
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
					LogPanic(LogUFS, "SetMTime: timestamp mismatch for %q\n\tfound:\t\t%v\n\texpected:\t\t%v", file.Name(), info.ModTime(), mtime)
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

var cleanPathCache SharedMapT[string, string]

// normVolumeName is like VolumeName, but makes drive letter upper case.
// result of EvalSymlinks must be unique, so we have
// EvalSymlinks(`c:\a`) == EvalSymlinks(`C:\a`).
func normVolumeName(path string) string {
	volume := filepath.VolumeName(path)

	if len(volume) > 2 { // isUNC
		return volume
	}

	return strings.ToUpper(volume)
}

// normBase returns the last element of path with correct case.
func normBase(path string) (string, error) {
	p, err := syscall.UTF16PtrFromString(path)
	if err != nil {
		return "", err
	}

	var data syscall.Win32finddata

	h, err := syscall.FindFirstFile(p, &data)
	if err != nil {
		return "", err
	}
	syscall.FindClose(h)

	return syscall.UTF16ToString(data.FileName[:]), nil
}

func writeLowerString(w interface {
	WriteRune(rune) (int, error)
}, in string) error {
	// avoid string copy
	for _, ch := range in {
		if _, err := w.WriteRune(unicode.ToLower(ch)); err != nil {
			return err
		}
	}
	return nil
}

func cacheCleanPath(in string) (string, error) {
	// see filepath.normBase(), this version is using a cache for each sub-directory

	// skip special cases
	if in == "" || in == "." || in == `\` {
		return in, nil
	}

	volName := normVolumeName(in)
	in = in[len(volName)+1:]

	cleaned := TransientBuffer.Allocate()
	defer TransientBuffer.Release(cleaned)

	dirty := TransientBuffer.Allocate()
	defer TransientBuffer.Release(dirty)

	cleaned.Grow(len(in))
	dirty.Grow(len(in))

	cleaned.WriteString(volName)
	writeLowerString(dirty, volName)

	var err error
	for len(in) > 0 {
		dirty.WriteRune(OSPathSeparator)

		var dirtyName string
		if i, ok := firstIndexOfPathSeparator(in); ok {
			dirtyName = in[:i]
			in = in[i+1:]
		} else {
			dirtyName = in
			in = ""
		}
		writeLowerString(dirty, dirtyName)

		if err == nil {
			dirtyPath := UnsafeStringFromBuffer(dirty)

			if realpath, ok := cleanPathCache.Get(dirtyPath); ok {
				cleaned.Reset()
				cleaned.WriteString(realpath)

			} else if realname, er := normBase(dirtyPath); er == nil {
				cleaned.WriteRune(OSPathSeparator)
				cleaned.WriteString(realname)

				// store in cache for future queries, avoid querying all files all paths all the time
				cleanPathCache.Add(
					// need string copies for caching here
					dirty.String(), filepath.Clean(cleaned.String()))
			} else {
				err = er
			}
		}

		if err != nil {
			cleaned.WriteRune(OSPathSeparator)
			cleaned.WriteString(dirtyName)
		}
	}

	return filepath.Clean(cleaned.String()), err
}

func CleanPath(in string) string {
	AssertMessage(func() bool { return filepath.IsAbs(in) }, "ufs: need absolute path -> %q", in)

	// Those checks are cheap compared to the followings
	in = filepath.Clean(in)

	// Maximize cache usage by always convert to lower-case on Windows
	query := strings.ToLower(in)

	// /!\ EvalSymlinks() is **SUPER** expansive !
	// Try to mitigate with an ad-hoc concurrent cache
	if cleaned, ok := cleanPathCache.Get(query); ok {
		return cleaned // cache-hit: already processed
	}

	if cleaned, err := filepath.Abs(in); err == nil {
		in = cleaned
	} else {
		LogPanicErr(err)
	}

	// result, err := filepath.EvalSymlinks(in)
	result, err := cacheCleanPath(in)
	if err != nil {
		// result = in
		err = nil // if path does not exist yet
	}

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
