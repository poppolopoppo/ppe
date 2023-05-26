package utils

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"log"
	"math"
	"math/rand"
	"os"
	"reflect"
	"runtime"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

/***************************************
 * Logger API
 ***************************************/

var LogGlobal = NewLogCategory("Global")

var gLogger Logger = newDeferredLogger(newInteractiveLogger(newBasicLogger()))

func LogDebug(category LogCategory, msg string, args ...interface{}) {
	gLogger.Log(category, LOG_DEBUG, msg, args...)
}
func LogDebugIf(category LogCategory, enabled bool, msg string, args ...interface{}) {
	if enabled {
		gLogger.Log(category, LOG_DEBUG, msg, args...)
	}
}
func LogVeryVerbose(category LogCategory, msg string, args ...interface{}) {
	gLogger.Log(category, LOG_VERYVERBOSE, msg, args...)
}
func LogTrace(category LogCategory, msg string, args ...interface{}) {
	gLogger.Log(category, LOG_TRACE, msg, args...)
}
func LogVerbose(category LogCategory, msg string, args ...interface{}) {
	gLogger.Log(category, LOG_VERBOSE, msg, args...)
}
func LogInfo(category LogCategory, msg string, args ...interface{}) {
	gLogger.Log(category, LOG_INFO, msg, args...)
}
func LogClaim(category LogCategory, msg string, args ...interface{}) {
	gLogger.Log(category, LOG_CLAIM, msg, args...)
}
func LogWarning(category LogCategory, msg string, args ...interface{}) {
	gLogger.Log(category, LOG_WARNING, msg, args...)
}
func LogError(category LogCategory, msg string, args ...interface{}) {
	gLogger.Log(category, LOG_ERROR, msg, args...)
}
func LogFatal(msg string, args ...interface{}) {
	gLogger.Purge()
	log.Fatalf(msg, args...)
}

func LogPanic(category LogCategory, msg string, args ...interface{}) {
	LogPanicErr(category, fmt.Errorf(msg, args...))
}
func LogPanicErr(category LogCategory, err error) {
	LogError(category, "panic: caught error %v", err)

	gLogger.Purge()

	if CommandEnv == nil || CommandEnv.OnPanic(err) {
		panic(fmt.Errorf("%v%v%v[PANIC] %v%v",
			ANSI_FG1_RED, ANSI_BG1_WHITE, ANSI_BLINK0, err, ANSI_RESET))
	} else {
		panic(fmt.Errorf("panic reentrancy: %v", err))
	}
}
func LogPanicIfFailed(category LogCategory, err error) {
	if err != nil {
		LogPanicErr(category, err)
	}
}

func LogForward(msg string) {
	gLogger.Forward(msg)
}
func LogForwardln(msg string) {
	gLogger.Forwardln(msg)
}
func LogForwardf(format string, args ...interface{}) {
	gLogger.Forwardf(format, args...)
}

func WithoutLog(block func()) {
	gLogger.WithoutPin(block)
}

func IsLogLevelActive(level LogLevel) bool {
	return gLogger.IsVisible(level)
}
func FlushLog() {
	gLogger.Flush()
}

/***************************************
 * Logger interface
 ***************************************/

type PinScope interface {
	Log(msg string, args ...interface{})
	Closable

	format(LogWriter)
}

type ProgressScope interface {
	Grow(int)
	Add(int)
	Inc()
	Set(int)
	PinScope
}

type LogCategory struct {
	Name  string
	Level LogLevel
	Hash  uint32
	Color [3]uint8
}

type LogWriter interface {
	io.Writer
	io.StringWriter
}

