//go:build profiling
// +build profiling

package utils

import (
	"github.com/pkg/profile"
)

const PROFILING_ENABLED = true

var StartProfiling = func() func() {
	x := profile.Start(profile.ProfilePath("."))
	return func() {
		x.Stop()
	}
}
