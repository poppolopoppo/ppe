package utils

import (
	"fmt"
	"io"
	"log"
	"math"
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
			mainText: fmt.Sprintf(msg, args...) + " ",
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
	log.subText = strings.ReplaceAll(log.subText, ANSI_KILL_LINE.String(), "")
	log.subText = strings.ReplaceAll(log.subText, ANSI_CURSOR_UP.String(), "")
	pinnedLog.refresh()
}
func (log *pinnedLogScope) String() string {
	if log.subText != "" {
		const maxLen = 20
		elapsed := int(maxLen * time.Since(programStartedAt).Seconds())
		croppedText := [maxLen]byte{}
		for i := 0; i < maxLen; i += 1 {
			croppedText[i] = log.mainText[(elapsed+i)%len(log.mainText)]
		}
		return fmt.Sprintf("%20s %s", croppedText, log.subText)
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

func smootherstep(edge0, edge1, x float64) float64 {
	x = (x - edge0) / (edge1 - edge0)
	x = math.Max(0.0, math.Min(1.0, x))
	return x * x * x * (x*(x*6.-15.) + 10.)
}

func (pg *pinnedLogProgress) String() (result string) {
	const width = 70

	totalDuration := time.Since(pg.startedat)

	result += ANSI_FG1_CYAN.String() + " "

	colb0 := [3]uint8{0, 250, 154}  // medium spring green
	colb1 := [3]uint8{60, 139, 113} // medium sea green

	cola0 := [3]uint8{255, 202, 229} // bubble gum pink
	cola1 := [3]uint8{255, 119, 188} // dark  bubble gum pink

	col2 := [3]uint8{42, 29, 69} // dark indigo
	// col3 := [3]uint8{255, 255, 255} // white

	fillPattern := func(off, w int, pattern []string, swing bool) {
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
				result += make_ansi_color("fg1", ANSI_COLORS[(off+i)%m]).String()
				result += pattern[c]
			}
		}
		result += string(ANSI_RESET)
	}

	sliderPattern := func() {
		t := int(totalDuration.Milliseconds() / 30)
		pattern := []string{"`", "`", "'", "-", ".", ",", "_", ",", ".", "-", "'", "`", "`", "'", "-", ".", ",", "_", ",", ".", "=", "'", `"`}
		// pattern := []string{`▁`, `▂`, `▃`, `▄`, `▅`, `▆`, `▇`, `█`, `▇`, `▆`, `▅`, `▄`, `▃`, `▂`}
		fillPattern(t, width+1, pattern, false)
	}

	wavePattern := func() {
		pattern := []string{`▁`, `▂`, `▃`, `▄`, `▅`, `▆`, `▇`, `█`, `▇`, `▆`, `▅`, `▄`, `▃`, `▂`}
		t := float64(totalDuration.Seconds()) * 3.0

		for i := 0; i < width; i += 1 {
			y := t + 4*float64(i)*math.Pi/width
			x := y

			f := 0.0
			m := 0.5
			for b := 0; b < 5; b += 1 {
				f += m * (math.Cos(x+math.Sin((t+2.0*float64(i)/width)/m)*m*math.Pi)*0.5 + 0.5)
				m *= 0.5
				x *= 2.0
			}

			j := int(math.Floor(f * float64(len(pattern))))

			f = smootherstep(0.0, 1.0, f)
			result += make_ansi_bg_truecolor(col2[0], col2[1], col2[2])

			xx := float64(i) / width
			xx = math.Cos(t*2.5+xx*math.Phi*3.0)*0.5 + 0.5
			col0 := lerp_color(cola0, colb0, xx)
			col1 := lerp_color(cola1, colb1, xx)

			result += lerp_ansi_fg_truecolor(col0, col1, f)
			result += pattern[j]
		}
	}

	ballPattern := func() {
		ping := []string{"`", "`", "'", "-", ".", ",", "_", ",", ".", "-", "'", "`", "`", "'", "-", ".", ",", "_", ",", ".", "=", "'", `"`}
		// ping := []string{`▁`, `▂`, `▃`, `▄`, `▅`, `▆`, `▇`, `█`, `▇`, `▆`, `▅`, `▄`, `▃`, `▂`}

		pong := make([]string, width)
		for i := 0; i < width; i += 1 {
			pong[i] = " "
		}

		ball := func(x int, color string) {
			pos := (pg.startedat.Nanosecond() + x) % (width * 2)
			if pos >= width {
				pos = width*2 - pos - 1
			}
			frm := x % len(ping)
			pong[pos] = color + ping[frm]
		}

		ball(pg.tick, string(ANSI_FG1_MAGENTA))
		ball(pg.tick+1, string(ANSI_FG1_RED))
		ball(pg.tick+2, string(ANSI_FG1_YELLOW))
		ball(pg.tick+3, string(ANSI_FG1_WHITE))

		result += strings.Join(pong, "")
	}

	if pg.first < pg.last {
		result += ANSI_FG1_WHITE.String()
		f := float64(pg.progress-pg.first) / float64(pg.last-pg.first)
		f = math.Max(0.0, math.Min(1.0, f))
		i := int(f * width)

		for j := 0; j < width; j += 1 {
			f := float64(j+1) / width

			xx := math.Cos(float64(totalDuration.Seconds()*5.0)+f*math.Phi*4.0)*0.5 + 0.5
			col0 := lerp_color(cola0, colb0, xx)
			col1 := lerp_color(cola1, colb1, xx)

			a := float64(j & 1)
			if j < i {
				result += lerp_ansi_bg_truecolor(col1, col0, a*0.2+0.8)
				result += lerp_ansi_fg_truecolor(col0, col1, a*0.1+0.9)
				result += `▂` //`▁`
			} else if j == i {
				result += make_ansi_bg_truecolor(col2[0], col2[1], col2[2])
				result += lerp_ansi_fg_truecolor(col0, col1, a*0.1+0.9)
				result += `▌` // `▋` //`▎`
			} else {
				result += make_ansi_bg_truecolor(col2[0], col2[1], col2[2])
				result += ` `
			}
		}

		xx := math.Cos(float64(totalDuration.Seconds()*5.0)+1.0*math.Phi*4.0)*0.5 + 0.5
		col1 := lerp_color(cola1, colb1, xx)

		result += ANSI_RESET.String()
		if i < width {
			result += make_ansi_fg_truecolor(col2[0], col2[1], col2[2])
		} else {
			result += make_ansi_fg_truecolor(col1[0], col1[1], col1[2])
		}
		result += `▌` // `▋` //`▎`
		result += ANSI_RESET.String()

		result += lerp_ansi_fg_truecolor(cola1, colb0, f)
		result += fmt.Sprintf(" %6.2f", f*100) + "%%  " // Sprintf() escaping hell

		numElts := float32(pg.progress - pg.first)
		eltUnit := ""
		if numElts > 5000 {
			eltUnit = "K"
			numElts /= 1000
		}
		if numElts > 5000 {
			eltUnit = "M"
			numElts /= 1000
		}
		eltPerSec := numElts / float32(totalDuration.Seconds()+0.00001)
		result += ANSI_FG1_YELLOW.String()
		result += fmt.Sprintf(" %.3f %s/s", eltPerSec, eltUnit)
	} else {
		result += ANSI_FG0_CYAN.String()

		if true {
			wavePattern()
		} else if false {
			sliderPattern()
		} else {
			ballPattern()
		}

		result += ANSI_RESET.String()
		result += make_ansi_fg_truecolor(col2[0], col2[1], col2[2])
		result += `▌` // `▋` //`▎`

		result += ANSI_RESET.String()
		result += ANSI_FG0_GREEN.String()
		result += fmt.Sprintf(" %6.2fs ", totalDuration.Seconds())
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
	duration := time.Since(x.startedAt)
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

func CopyWithProgress(context string, totalSize int64, dst io.Writer, src io.Reader) (err error) {
	if enableInteractiveShell {
		var pbar PinnedProgress
		if totalSize > 0 {
			pbar = LogProgress(0, int(totalSize), context)
		} else {
			pbar = LogSpinner(context)
		}
		defer pbar.Close()

		for {
			var toRead int64 = TRANSIENT_BYTES_CAPACITY
			if toRead, err = io.CopyN(dst, src, toRead); err == nil {
				pbar.Add(int(toRead))
			} else {
				break
			}
		}

		if err == io.EOF {
			err = nil
		}

	} else {
		_, err = io.Copy(dst, src)
	}
	return err
}

func CopyWithSpinner(context string, dst io.Writer, src io.Reader) (err error) {
	return CopyWithProgress(context, 0, dst, src)
}