type Logger interface {
	IsInteractive() bool
	IsVisible(LogLevel) bool

	SetLevel(LogLevel) LogLevel
	SetLevelMaximum(LogLevel) LogLevel
	SetLevelMinimum(LogLevel) LogLevel
	SetWarningAsError(bool)
	SetShowCategory(bool)
	SetShowTimestamp(bool)
	SetWriter(LogWriter)

	Forward(msg string)
	Forwardln(msg string)
	Forwardf(msg string, args ...interface{})

	Log(category LogCategory, level LogLevel, msg string, args ...interface{})

	Pin(msg string, args ...interface{}) PinScope
	Progress(first, last int, msg string, args ...interface{}) ProgressScope
	WithoutPin(func())
	Close(PinScope)

	Flush()   // wait for all pending all messages
	Purge()   // close the log and every on-going pins
	Refresh() // re-draw all pins, need for animated effects

}

var logCategoryIndex atomic.Uint32

func NewLogCategory(name string) LogCategory {
	hash := logCategoryIndex.Add(1)
	_, frac := math.Modf((float64(hash) + math.Pi) * 1.61803398875)
	return LogCategory{
		Name:  name,
		Level: LOG_ALL,
		Hash:  hash,
		Color: pastelizer_truecolor(frac),
	}
}

func MakeError(msg string, args ...interface{}) error {
	LogError(LogGlobal, msg, args...)
	return fmt.Errorf(msg, args...)
}

func MakeUnexpectedValueError(dst interface{}, any interface{}) error {
	return MakeError("unexpected <%v> value '%v'", reflect.TypeOf(dst), any)
}
func UnexpectedValuePanic(dst interface{}, any interface{}) {
	LogPanicErr(LogGlobal, MakeUnexpectedValueError(dst, any))
}

/***************************************
 * Log level
 ***************************************/

type LogLevel int32

const (
	LOG_ALL LogLevel = iota
	LOG_DEBUG
	LOG_TRACE
	LOG_VERYVERBOSE
	LOG_VERBOSE
	LOG_INFO
	LOG_CLAIM
	LOG_WARNING
	LOG_ERROR
	LOG_FATAL
)

func (x LogLevel) IsVisible(level LogLevel) bool {
	return (int32(level) >= int32(x))
}
func (x LogLevel) Style(dst io.Writer) {
	switch x {
	case LOG_DEBUG:
		fmt.Fprint(dst, ANSI_FG0_MAGENTA, ANSI_ITALIC, ANSI_FAINT)
	case LOG_TRACE:
		fmt.Fprint(dst, ANSI_FG0_CYAN, ANSI_ITALIC, ANSI_FAINT)
	case LOG_VERYVERBOSE:
		fmt.Fprint(dst, ANSI_FG1_MAGENTA, ANSI_ITALIC, ANSI_ITALIC)
	case LOG_VERBOSE:
		fmt.Fprint(dst, ANSI_FG0_BLUE)
	case LOG_INFO:
		fmt.Fprint(dst, ANSI_FG1_WHITE, ANSI_BG0_BLACK)
	case LOG_CLAIM:
		fmt.Fprint(dst, ANSI_FG1_GREEN, ANSI_BG0_BLACK, ANSI_BOLD)
	case LOG_WARNING:
		fmt.Fprint(dst, ANSI_FG0_YELLOW)
	case LOG_ERROR:
		fmt.Fprint(dst, ANSI_FG1_RED, ANSI_BOLD)
	case LOG_FATAL:
		fmt.Fprint(dst, ANSI_FG1_WHITE, ANSI_BG0_RED, ANSI_BLINK0)
	default:
		UnexpectedValue(x)
	}
}
func (x LogLevel) Header(dst io.Writer) {
	switch x {
	case LOG_DEBUG:
		fmt.Fprint(dst, "  ~  ")
	case LOG_TRACE:
		fmt.Fprint(dst, "  -  ")
	case LOG_VERYVERBOSE:
		fmt.Fprint(dst, "     ")
	case LOG_VERBOSE:
		fmt.Fprint(dst, "     ")
	case LOG_INFO:
		fmt.Fprint(dst, " --- ")
	case LOG_CLAIM:
		fmt.Fprint(dst, " -+- ")
	case LOG_WARNING:
		fmt.Fprint(dst, " /?\\ ")
	case LOG_ERROR:
		fmt.Fprint(dst, " /!\\ ")
	case LOG_FATAL:
		fmt.Fprint(dst, " [!] ")
	default:
		UnexpectedValue(x)
	}
}
func (x LogLevel) String() string {
	outp := strings.Builder{}
	x.Header(&outp)
	return outp.String()
}

