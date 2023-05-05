package utils

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"math"
	"math/rand"
	"os"
	"reflect"
	"sync"
	"sync/atomic"
	"time"
)

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

func (x LogLevel) String() string {
	switch x {
	case LOG_DEBUG:
		return fmt.Sprint(ANSI_FG0_MAGENTA, ANSI_ITALIC, ANSI_FAINT, " ~ ", " ")
	case LOG_TRACE:
		return fmt.Sprint(ANSI_FG0_CYAN, ANSI_ITALIC, " - ", " ")
	case LOG_VERYVERBOSE:
		return fmt.Sprint(ANSI_FG1_MAGENTA, ANSI_ITALIC, ANSI_ITALIC, "   ", " ")
	case LOG_VERBOSE:
		return fmt.Sprint(ANSI_FG0_BLUE, "   ", " ")
	case LOG_INFO:
		return fmt.Sprint(ANSI_FG1_WHITE, ANSI_BG0_BLACK, "---", " ")
	case LOG_CLAIM:
		return fmt.Sprint(ANSI_FG1_GREEN, ANSI_BG0_BLACK, ANSI_BOLD, "-+-", " ")
	case LOG_WARNING:
		return fmt.Sprint(ANSI_FG0_YELLOW, "/?\\", " ")
	case LOG_ERROR:
		return fmt.Sprint(ANSI_FG1_RED, ANSI_BOLD, "/!\\", " ")
	case LOG_FATAL:
		return fmt.Sprint(ANSI_FG1_WHITE, ANSI_BG0_RED, ANSI_BLINK0, "[!]", " ")
	default:
		UnexpectedValue(x)
	}
	return ""
}

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

type logEvent func()
type logQueue interface {
	Flush()
	Queue(logEvent)
}

type logQueue_immediate struct {
	barrier sync.Mutex
}

func make_logQueue_immediate() logQueue {
	return &logQueue_immediate{barrier: sync.Mutex{}}
}
func (x *logQueue_immediate) Flush() {}
func (x *logQueue_immediate) Queue(e logEvent) {
	x.barrier.Lock()
	defer x.barrier.Unlock()
	e()
}

type logQueue_deferred struct {
	pool WorkerPool
}

func make_logQueue_deferred() logQueue {
	pool := GetLoggerWorkerPool()
	return logQueue_deferred{pool}
}
func (x logQueue_deferred) Flush() {
	x.pool.Join()
}
func (x logQueue_deferred) Queue(e logEvent) {
	x.pool.Queue(TaskFunc(e))
}

type Logger struct {
	Level        LogLevel
	WarningLevel LogLevel
	logQueue
}

func make_logger() Logger {
	SetLogTimestamp(false)

	level := LOG_INFO
	if EnableDiagnostics() {
		level = LOG_ALL
	}

	return Logger{
		Level:        level,
		WarningLevel: LOG_WARNING,
		logQueue:     make_logQueue()}
}

var logger Logger = make_logger()

func SetLogOutput(dst io.Writer) {
	log.SetOutput(dst)
}
func SetLogTimestamp(enabled bool) {
	if enabled {
		log.SetFlags(log.Lmicroseconds)
	} else {
		log.SetFlags(0)
	}
}
func SetLogWarningAsError(enabled bool) bool {
	previous := logger.WarningLevel != LOG_WARNING
	if enabled {
		logger.WarningLevel = LOG_ERROR
	} else {
		logger.WarningLevel = LOG_WARNING
	}
	return previous
}
func SetLogLevel(level LogLevel) LogLevel {
	previous := logger.Level
	if level < LOG_FATAL {
		logger.Level = level
	} else {
		logger.Level = LOG_FATAL
	}
	return previous
}
func SetLogLevelMininum(level LogLevel) LogLevel {
	previous := logger.Level
	if level < LOG_FATAL && level < logger.Level {
		logger.Level = level
	}
	return previous
}
func SetLogLevelMaximum(level LogLevel) LogLevel {
	previous := logger.Level
	if level < LOG_FATAL && level > logger.Level {
		logger.Level = level
	}
	return previous
}
func IsLogLevelActive(level LogLevel) bool {
	return logger.Level <= level
}
func FlushLog() {
	logger.Flush()
}
func Log(level LogLevel, msg string, args ...interface{}) {
	if IsLogLevelActive(level) {
		pinnedLog().without(func() {
			log.Printf(level.String()+msg+ANSI_RESET.String()+"\n", args...)
			if level >= LOG_WARNING {
				log_callstack()
			}
		})
	}
}

