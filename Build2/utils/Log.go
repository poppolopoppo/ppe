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
		return fmt.Sprint(ANSI_FG0_MAGENTA, ANSI_ITALIC, ANSI_FAINT, " ~ ", " ")
	case LOG_VERYVERBOSE:
		return fmt.Sprint(ANSI_FG1_BLACK, ANSI_ITALIC, ANSI_FAINT, "   ", " ")
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
	Flush()
	Queue(logEvent)
}

type logQueue_immediate struct{}

func make_logQueue_immediate() logQueue {
	return logQueue_immediate{}
}
func (x logQueue_immediate) Flush()           {}
func (x logQueue_immediate) Queue(e logEvent) { e() }

type logQueue_deferred struct {
	queue chan<- logEvent
}

func make_logQueue_deferred() logQueue {
	queue := make(chan logEvent)
	x := &logQueue_deferred{queue}
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
	return x
}
func (x *logQueue_deferred) Flush() {
	wg := sync.WaitGroup{}
	wg.Add(1)
	x.Queue(func() {
		defer wg.Done()
	})
	wg.Wait()
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
func FlushLog() {
	logger.Flush()
}
func Log(level LogLevel, msg string, args ...interface{}) {
	if IsLogLevelActive(level) {
		logger.Queue(func() {
			pinnedLog.without(func() {
				log.Printf(level.String()+msg+ANSI_RESET.String()+"\n", args...)
				if level >= LOG_WARNING {
					log_callstack()
				}
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
func LogForward(msg string) {
	pinnedLog.without(func() {
		log.Print(msg)
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
	messages SetT[*pinnedLogScope]
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
		fmt.Println(&buf)
		for _, x := range pin.messages {
			fmt.Fprintln(&buf,
				"\r", ANSI_KILL_LINE,
				ANSI_FG1_YELLOW,
				x,
				ANSI_RESET)
		}

		// flush with one call
		fmt.Fprintf(pin.stream, buf.String())
	}
}
func detachPinUnsafe(pin *pinnedLogManager, clear bool) {
	if pin.inflight > 0 {
		// format messages in local buffer
		buf := strings.Builder{}
		if false && len(pin.messages) == pin.inflight && !clear {
			// one clear line: ghosting
			for i := 0; i < pin.inflight+1; i += 1 {
				fmt.Fprint(&buf, ANSI_CURSOR_UP)
			}
			fmt.Fprint(&buf, ANSI_KILL_LINE)
		} else {
			// multiple clear lines: flicker
			for i := 0; i < pin.inflight+1; i += 1 {
				fmt.Fprint(&buf, ANSI_CURSOR_UP, ANSI_KILL_LINE)
			}
		}

		pin.inflight = 0

		// flush with one call
		fmt.Fprintf(pin.stream, buf.String())
	}
}
func refreshPinUnsafe(pin *pinnedLogManager) {
	detachPinUnsafe(pin, false)
	attachPinUnsafe(pin)
}

func (pin *pinnedLogManager) push(msg string, args ...interface{}) PinnedLog {
	if msg != "" && enableInteractiveShell {
		log := &pinnedLogScope{
			mainText: fmt.Sprintf(msg, args...),
		}

		logger.Queue(func() {
			pin.barrier.Lock()
			defer pin.barrier.Unlock()

			pin.messages.Append(log)
			refreshPinUnsafe(pin)
		})
		return log
	}
	return pinnedFake
}
func (pin *pinnedLogManager) pop(log *pinnedLogScope) {
	if log != nil {
		logger.Queue(func() {
			pin.barrier.Lock()
			defer pin.barrier.Unlock()

			pin.messages.Remove(log)
			refreshPinUnsafe(pin)
		})
	}
}
func (pin *pinnedLogManager) refresh() {
	if enableInteractiveShell && atomic.AddInt32(&pin.cooldown, 1) == 1 {
		logger.Queue(func() {
			pin.barrier.Lock()
			defer pin.barrier.Unlock()

			refreshPinUnsafe(pin)
			atomic.StoreInt32(&pin.cooldown, 0)
		})
	}
}
func (pin *pinnedLogManager) without(block func()) {
	if enableInteractiveShell {
		pin.barrier.Lock()
		defer pin.barrier.Unlock()
		detachPinUnsafe(pin, false)
		block()
		attachPinUnsafe(pin)
	} else {
		block()
	}
}
func (pin *pinnedLogManager) forceClose() {
	pin.barrier.Lock() // can't use pinned log after calling this
	detachPinUnsafe(pin, true)
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
		const maxLen = 30
		off := len(log.mainText) - maxLen
		if off < 0 {
			off = 0
		}
		return fmt.Sprintf("%30s %s", log.mainText[off:], log.subText)
	} else {
		return log.mainText
	}
}

func LogPin(msg string, args ...interface{}) PinnedLog {
	return pinnedLog.push(msg, args...)
}

func PurgePinnedLogs() {
	logger.Flush()
	pinnedLog.forceClose()
}

type PinnedProgress interface {
	Close()
	Add(int)
	Inc()
	Set(int)
}

type noopLogProgress struct{}

func (x noopLogProgress) Close()  {}
func (x noopLogProgress) Add(int) {}
func (x noopLogProgress) Inc()    {}
func (x noopLogProgress) Set(int) {}

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
	const width = 60

	totalSecs := time.Now().Sub(pg.startedat)

	result += ANSI_FG1_CYAN.String() + " ["

	fillPattern := func(off, w int, pattern string, swing bool) {
		n := len(pattern)
		m := len(ANSI_COLORS)
		for i := 0; i < w; i += 1 {
			c := (off + i) % n
			if swing {
				off += i/2 + 1
			}
			if i%20 == 0 && i > 1 && i+1 != w {
				result += string(ANSI_FG1_WHITE) + "|"
			} else if i%10 == 0 && i > 1 && i+1 != w {
				result += string(ANSI_FG1_CYAN) + "-"
			} else {
				result += make_ansi_color("fg1", ANSI_COLORS[(i)%m]).String()
				result += pattern[c : c+1]
			}
		}
		result += string(ANSI_RESET)
	}

	if pg.first < pg.last {
		result += ANSI_FG1_WHITE.String()
		f := float32(pg.progress-pg.first) / float32(pg.last-pg.first)
		if f < 0 {
			f = 0
		}
		if f > 1 {
			f = 1
		}
		i := int(f * width)

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
		result += strings.Repeat("=", pos)
		result += string(ANSI_FG1_YELLOW) + string(ANSI_BLINK0)
		result += ">"
		result += string(ANSI_RESET) + ANSI_FG1_WHITE.String()
		result += strings.Repeat("=", i-pos)
		result += ANSI_FG1_CYAN.String()
		result += "]"
		result += ANSI_FG1_BLACK.String()
		result += strings.Repeat("-", width-i)
		result += ANSI_FG1_GREEN.String()
		result += fmt.Sprintf(" %6.2f  ", f*100)

		eltPerSec := float32(pg.progress-pg.first) / float32(totalSecs.Seconds())
		result += ANSI_FG1_YELLOW.String()
		result += fmt.Sprintf(" %.3f/s", eltPerSec)
	} else {
		result += ANSI_FG0_CYAN.String()
		t := int((totalSecs.Milliseconds() / 30))
		fillPattern(t, width+1, "``'-.,_,.-'``'-.,_,.='``'-.,_,.-'``'-.,_,.='", false)
		result += ANSI_FG1_CYAN.String()
		result += "]"
		result += ANSI_FG0_GREEN.String()
		result += fmt.Sprintf(" %6.2fs ", totalSecs.Seconds())
	}

	if pg.progress == pg.last {
		result += " DONE"
	}
	return result
}

func LogProgress(first, last int, msg string, args ...interface{}) PinnedProgress {
	if enableInteractiveShell {
		return &pinnedLogProgress{
			first:     first,
			last:      last,
			progress:  first,
			startedat: time.Now(),
			log:       LogPin(msg, args...),
		}
	} else {
		return noopLogProgress{}
	}
}
func LogSpinner(msg string, args ...interface{}) PinnedProgress {
	if enableInteractiveShell {
		return &pinnedLogProgress{
			startedat: time.Now(),
			log:       LogPin(msg, args...),
		}
	} else {
		return noopLogProgress{}
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

type lambdaStringer func() string

func (x lambdaStringer) String() string {
	return x()
}
func MakeStringer(fn func() string) fmt.Stringer {
	return lambdaStringer(fn)
}