/***************************************
 * Basic Logger
 ***************************************/

type basicLogPin struct{}

func (x basicLogPin) Log(string, ...interface{}) {}
func (x basicLogPin) Close()                     {}
func (x basicLogPin) format(LogWriter)           {}

type basicLogProgress struct {
	basicLogPin
}

func (x basicLogProgress) Grow(int) {}
func (x basicLogProgress) Add(int)  {}
func (x basicLogProgress) Inc()     {}
func (x basicLogProgress) Set(int)  {}

type basicLogger struct {
	MinimumLevel   LogLevel
	WarningAsError bool
	ShowCategory   bool
	ShowTimestamp  bool
	Writer         *bufio.Writer
}

func newBasicLogger() *basicLogger {
	level := LOG_INFO
	if EnableDiagnostics() {
		level = LOG_ALL
	}

	return &basicLogger{
		MinimumLevel:   level,
		WarningAsError: false,
		ShowCategory:   true,
		ShowTimestamp:  false,
		Writer:         bufio.NewWriter(os.Stdout),
	}
}

func (x *basicLogger) IsInteractive() bool {
	return false
}
func (x *basicLogger) IsVisible(level LogLevel) bool {
	return x.MinimumLevel.IsVisible(level)
}

func (x *basicLogger) SetLevel(level LogLevel) LogLevel {
	previous := x.MinimumLevel
	if level < LOG_FATAL {
		x.MinimumLevel = level
	} else {
		x.MinimumLevel = LOG_FATAL
	}
	return previous
}
func (x *basicLogger) SetLevelMinimum(level LogLevel) LogLevel {
	previous := x.MinimumLevel
	if level < LOG_FATAL && level < x.MinimumLevel {
		x.MinimumLevel = level
	}
	return previous
}
func (x *basicLogger) SetLevelMaximum(level LogLevel) LogLevel {
	previous := x.MinimumLevel
	if level < LOG_FATAL && level > x.MinimumLevel {
		x.MinimumLevel = level
	}
	return previous
}
func (x *basicLogger) SetWarningAsError(enabled bool) {
	x.WarningAsError = enabled
}
func (x *basicLogger) SetShowCategory(enabled bool) {
	x.ShowCategory = enabled
}
func (x *basicLogger) SetShowTimestamp(enabled bool) {
	x.ShowTimestamp = enabled
}
func (x *basicLogger) SetWriter(dst LogWriter) {
	Assert(func() bool { return !IsNil(dst) })
	x.Writer.Flush()
	x.Writer.Reset(dst)
}

func (x *basicLogger) Forward(msg string) {
	x.Writer.WriteString(msg)
}
func (x *basicLogger) Forwardln(msg string) {
	x.Writer.WriteString(msg)
	if !strings.HasSuffix(msg, "\n") {
		x.Writer.WriteRune('\n')
	}

	x.flushLogToAvoidCropIFN()
}
func (x *basicLogger) Forwardf(msg string, args ...interface{}) {
	fmt.Fprintf(x.Writer, msg, args...)
	fmt.Fprintln(x.Writer, "")

	x.flushLogToAvoidCropIFN()
}