func LogDebug(msg string, args ...interface{}) {
	Log(LOG_DEBUG, msg, args...)
}
func LogDebugIf(enabled bool, msg string, args ...interface{}) {
	if enabled {
		Log(LOG_DEBUG, msg, args...)
	}
}
func LogVeryVerbose(msg string, args ...interface{}) {
	Log(LOG_VERYVERBOSE, msg, args...)
}
func LogTrace(msg string, args ...interface{}) {
	Log(LOG_TRACE, msg, args...)
}
func LogVerbose(msg string, args ...interface{}) {
	Log(LOG_VERBOSE, msg, args...)
}
func LogInfo(msg string, args ...interface{}) {
	Log(LOG_INFO, msg, args...)
}
func LogClaim(msg string, args ...interface{}) {
	Log(LOG_CLAIM, msg, args...)
}
func LogWarning(msg string, args ...interface{}) {
	Log(logger.WarningLevel, msg, args...)
}
func LogError(msg string, args ...interface{}) {
	Log(LOG_ERROR, msg, args...)
}
func LogFatal(msg string, args ...interface{}) {
	WithoutLog(func() {
		log.Fatalf(msg, args...)
	})
}

func LogPanic(msg string, args ...interface{}) {
	LogPanicErr(fmt.Errorf(msg, args...))
}
func LogPanicErr(err error) {
	LogError("panic: caught error %v", err)

	if CommandEnv == nil || CommandEnv.OnPanic(err) {
		PurgePinnedLogs()
		panic(fmt.Errorf("%v%v%v[PANIC] %v%v",
			ANSI_FG1_RED, ANSI_BG1_WHITE, ANSI_BLINK0, err, ANSI_RESET))
	} else {
		panic("panic reentrancy!")
	}
}
func LogPanicIfFailed(err error) {
	if err != nil {
		LogPanicErr(err)
	}
}

func LogForward(msg string) {
	pinnedLog().without(func() {
		log.Print(msg)
	})
}
func LogForwardf(format string, args ...interface{}) {
	pinnedLog().without(func() {
		log.Printf(format, args...)
	})
}
func WithoutLog(block func()) {
	pinnedLog().without(block)
}

func MakeError(msg string, args ...interface{}) error {
	LogError(msg, args...)
	return fmt.Errorf(msg, args...)
}

func MakeUnexpectedValueError(dst interface{}, any interface{}) error {
	return MakeError("unexpected <%v> value '%v'", reflect.TypeOf(dst), any)
}
func UnexpectedValuePanic(dst interface{}, any interface{}) {
	LogPanicErr(MakeUnexpectedValueError(dst, any))
}

type FormatFunc func() string

func MakeFormat[T any](fn func() T) FormatFunc {
	return func() string {
		return fmt.Sprint(fn())
	}
}
func (fn FormatFunc) String() string {
	return fn()
}

type PinnedLog interface {
	Close()
	Log(msg func(io.Writer))
}

type pinnedLogFake struct{}

func (log *pinnedLogFake) Close()              {}
func (log *pinnedLogFake) Log(func(io.Writer)) {}

type pinnedLogScope struct {
	mainText string
	subText  atomic.Pointer[func(io.Writer)]
}

type pinnedLogManager struct {
	stream   io.Writer
	messages SetT[*pinnedLogScope]
	inflight int
	maxLen   int

	cooldown int32
}

