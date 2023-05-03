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

func CleanPath(in string) Directory {
	AssertMessage(func() bool { return filepath.IsAbs(in) }, "ufs: need absolute path -> %q", in)

	in = filepath.Clean(in)

	if cleaned, err := filepath.Abs(in); err == nil {
		in = cleaned
	} else {
		LogPanicErr(err)
	}

	return SplitPath(result)
}