func (x *basicLogger) Log(category LogCategory, level LogLevel, msg string, args ...interface{}) {
	// warning as error?
	if level == LOG_WARNING && x.WarningAsError {
		level = LOG_ERROR
	}

	// log level visible?
	if !x.IsVisible(level) || !category.Level.IsVisible(level) {
		return
	}

	// format message
	if x.ShowTimestamp {
		fmt.Fprintf(x.Writer, "%s%08f |%s  ", ANSI_FG1_BLACK, Elapsed().Seconds(), ANSI_RESET)
	}

	level.Style(x.Writer)
	level.Header(x.Writer)

	if x.ShowCategory {
		fmt.Fprintf(x.Writer, " %s%s%s%s: ", ANSI_RESET, make_ansi_fg_truecolor(category.Color[:]...), category.Name, ANSI_RESET)
		level.Style(x.Writer)
	}

	fmt.Fprintf(x.Writer, msg, args...)

	x.Writer.WriteString(ANSI_RESET.String())
	x.Writer.WriteRune('\n')

	x.flushLogToAvoidCropIFN()
}

func (x *basicLogger) Pin(msg string, args ...interface{}) PinScope {
	return basicLogPin{} // see interactiveLogger struct
}
func (x *basicLogger) Progress(first, last int, msg string, args ...interface{}) ProgressScope {
	return basicLogProgress{} // see interactiveLogger struct
}
func (x *basicLogger) WithoutPin(block func()) {
	block() // see interactiveLogger struct
}
func (x *basicLogger) Close(pin PinScope) {
	UnreachableCode() // see interactiveLogger struct
}

func (x *basicLogger) Flush()   { x.Writer.Flush() }
func (x *basicLogger) Purge()   { x.Writer.Flush() }
func (x *basicLogger) Refresh() { x.Writer.Flush() }

func (x *basicLogger) flushLogToAvoidCropIFN() {
	if x.Writer.Available() < 200 {
		x.Writer.Flush()
	}
}

/***************************************
 * Deferred Logger
 ***************************************/

type deferredPinScope struct {
	future Future[PinScope]
}

func (x deferredPinScope) Log(msg string, args ...interface{}) {
	x.future.Join().Success().Log(msg, args...)
}
func (x deferredPinScope) Close() {
	x.future.Join().Success().Close()
}
func (x deferredPinScope) format(dst LogWriter) {
	x.future.Join().Success().format(dst)
}

type deferredProgressScope struct {
	future Future[ProgressScope]
}

func (x deferredProgressScope) Log(msg string, args ...interface{}) {
	x.future.Join().Success().Log(msg, args...)
}
func (x deferredProgressScope) Close() {
	x.future.Join().Success().Close()
}
func (x deferredProgressScope) format(dst LogWriter) {
	x.future.Join().Success().format(dst)
}

func (x deferredProgressScope) Grow(n int) {
	x.future.Join().Success().Grow(n)
}
func (x deferredProgressScope) Add(n int) {
	x.future.Join().Success().Add(n)
}
func (x deferredProgressScope) Inc() {
	x.future.Join().Success().Inc()
}
func (x deferredProgressScope) Set(v int) {
	x.future.Join().Success().Set(v)
}

type deferredLogger struct {
	logger  Logger
	thread  WorkerPool
	barrier *sync.Mutex
}

func newDeferredLogger(logger Logger) deferredLogger {
	barrier := &sync.Mutex{}
	return deferredLogger{
		logger:  logger,
		barrier: barrier,
		thread: NewFixedSizeWorkerPoolEx("logger", 1,
			func(fswp *fixedSizeWorkerPool, i int) {
				onWorkerThreadStart(fswp, i)
				defer onWorkerThreadStop(fswp, i)

				runTask := func(task TaskFunc) {
					barrier.Lock()
					defer barrier.Unlock()
					task()
				}

				for quit := false; !quit; {
					if logger.IsInteractive() {
						// refresh pinned logs if no message output after a while
						select {
						case task := (<-fswp.give):
							if task != nil {
								runTask(task)
							} else {
								quit = true
							}
						case <-time.After(33 * time.Millisecond):
							logger.Refresh()
						}
					} else {
						if task := (<-fswp.give); task != nil {
							runTask(task)
						} else {
							quit = true // a nil task means quit
						}
					}
				}
			}),
	}
}

func (x deferredLogger) IsInteractive() bool {
	return x.logger.IsInteractive()
}
func (x deferredLogger) IsVisible(level LogLevel) bool {
	return x.logger.IsVisible(level)
}

