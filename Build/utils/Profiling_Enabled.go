//go:build profiling
// +build profiling

package utils

import (
	"github.com/pkg/profile"
)

const PROFILING_ENABLED = true
const PROFILING_MEMORY = false

var ProfilingTag = MakeArchiveTag(MakeFourCC('P', 'R', 'O', 'F'))

var running_profiler interface {
	Stop()
}

func StartProfiling() func() {
	mode := profile.CPUProfile
	if PROFILING_MEMORY {
		mode = profile.MemProfile
	}
	running_profiler = profile.Start(
		mode,
		profile.NoShutdownHook,
		profile.ProfilePath("."))
	return PurgeProfiling
}

func PurgeProfiling() {
	if running_profiler != nil {
		running_profiler.Stop()
		running_profiler = nil
	}
}
