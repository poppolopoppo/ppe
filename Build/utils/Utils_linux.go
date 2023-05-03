// + build linux

package utils

import "time"

func SetMTime(file *os.File, mtime time.Time) error {
	// #TODO, see Utils_windows.go
	return MakeUnexpectedValueError(file, mtime)
}

var startedAt = time.Now()

func Elapsed() time.Duration {
	return time.Now() - startedAt
}