func (x deferredLogger) SetLevel(level LogLevel) LogLevel {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	return x.logger.SetLevel(level)
}
func (x deferredLogger) SetLevelMinimum(level LogLevel) LogLevel {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	return x.logger.SetLevelMinimum(level)
}
func (x deferredLogger) SetLevelMaximum(level LogLevel) LogLevel {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	return x.logger.SetLevelMaximum(level)
}
func (x deferredLogger) SetWarningAsError(enabled bool) {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	x.logger.SetWarningAsError(enabled)
}
func (x deferredLogger) SetShowCategory(enabled bool) {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	x.logger.SetShowCategory(enabled)
}
func (x deferredLogger) SetShowTimestamp(enabled bool) {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	x.logger.SetShowTimestamp(enabled)
}
func (x deferredLogger) SetWriter(dst LogWriter) {
	x.thread.Queue(func() {
		x.logger.SetWriter(dst)
	})
}

func (x deferredLogger) Forward(msg string) {
	x.thread.Queue(func() {
		x.logger.Forward(msg)
	})
}
func (x deferredLogger) Forwardln(msg string) {
	x.thread.Queue(func() {
		x.logger.Forwardln(msg)
	})
}
func (x deferredLogger) Forwardf(msg string, args ...interface{}) {
	x.thread.Queue(func() {
		x.logger.Forwardf(msg, args...)
	})
}
func (x deferredLogger) Log(category LogCategory, level LogLevel, msg string, args ...interface{}) {
	if category.Level.IsVisible(level) && x.logger.IsVisible(level) {
		x.thread.Queue(func() {
			x.logger.Log(category, level, msg, args...)
		})
	}
	if level >= LOG_ERROR {
		x.thread.Join() // flush log when an error occurred
	}
}
func (x deferredLogger) Pin(msg string, args ...interface{}) PinScope {
	return deferredPinScope{
		future: MakeWorkerFuture(x.thread, func() (PinScope, error) {
			pin := x.logger.Pin(msg, args...)
			return pin, nil
		})}
}
func (x deferredLogger) Progress(first, last int, msg string, args ...interface{}) ProgressScope {
	return deferredProgressScope{
		future: MakeWorkerFuture(x.thread, func() (ProgressScope, error) {
			pin := x.logger.Progress(first, last, msg, args...)
			return pin, nil
		})}
}
func (x deferredLogger) WithoutPin(block func()) {
	x.thread.Queue(func() {
		x.logger.WithoutPin(block)
	})
}
func (x deferredLogger) Close(pin PinScope) {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	x.logger.Close(pin)
}
func (x deferredLogger) Flush() {
	x.thread.Queue(func() {
		x.logger.Flush()
	})
	x.thread.Join()
}
func (x deferredLogger) Purge() {
	x.thread.Queue(func() {
		x.logger.Purge()
	})
	x.thread.Join()
}
func (x deferredLogger) Refresh() {
	x.thread.Queue(func() {
		x.logger.Refresh()
	})
}

/***************************************
 * Interactive Logger
 ***************************************/

var enableInteractiveShell bool = true
var forceInteractiveShell bool = false

func EnableInteractiveShell() bool {
	return enableInteractiveShell
}
func SetEnableInteractiveShell(enabled bool) {
	if enableInteractiveShell {
		enabled = enabled || forceInteractiveShell
		enableInteractiveShell = enabled
		SetEnableAnsiColor(enabled)
	}
}

type interactiveLogPin struct {
	header string
	writer func(LogWriter)

	tick      int
	first     int
	last      atomic.Int32
	progress  atomic.Int32
	startedAt time.Duration

	color [3]byte
}

func (x *interactiveLogPin) reset() {
	x.header = ""
	x.writer = nil
	x.tick = 0
	x.first = 0
	x.startedAt = 0
}
func (x *interactiveLogPin) format(dst LogWriter) {
	x.writer(dst)
}