var pinnedFake = &pinnedLogFake{}
var pinnedLogRefresh func()
var pinnedLog = Memoize(func() *pinnedLogManager {
	manager := &pinnedLogManager{
		stream: os.Stderr,
	}
	pinnedLogRefresh = manager.refresh
	return manager
})

func attachPinUnsafe(pin *pinnedLogManager) {
	if !pin.messages.Empty() && pin.inflight == 0 {
		pin.inflight = len(pin.messages)

		// format messages in local buffer
		tmp := TransientSmallPage.Allocate()
		defer TransientSmallPage.Release(tmp)

		buf := bytes.NewBuffer(tmp)
		buf.Reset()

		fmt.Println(buf, "")

		pin.maxLen = 0
		for _, x := range pin.messages {
			if x == nil {
				continue
			}

			offset := buf.Len()

			fmt.Fprint(buf, "\r", ANSI_ERASE_END_LINE.Always(), ANSI_FG1_YELLOW)
			x.Print(buf)
			fmt.Fprintln(buf, ANSI_RESET)

			if len := int(buf.Len() - offset); pin.maxLen < len {
				pin.maxLen = len
			}
		}

		// flush with one call
		pin.stream.Write(buf.Bytes())
	}
}
func detachPinUnsafe(pin *pinnedLogManager, clear bool) {
	inflight := pin.inflight
	if inflight > 0 {
		// format messages in local buffer
		tmp := TransientSmallPage.Allocate()
		defer TransientSmallPage.Release(tmp)

		buf := bytes.NewBuffer(tmp)
		buf.Reset()

		fmt.Fprint(buf,
			ANSI_ERASE_ALL_LINE.Always(),
			"\033[", inflight+1, "F", // move cursor up  # lines
			ANSI_ERASE_SCREEN_FROM_CURSOR.Always())

		pin.inflight = 0

		// flush with one call
		pin.stream.Write(buf.Bytes())
	}
}
func refreshPinUnsafe(pin *pinnedLogManager) {
	if ref := atomic.AddInt32(&pin.cooldown, -1); ref == 0 {
		detachPinUnsafe(pin, false)
		attachPinUnsafe(pin)
	}
}

func (pin *pinnedLogManager) push(msg string, args ...interface{}) PinnedLog {
	if len(msg) > 0 && enableInteractiveShell {
		log := &pinnedLogScope{
			mainText: fmt.Sprintf(msg, args...),
		}
		atomic.AddInt32(&pin.cooldown, 1)
		logger.Queue(func() {
			pin.messages.Append(log)
			refreshPinUnsafe(pin)
		})
		return log
	}
	return pinnedFake
}
func (pin *pinnedLogManager) pop(log *pinnedLogScope) {
	if log != nil {
		atomic.AddInt32(&pin.cooldown, 1)
		logger.Queue(func() {
			pin.messages.Remove(log)
			refreshPinUnsafe(pin)
		})
	}
}
func (pin *pinnedLogManager) refresh() {
	if enableInteractiveShell {
		atomic.AddInt32(&pin.cooldown, 1)
		logger.Queue(func() {
			refreshPinUnsafe(pin)
		})
	}
}
func (pin *pinnedLogManager) without(block func()) {
	if enableInteractiveShell {
		logger.Queue(func() {
			detachPinUnsafe(pin, false)
			block()
			attachPinUnsafe(pin)
		})
	} else {
		block()
	}
}
func (pin *pinnedLogManager) forceClose() {
	detachPinUnsafe(pin, true)
}

