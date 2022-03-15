//go:build !trace
// +build !trace

package utils

const TRACE_ENABLED = false

var StartTrace = func() func() {
	return func() {}
}