func (x *interactiveLogPin) Log(msg string, args ...interface{}) {
	x.header = fmt.Sprintf(msg, args...)
}
func (x *interactiveLogPin) Close() {
	gLogger.Close(x)
}

func (x *interactiveLogPin) Grow(n int) {
	x.last.Add(int32(n))
}
func (x *interactiveLogPin) Add(n int) {
	x.progress.Add(int32(n))
}
func (x *interactiveLogPin) Inc() {
	x.progress.Add(1)
}
func (x *interactiveLogPin) Set(v int) {
	for {
		prev := x.progress.Load()
		if prev > int32(v) || x.progress.CompareAndSwap(prev, int32(v)) {
			break
		}
	}
}

type interactiveLogger struct {
	messages SetT[*interactiveLogPin]
	inflight int
	maxLen   int

	recycler  Recycler[*interactiveLogPin]
	transient bytes.Buffer
	*basicLogger
}

func newInteractiveLogger(basic *basicLogger) *interactiveLogger {
	return &interactiveLogger{
		messages:    make([]*interactiveLogPin, 0, runtime.NumCPU()),
		inflight:    0,
		maxLen:      0,
		basicLogger: basic,
		recycler: NewRecycler(
			func() *interactiveLogPin {
				return new(interactiveLogPin)
			},
			func(ip *interactiveLogPin) {
				ip.reset()
			}),
	}
}
func (x *interactiveLogger) IsInteractive() bool {
	return true
}
func (x *interactiveLogger) Forward(msg string) {
	x.WithoutPin(func() {
		x.basicLogger.Forward(msg)
	})
}
func (x *interactiveLogger) Forwardln(msg string) {
	x.WithoutPin(func() {
		x.basicLogger.Forwardln(msg)
	})
}
func (x *interactiveLogger) Forwardf(msg string, args ...interface{}) {
	x.WithoutPin(func() {
		x.basicLogger.Forwardf(msg, args...)
	})
}
func (x *interactiveLogger) Log(category LogCategory, level LogLevel, msg string, args ...interface{}) {
	x.WithoutPin(func() {
		x.basicLogger.Log(category, level, msg, args...)
	})
}
func (x *interactiveLogger) Pin(msg string, args ...interface{}) PinScope {
	if enableInteractiveShell {
		pin := x.recycler.Allocate()
		pin.Log(msg, args...)
		pin.startedAt = Elapsed()
		pin.color = pastelizer_truecolor(rand.Float64())
		pin.writer = pin.writeLogHeader

		x.messages.Append(pin)
		x.attachMessages()
		return pin
	}
	return basicLogPin{}
}
func (x *interactiveLogger) Progress(first, last int, msg string, args ...interface{}) ProgressScope {
	if enableInteractiveShell {
		pin := x.recycler.Allocate()
		pin.Log(msg, args...)
		pin.startedAt = Elapsed()
		pin.first = first
		pin.last.Store(int32(last))
		pin.progress.Store(0)
		pin.color = pastelizer_truecolor(rand.Float64())
		pin.writer = pin.writeLogProgress

		x.messages.Append(pin)
		x.attachMessages()
		return pin
	}
	return basicLogProgress{}
}
func (x *interactiveLogger) WithoutPin(block func()) {
	if x.hasInflightMessages() {
		x.detachMessages()
		block()
		x.attachMessages()
	} else {
		block()
	}
}
func (x *interactiveLogger) Close(scope PinScope) {
	if !IsNil(scope) {
		pin := scope.(*interactiveLogPin)
		x.messages.Remove(pin)
		x.recycler.Release(pin)
	}
}
func (x *interactiveLogger) Flush() {
	x.basicLogger.Flush()
}
func (x *interactiveLogger) Purge() {
	x.detachMessages()
	x.basicLogger.Purge()
}
func (x *interactiveLogger) Refresh() {
	if x.hasInflightMessages() {
		x.detachMessages()
		x.basicLogger.Refresh()
		x.attachMessages()
	}
}