func (log *pinnedLogScope) Close() {
	pinnedLog().pop(log)
}
func (log *pinnedLogScope) Log(msg func(io.Writer)) {
	if msg != nil {
		log.subText.Store(&msg)
		pinnedLog().refresh()
	} else {
		log.subText.Store(nil)
	}
}
func (log *pinnedLogScope) Print(dst io.Writer) {
	printCropped := func(buf []byte, capacity int, in string) {
		elapsed := int(float64(capacity) * time.Since(programStartedAt).Seconds())
		for i := 0; i < capacity; {
			ci := (elapsed + i) % len(in)
			ch := in[ci]
			switch ch {
			case '\r', '\n':
				continue
			case '\t':
				ch = ' '
				fallthrough
			default:
				if ci == 0 {
					buf[i] = ' '
					i += 1
				}
				if i < capacity {
					buf[i] = ch
					i += 1
				}
			}
		}
		dst.Write(buf)
	}

	if subText := log.subText.Load(); subText != nil {
		const maxLen = 20
		buf := [maxLen]byte{}
		printCropped(buf[:], int(maxLen), log.mainText)
		(*subText)(dst)
	} else {
		const maxLen = 100
		buf := [maxLen]byte{}
		printCropped(buf[:], int(maxLen), log.mainText)
	}
}

func LogPin(msg string, args ...interface{}) PinnedLog {
	return pinnedLog().push(msg, args...)
}

func PurgePinnedLogs() {
	JoinAllWorkerPools()
	logger.Flush()
	pinnedLog().forceClose()
}

type PinnedProgress interface {
	Close()
	Grow(int)
	Add(int)
	Inc()
	Set(int)
	Reset()
}

type noopLogProgress struct{}

func (x noopLogProgress) Close()   {}
func (x noopLogProgress) Grow(int) {}
func (x noopLogProgress) Add(int)  {}
func (x noopLogProgress) Inc()     {}
func (x noopLogProgress) Set(int)  {}
func (x noopLogProgress) Reset()   {}

type pinnedLogProgress struct {
	first     int
	last      atomic.Int32
	progress  atomic.Int32
	tick      atomic.Int32
	startedat time.Time
	log       PinnedLog

	color [3]uint8

	sync.Mutex
}

func (pg *pinnedLogProgress) Close() {
	pg.log.Close()
}
func (pg *pinnedLogProgress) Grow(n int) {
	pg.Lock()
	pg.last.Add(int32(n))
	pg.tick.Add(1)
	pg.log.Log(pg.Print)
	pg.Unlock()
}
func (pg *pinnedLogProgress) Add(n int) {
	pg.Invalidate(int(pg.progress.Add(int32(n))) + n)
}
func (pg *pinnedLogProgress) Inc() {
	pg.Add(1)
}
func (pg *pinnedLogProgress) Set(x int) {
	for {
		prev := pg.progress.Load()
		if prev > int32(x) {
			break
		}
		if pg.progress.CompareAndSwap(prev, int32(x)) {
			pg.Invalidate(x)
			break
		}
	}
}
func (pg *pinnedLogProgress) Reset() {
	pg.progress.Store(int32(pg.first))
	pg.Invalidate(pg.first)
}
func (pg *pinnedLogProgress) Invalidate(x int) {
	pg.Lock()
	if pg.progress.Load() == int32(x) {
		pg.tick.Add(1)
		pg.log.Log(pg.Print)
	}
	pg.Unlock()
}
func (pg *pinnedLogProgress) SetRandomColors() {
	pg.color = pastelizer_truecolor(rand.Float64())
}

