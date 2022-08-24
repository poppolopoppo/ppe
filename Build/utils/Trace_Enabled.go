//go:build trace
// +build trace

package utils

import (
	"os"
	"runtime/trace"
)

const TRACE_ENABLED = true

func StartTrace() func() {
	if fd, err := os.Create("app.trace"); err == nil {
		trace.Start(fd)
		return func() {
			trace.Stop()
		}
	} else {
		panic(err)
	}
}