func (x *interactiveLogger) hasInflightMessages() bool {
	return x.inflight > 0
}
func (x *interactiveLogger) attachMessages() bool {
	if x.inflight != 0 || x.messages.Empty() {
		return false
	}

	x.inflight = len(x.messages)
	x.maxLen = 0

	// format pins in memory
	defer x.transient.Reset()
	fmt.Fprintln(&x.transient, "")

	for _, it := range x.messages {
		if it != nil {
			offset := x.transient.Len()

			fmt.Fprint(&x.transient, "\r", ANSI_ERASE_END_LINE.Always(), make_ansi_fg_truecolor(it.color[:]...))
			{
				it.format(&x.transient)
			}
			fmt.Fprintln(&x.transient, ANSI_RESET.Always())

			if len := int(x.transient.Len() - offset); x.maxLen < len {
				x.maxLen = len
			}
		} else {
			x.inflight -= 1
		}
	}

	// write all output with 1 call
	os.Stderr.Write(x.transient.Bytes())

	return true
}
func (x *interactiveLogger) detachMessages() bool {
	if x.inflight == 0 {
		return false
	}

	// format pins in memory
	defer x.transient.Reset()

	fmt.Fprint(&x.transient,
		ANSI_ERASE_ALL_LINE.Always(),
		"\033[", x.inflight+1, "F", // move cursor up  # lines
		ANSI_ERASE_SCREEN_FROM_CURSOR.Always())

	// write all output with 1 call
	os.Stderr.Write(x.transient.Bytes())

	x.inflight = 0
	return true
}

/***************************************
 * Log Progress
 ***************************************/

func writeLogCropped(dst io.Writer, buf []byte, capacity int, in string) {
	i := int(Elapsed().Seconds() * 20)
	for w := 0; w < capacity; i += 1 {
		ci := i % len(in)
		ch := in[ci]
		switch ch {
		case '\r', '\n':
			continue
		case '\t':
			ch = ' '
			fallthrough
		default:
			if ci == 0 {
				buf[w] = ' '
				w += 1
			}
			if w < capacity {
				buf[w] = ch
				w += 1
			}
		}
	}
	dst.Write(buf)
}

func (x *interactiveLogPin) writeLogHeader(lw LogWriter) {
	const width = 100
	buf := [width]byte{}
	writeLogCropped(lw, buf[:], width, x.header)
}

