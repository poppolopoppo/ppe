// + build windows

package utils

import (
	"os"
	"syscall"
	"time"
)

// avoid UFS.MTime() when we already opened an *os.File, this is much faster:
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
