// + build darwin

package utils

func SetMTime(file *os.File, mtime time.Time) error {
	// #TODO, see Utils_windows.go
	return utils.MakeUnexpectedValueError(file, mtime)
}