func (x *interactiveLogPin) writeLogProgress(lw LogWriter) {
	const width = 50

	header := [25]byte{}
	writeLogCropped(lw, header[:], len(header), x.header)
	lw.WriteString(" ")

	progress := int(x.progress.Load())
	last := int(x.last.Load())

	duration := Elapsed() - x.startedAt
	t := float64(duration.Seconds()+float64(x.color[0])) * 5.0

	if x.first < last {
		// progress-bar (%)

		lw.WriteString(ANSI_FG1_WHITE.String())
		ff := float64(progress-x.first) / float64(last-x.first)
		ff = math.Max(0.0, math.Min(1.0, ff))
		fi := int(ff * (width - 1))

		for i := 0; i < width; i += 1 {
			ft := smootherstep(math.Cos(t*2+float64(i)/(width-1)*math.Pi)*0.5 + 0.5)
			mi := 0.49 + float64(i&1)*0.02

			if i < fi {
				bg := expose_truecolor(x.color, ft*.3+.38)
				fg := expose_truecolor(x.color, mi)
				fmt.Fprint(lw,
					make_ansi_fg_truecolor(bg[0], bg[1], bg[2]),
					make_ansi_bg_truecolor(fg[0], fg[1], fg[2]),
					`▂`) //`▁`)

			} else {
				bg := expose_truecolor(x.color, ft*.09+.3)
				fmt.Fprint(lw,
					make_ansi_bg_truecolor(bg[0], bg[1], bg[1]),
					` `)
			}
		}

		lw.WriteString(ANSI_RESET.String())
		fmt.Fprintf(lw, " %6.2f%% ", ff*100) // Sprintf() escaping hell

		numElts := float32(progress - x.first)
		eltUnit := ""
		if numElts > 5000 {
			eltUnit = "K"
			numElts /= 1000
		}
		if numElts > 5000 {
			eltUnit = "M"
			numElts /= 1000
		}

		eltPerSec := numElts / float32(duration.Seconds()+0.00001)
		lw.WriteString(ANSI_FG1_YELLOW.String())
		fmt.Fprintf(lw, " %.3f %s/s", eltPerSec, eltUnit)

	} else {
		// spinner (?)

		lw.WriteString(ANSI_FG0_CYAN.String())

		pattern := []string{` `, `▁`, `▂`, `▃`, `▄`, `▅`, `▆`, `▇`}

		for i := 0; i < width; i += 1 {
			lx := math.Pi * 100 * float64(i) / (width - 1)

			const phi float64 = 1.0 / 1.61803398875
			const gravitationalConst float64 = 9.81

			var waves_x, waves_y float64
			var steepness float64 = 1.0
			var wavelength float64 = 100.0
			for i := 0; i < 5; i += 1 {
				k := 2.0 * math.Pi / wavelength
				c := math.Sqrt(float64(gravitationalConst / k))
				a := steepness / k
				f := k * (lx - c*t*steepness*4)

				waves_x += a * math.Sin(f)
				waves_y += a
				steepness *= phi
				wavelength *= phi
			}

			f := (0.5*waves_x/waves_y + 0.5)
			if f > 1 {
				f = 1
			} else if f < 0 {
				f = 0
			}
			// f = float64(i) / width
			c := int(math.Round(float64(len(pattern)-1) * f))

			fg := expose_truecolor(x.color, smootherstep(f)*.3+.35)
			bg := expose_truecolor(x.color, (math.Cos(t+float64(i)/(width-1)*math.Pi)*0.5+0.5)*.2+.3)

			fmt.Fprint(lw,
				make_ansi_bg_truecolor(bg[0], bg[1], bg[2]),
				make_ansi_fg_truecolor(fg[0], fg[1], fg[2]),
				pattern[c])
		}

		lw.WriteString(ANSI_RESET.String())
		lw.WriteString(ANSI_FG0_GREEN.String())
		fmt.Fprintf(lw, " %6.2fs ", duration.Seconds())
	}

	if progress == last {
		lw.WriteString("DONE")
	}
}

/***************************************
 * Logger helpers
 ***************************************/

func PurgePinnedLogs() {
	gLogger.Purge()
}

func LogProgress(first, last int, msg string, args ...interface{}) ProgressScope {
	return gLogger.Progress(first, last, msg, args...)
}
func LogSpinner(msg string, args ...interface{}) ProgressScope {
	return gLogger.Progress(1, 0, msg, args...)
}

type BenchmarkLog struct {
	category  LogCategory
	message   string
	startedAt time.Duration
}

func (x BenchmarkLog) Close() time.Duration {
	duration := Elapsed() - x.startedAt
	LogVeryVerbose(x.category, "benchmark: %10v   %s", duration, x.message)
	return duration
}
func LogBenchmark(category LogCategory, msg string, args ...interface{}) BenchmarkLog {
	formatted := fmt.Sprintf(msg, args...) // before measured scope
	return BenchmarkLog{
		category:  category,
		message:   formatted,
		startedAt: Elapsed(),
	}
}

type lambdaStringer func() string

func (x lambdaStringer) String() string {
	return x()
}
func MakeStringer(fn func() string) fmt.Stringer {
	return lambdaStringer(fn)
}

func CopyWithProgress(context string, totalSize int64, dst io.Writer, src io.Reader) (err error) {
	if enableInteractiveShell {
		return TransientIoCopyWithProgress(context, totalSize, dst, src)
	} else {
		return TransientIoCopy(dst, src)
	}
}

func CopyWithSpinner(context string, dst io.Writer, src io.Reader) (err error) {
	return CopyWithProgress(context, 0, dst, src)
}