func (pg *pinnedLogProgress) Print(dst io.Writer) {
	const width = 50

	progress := int(pg.progress.Load())
	duration := time.Since(pg.startedat)
	t := float64(duration.Seconds()+float64(pg.color[0])) * 3.0

	fmt.Fprint(dst, ` `)

	last := int(pg.last.Load())
	if pg.first < last {
		// progress-bar

		fmt.Fprint(dst, ANSI_FG1_WHITE.String())
		ff := float64(progress-pg.first) / float64(last-pg.first)
		ff = math.Max(0.0, math.Min(1.0, ff))
		fi := int(ff * (width - 1))

		for i := 0; i < width; i += 1 {
			ft := smootherstep(math.Cos(t*2+float64(i)/(width-1)*math.Pi)*0.5 + 0.5)
			mi := 0.49 + float64(i&1)*0.02

			if i < fi {
				bg := expose_truecolor(pg.color, ft*.3+.38)
				fg := expose_truecolor(pg.color, mi)
				fmt.Fprint(dst,
					make_ansi_fg_truecolor(bg[0], bg[1], bg[2]),
					make_ansi_bg_truecolor(fg[0], fg[1], fg[2]),
					`▂`) //`▁`)

			} else {
				bg := expose_truecolor(pg.color, ft*.09+.3)
				fmt.Fprint(dst,
					make_ansi_bg_truecolor(bg[0], bg[1], bg[1]),
					` `)
			}
		}

		fmt.Fprint(dst, ANSI_RESET.String())
		fmt.Fprintf(dst, " %6.2f%% ", ff*100) // Sprintf() escaping hell

		numElts := float32(progress - pg.first)
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
		fmt.Fprint(dst, ANSI_FG1_YELLOW.String())
		fmt.Fprintf(dst, " %.3f %s/s", eltPerSec, eltUnit)

	} else {
		// spinner

		fmt.Fprint(dst, ANSI_FG0_CYAN.String())

		pattern := []string{` `, `▁`, `▂`, `▃`, `▄`, `▅`, `▆`, `▇`}

		for i := 0; i < width; i += 1 {
			x := math.Pi * 100 * float64(i) / (width - 1)

			const phi float64 = 1.0 / 1.61803398875
			const gravitationalConst float64 = 9.81

			var waves_x, waves_y float64
			var steepness float64 = 1.0
			var wavelength float64 = 100.0
			for i := 0; i < 5; i += 1 {
				k := 2.0 * math.Pi / wavelength
				c := math.Sqrt(float64(gravitationalConst / k))
				a := steepness / k
				f := k * (x - c*t*steepness*4)

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

			fmt.Fprint(dst)

			fg := expose_truecolor(pg.color, smootherstep(f)*.3+.35)
			bg := expose_truecolor(pg.color, (math.Cos(t+float64(i)/(width-1)*math.Pi)*0.5+0.5)*.2+.3)

			fmt.Fprint(dst,
				make_ansi_bg_truecolor(bg[0], bg[1], bg[2]),
				make_ansi_fg_truecolor(fg[0], fg[1], fg[2]),
				pattern[c])
		}

		fmt.Fprint(dst, ANSI_RESET.String(), ANSI_FG0_GREEN.String())
		fmt.Fprintf(dst, " %6.2fs ", duration.Seconds())
	}

	if progress == last {
		fmt.Fprint(dst, " DONE")
	}
}

func LogProgress(first, last int, msg string, args ...interface{}) PinnedProgress {
	if enableInteractiveShell {
		result := &pinnedLogProgress{
			first:     first,
			startedat: time.Now(),
			log:       LogPin(msg, args...),
		}
		result.last.Store(int32(last))
		result.progress.Store(int32(first))
		result.SetRandomColors()
		result.Invalidate(first)
		return result
	} else {
		return noopLogProgress{}
	}
}
func LogSpinner(msg string, args ...interface{}) PinnedProgress {
	if enableInteractiveShell {
		result := &pinnedLogProgress{
			startedat: time.Now(),
			log:       LogPin(msg, args...),
		}
		result.SetRandomColors()
		result.Invalidate(0)
		return result
	} else {
		return noopLogProgress{}
	}
}

type BenchmarkLog struct {
	message   string
	startedAt time.Duration
}

func (x BenchmarkLog) Close() time.Duration {
	duration := Elapsed() - x.startedAt
	LogVeryVerbose("benchmark: %10v   %s", duration, x.message)
	return duration
}
func LogBenchmark(msg string, args ...interface{}) BenchmarkLog {
	formatted := fmt.Sprintf(msg, args...) // before measured scope
	return BenchmarkLog{
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
