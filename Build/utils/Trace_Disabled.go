//go:build !trace
// +build !trace

package utils

const TRACE_ENABLED = false

func StartTrace() func() {
	return func() {}
}
