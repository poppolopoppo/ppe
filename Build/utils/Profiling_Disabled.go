//go:build !profiling
// +build !profiling

package utils

const PROFILING_ENABLED = false

func StartProfiling() func() {
	return PurgeProfiling
}

func PurgeProfiling() {}
