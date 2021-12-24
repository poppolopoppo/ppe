//go:build !profiling
// +build !profiling

package utils

const PROFILING_ENABLED = false

var StartProfiling = func() func() {
	return func() {}
}
