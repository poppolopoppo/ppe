package utils

import (
	"fmt"
	"io"
	"log"
	"os"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

type LogLevel int32

const (
	LOG_ALL LogLevel = iota
	LOG_DEBUG
	LOG_VERYVERBOSE
	LOG_TRACE
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
		return fmt.Sprint(ANSI_FG0_MAGENTA, ANSI_ITALIC, " ~ ", " ")
	case LOG_VERYVERBOSE:
		return fmt.Sprint(ANSI_FG1_BLACK, ANSI_ITALIC, "   ", " ")
	case LOG_TRACE:
		return fmt.Sprint(ANSI_FG0_BLUE, ANSI_ITALIC, "   ", " ")
	case LOG_VERBOSE:
		return fmt.Sprint(ANSI_FG0_CYAN, ANSI_ITALIC, " - ", " ")
	case LOG_INFO:
		return fmt.Sprint(ANSI_FG0_WHITE, ANSI_BG0_BLACK, "---", " ")
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

var enableInteractiveShell bool = false

func EnableInteractiveShell() bool {
	return enableInteractiveShell
}
func SetEnableInteractiveShell(enabled bool) {
	enableInteractiveShell = enabled
	SetEnableAnsiColor(enabled)
}

type logEvent func()
type logQueue interface {
	Queue(logEvent)
}

type logQueue_immediate struct{}

func make_logQueue_immediate() logQueue {
	return logQueue_immediate{}
}
func (x logQueue_immediate) Queue(e logEvent) { e() }

type logQueue_deferred struct {
	queue chan<- logEvent
}

func make_logQueue_deferred() logQueue {
	queue := make(chan logEvent)
	// start logging goroutine
	go func() {
		for {
			if event, ok := <-queue; ok {
				event()
			} else {
				break
			}
		}
	}()
	return &logQueue_deferred{queue}
}
func (x *logQueue_deferred) Queue(e logEvent) {
	x.queue <- e
}

type Logger struct {
	Level LogLevel
	logQueue
}

func make_logger() Logger {
	SetLogTimestamp(false)

	level := LOG_INFO
	if EnableDiagnostics() {
		level = LOG_ALL
	}

	return Logger{Level: level, logQueue: make_logQueue()}
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
func SetLogLevel(level LogLevel) {
	if level < LOG_FATAL {
		logger.Level = level
	} else {
		logger.Level = LOG_FATAL
	}
}
func IsLogLevelActive(level LogLevel) bool {
	return logger.Level <= level
}
func Log(level LogLevel, msg string, args ...interface{}) {
	if IsLogLevelActive(level) {
		logger.Queue(func() {
			pinnedLog.without(func() {
				log.Printf(level.String()+msg+ANSI_RESET.String()+"\n", args...)
			})
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
func LogForward(msg string, args ...interface{}) {
	pinnedLog.without(func() {
		log.Printf(msg, args...)
	})
}
func LogClaim(msg string, args ...interface{}) {
	Log(LOG_CLAIM, msg, args...)
}
func LogWarning(msg string, args ...interface{}) {
	Log(LOG_WARNING, msg, args...)
}
func LogError(msg string, args ...interface{}) {
	Log(LOG_ERROR, msg, args...)
}
func LogFatal(msg string, args ...interface{}) {
	log.Fatalf(msg, args...)
}
func LogPanic(msg string, args ...interface{}) {
	log.Panicf(fmt.Sprint(
		ANSI_FG1_RED, ANSI_BG1_WHITE, ANSI_BLINK0,
		"[PANIC] ", msg, ANSI_RESET),
		args...)
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
	Log(msg string, args ...interface{})
	fmt.Stringer
}

type pinnedLogFake struct{}

func (log *pinnedLogFake) Close()                     {}
func (log *pinnedLogFake) Log(string, ...interface{}) {}
func (log *pinnedLogFake) String() string             { return "" }

type pinnedLogScope struct {
	mainText string
	subText  string
}

type pinnedLogManager struct {
	stream   io.Writer
	barrier  sync.RWMutex
	messages SetT[PinnedLog]
	inflight int

	cooldown int32
	readers  int32
}

var pinnedFake = &pinnedLogFake{}
var pinnedLog = &pinnedLogManager{
	stream: os.Stderr,
}

func attachPinUnsafe(pin *pinnedLogManager) {
	if !pin.messages.Empty() && pin.inflight == 0 {
		pin.inflight = len(pin.messages)

		// format messages in local buffer
		buf := strings.Builder{}
		for _, x := range pin.messages.Slice() {
			fmt.Fprint(&buf,
				"\r", ANSI_KILL_LINE,
				ANSI_FG1_GREEN,
				x.String(),
				ANSI_RESET, "\n")
		}

		// flush with one call
		fmt.Fprintf(pin.stream, buf.String())
	}
}
func detachPinUnsafe(pin *pinnedLogManager) {
	if pin.inflight > 0 {
		buf := strings.Builder{}

		// format messages in local buffer
		for i := 0; i < pin.inflight; i += 1 {
			fmt.Fprint(&buf, ANSI_CURSOR_UP, ANSI_KILL_LINE)
		}
		pin.inflight = 0

		// flush with one call
		fmt.Fprintf(pin.stream, buf.String())
	}
}
func refreshPinUnsafe(pin *pinnedLogManager) {
	detachPinUnsafe(pin)
	attachPinUnsafe(pin)
}

func (pin *pinnedLogManager) push(msg string, args ...interface{}) PinnedLog {
	if msg != "" && enableInteractiveShell {
		log := &pinnedLogScope{
			mainText: fmt.Sprintf(msg, args...),
		}

		pin.barrier.Lock()
		defer pin.barrier.Unlock()

		pin.messages.Append(log)
		refreshPinUnsafe(pin)
		return log
	}
	return pinnedFake
}
func (pin *pinnedLogManager) pop(log PinnedLog) {
	if log != nil && enableInteractiveShell {
		pin.barrier.Lock()
		defer pin.barrier.Unlock()

		pin.messages.Remove(log)
		refreshPinUnsafe(pin)
	}
}
func (pin *pinnedLogManager) refresh() {
	if atomic.AddInt32(&pin.cooldown, 1) == 1 {
		pin.barrier.Lock()
		defer pin.barrier.Unlock()
		refreshPinUnsafe(pin)
		atomic.StoreInt32(&pin.cooldown, 0)
	}
}
func (pin *pinnedLogManager) without(block func()) {
	if true {
		pin.barrier.Lock()
		defer pin.barrier.Unlock()
		detachPinUnsafe(pin)
		block()
		attachPinUnsafe(pin)
	} else {
		block()
	}
}

func (log *pinnedLogScope) Close() {
	pinnedLog.pop(log)
}
func (log *pinnedLogScope) Log(msg string, args ...interface{}) {
	log.subText = fmt.Sprintf(msg, args...)
	log.subText = strings.TrimSpace(log.subText)
	log.subText = strings.ReplaceAll(log.subText, "\r", "")
	log.subText = strings.ReplaceAll(log.subText, ANSI_KILL_LINE, "")
	log.subText = strings.ReplaceAll(log.subText, ANSI_CURSOR_UP, "")
	pinnedLog.refresh()
}
func (log *pinnedLogScope) String() string {
	if log.subText != "" {
		return fmt.Sprintf("%20s -> %s", log.mainText, log.subText)
	} else {
		return log.mainText
	}
}

func LogPin(msg string, args ...interface{}) PinnedLog {
	return pinnedLog.push(msg, args...)
}

type PinnedProgress interface {
	Close()
	Add(int)
	Inc()
	Set(int)
}

type pinnedLogProgress struct {
	first, last int
	progress    int
	tick        int
	startedat   time.Time
	log         PinnedLog
}

func (pg *pinnedLogProgress) Close() {
	pg.log.Close()
}
func (pg *pinnedLogProgress) Add(n int) {
	pg.tick += 1
	pg.last += n
	pg.log.Log(pg.String())
}
func (pg *pinnedLogProgress) Inc() {
	pg.tick += 1
	pg.progress += 1
	pg.log.Log(pg.String())
}
func (pg *pinnedLogProgress) Set(x int) {
	pg.tick += 1
	pg.progress = x
	pg.log.Log(pg.String())
}
func (pg *pinnedLogProgress) String() (result string) {
	const width = 50
	const spinner = "qpbd"

	spin := pg.tick % len(spinner)
	totalSecs := time.Now().Sub(pg.startedat)

	result += " ["

	if pg.first < pg.last {
		f := float32(pg.progress-pg.first) / float32(pg.last-pg.first)
		if f < 0 {
			f = 0
		}
		if f > 1 {
			f = 1
		}
		i := int(f * width)

		if true {
			pos := 0
			if i > 0 {
				pos = pg.tick % width
				if pos > i {
					pos = 0
					pg.tick = 0
				} else if pg.progress == pg.last {
					pos = i
				}
			}
			result += strings.Repeat("=", pos) + ">" + strings.Repeat("=", i-pos)
		} else {
			pattern := "---=>->->-=>---"
			for j := 0; j < i; j += 1 {
				t := (pg.tick - j) % len(pattern)
				result += pattern[t : t+1]
			}
		}

		result += "]" + strings.Repeat("-", width-i)
		result += fmt.Sprintf(" %6.2f  ", f*100)
	} else {
		if true {
			i := pg.progress % (2*width - 2)
			if i > width {
				i = width*2 - i
			}

			result += strings.Repeat("-", i) + spinner[spin:spin+1] + strings.Repeat("-", width-i)
			result += fmt.Sprintf("] %6.2fs ", totalSecs.Seconds())
		} else {
			pattern := "-WAIT!" + spinner[spin:spin+1]
			for j := 0; j <= width; j += 1 {
				t := (pg.tick + j) % len(pattern)
				result += pattern[t : t+1]
			}
			result += "]"
		}
	}

	eltPerSec := float32(pg.progress-pg.first) / float32(totalSecs.Seconds())
	result += fmt.Sprintf(" %.3f/s", eltPerSec)

	if pg.progress == pg.last {
		result += " DONE"
	}
	return result
}

func LogProgress(first, last int, msg string, args ...interface{}) PinnedProgress {
	return &pinnedLogProgress{
		first:     first,
		last:      last,
		progress:  first,
		startedat: time.Now(),
		log:       LogPin(msg, args...),
	}
}
func LogSpinner(msg string, args ...interface{}) PinnedProgress {
	return &pinnedLogProgress{
		startedat: time.Now(),
		log:       LogPin(msg, args...),
	}
}

type benchmarkLog struct {
	message   string
	startedAt time.Time
}

func (x benchmarkLog) Close() {
	duration := time.Now().Sub(x.startedAt)
	LogTrace("Benchmark <%s>: %v", x.message, duration)
}
func LogBenchmark(msg string, args ...interface{}) Closable {
	formatted := fmt.Sprintf(msg, args...) // before measured scope
	return benchmarkLog{
		message:   formatted,
		startedAt: time.Now(),
	}
}
