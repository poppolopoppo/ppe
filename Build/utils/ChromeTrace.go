package utils

import (
	"fmt"
	"io"
	"os"
	"reflect"
	"time"
)

/***************************************
 * Chrome Trace Format (for profiling)
 ***************************************/

// https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview

type ChromeTrace struct {
	TraceEvents     []ChromeTraceEvent `json:"traceEvents"`
	DisplayTimeUnit string             `json:"displayTimeUnit"`
}

type ChromeTraceEvent struct {
	Name      string `json:"name"`
	Category  string `json:"cat"`
	Phase     string `json:"ph"`
	Timestamp int64  `json:"ts"`
	Pid       int    `json:"pid"`
	Tid       int    `json:"tid"`
}

func NewChromeTrace() ChromeTrace {
	return ChromeTrace{
		DisplayTimeUnit: "ns",
	}
}
func (x *ChromeTrace) Save(dst Filename) error {
	return UFS.CreateBuffered(dst, func(w io.Writer) error {
		return JsonSerialize(x, w, OptionJsonPrettyPrint(true))
	})
}

/***************************************
 * ChromeTracer
 ***************************************/

var GlobalChromeTracer ChromeTracer = chromeTracerDummy{}

type ChromeTracer interface {
	Close() error
	BeginScope(name fmt.Stringer, category string, tid int) func()
	BeginDuration(name, category string, tid int)
	EndDuration(name, category string, tid int)
}

type chromeTracerDummy struct{}

func (x chromeTracerDummy) Close() error { return nil }
func (x chromeTracerDummy) BeginScope(name fmt.Stringer, category string, tid int) func() {
	return func() {}
}
func (x chromeTracerDummy) BeginDuration(name, category string, tid int) {}
func (x chromeTracerDummy) EndDuration(name, category string, tid int)   {}

type chromeTracer struct {
	ChromeTrace
	Pid        int
	OutputFile Filename
}

func NewChromeTracer(dst Filename) ChromeTracer {
	return &chromeTracer{
		ChromeTrace: NewChromeTrace(),
		Pid:         os.Getpid(),
		OutputFile:  dst,
	}
}

func SetupBuildGraphChromeTracer(dst Filename) {
	GlobalChromeTracer = NewChromeTracer(dst)
	bg := CommandEnv.buildGraph
	tid := 1234
	bg.OnBuildGraphStart().Add(func(bg BuildGraph) error {
		GlobalChromeTracer.BeginDuration("Build-Graph", PROCESS_INFO.Path.Basename, tid)
		return nil
	})
	bg.OnBuildNodeStart().Add(func(bn BuildNode) error {
		category := reflectTypename(reflect.TypeOf(bn.GetBuildable()))
		GlobalChromeTracer.BeginDuration(bn.Alias().String(), category, tid)
		return nil
	})
	bg.OnBuildNodeFinished().Add(func(bn BuildNode) error {
		category := reflectTypename(reflect.TypeOf(bn.GetBuildable()))
		GlobalChromeTracer.EndDuration(bn.Alias().String(), category, tid)
		return nil
	})
	bg.OnBuildGraphFinished().Add(func(bg BuildGraph) error {
		GlobalChromeTracer.EndDuration("Build-Graph", PROCESS_INFO.Path.Basename, tid)
		return nil
	})
	CommandEnv.OnExit(func(cet *CommandEnvT) error {
		return GlobalChromeTracer.Close()
	})
}

func (x *chromeTracer) Close() error {
	return x.Save(x.OutputFile)
}
func (x *chromeTracer) Add(name, category string, tid int, phase string) {
	x.TraceEvents = append(x.TraceEvents, ChromeTraceEvent{
		Name:      name,
		Category:  category,
		Phase:     phase,
		Timestamp: time.Since(CommandEnv.startedAt).Microseconds(),
		Pid:       x.Pid,
		Tid:       tid,
	})
}

func (x *chromeTracer) BeginScope(name fmt.Stringer, category string, tid int) func() {
	x.BeginDuration(name.String(), category, tid)
	return func() {
		x.EndDuration(name.String(), category, tid)
	}
}
func (x *chromeTracer) BeginDuration(name, category string, tid int) {
	x.Add(name, category, tid, "B")
}
func (x *chromeTracer) EndDuration(name, category string, tid int) {
	x.Add(name, category, tid, "E")
}
