package utils

import (
	"fmt"
	"math"
	"strconv"
)

type AnsiCode string

var enableAnsiColor bool = true

func SetEnableAnsiColor(enabled bool) {
	if enableAnsiColor {
		enableAnsiColor = enabled
	}
}

func (x AnsiCode) Always() string {
	return (string)(x)
}
func (x AnsiCode) String() string {
	if enableAnsiColor {
		return (string)(x)
	}
	return ""
}

// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

const (
	ANSI_RESET     AnsiCode = "\033[0m"
	ANSI_BOLD      AnsiCode = "\033[1m"
	ANSI_FAINT     AnsiCode = "\033[2m"
	ANSI_ITALIC    AnsiCode = "\033[3m"
	ANSI_UNDERLINE AnsiCode = "\033[4m"
	ANSI_BLINK0    AnsiCode = "\033[5m"
	ANSI_BLINK1    AnsiCode = "\033[6m"
	ANSI_REVERSED  AnsiCode = "\033[7m"
	ANSI_FRAME     AnsiCode = "\033[51m"
	ANSI_ENCIRCLE  AnsiCode = "\033[52m"
	ANSI_OVERLINE  AnsiCode = "\033[53m"

	ANSI_FG0_BLACK   AnsiCode = "\033[30m"
	ANSI_FG0_RED     AnsiCode = "\033[31m"
	ANSI_FG0_GREEN   AnsiCode = "\033[32m"
	ANSI_FG0_YELLOW  AnsiCode = "\033[33m"
	ANSI_FG0_BLUE    AnsiCode = "\033[34m"
	ANSI_FG0_MAGENTA AnsiCode = "\033[35m"
	ANSI_FG0_CYAN    AnsiCode = "\033[36m"
	ANSI_FG0_WHITE   AnsiCode = "\033[37m"
	ANSI_FG1_BLACK   AnsiCode = "\033[30;1m"
	ANSI_FG1_RED     AnsiCode = "\033[31;1m"
	ANSI_FG1_GREEN   AnsiCode = "\033[32;1m"
	ANSI_FG1_YELLOW  AnsiCode = "\033[33;1m"
	ANSI_FG1_BLUE    AnsiCode = "\033[34;1m"
	ANSI_FG1_MAGENTA AnsiCode = "\033[35;1m"
	ANSI_FG1_CYAN    AnsiCode = "\033[36;1m"
	ANSI_FG1_WHITE   AnsiCode = "\033[37;1m"
	ANSI_BG0_BLACK   AnsiCode = "\033[40m"
	ANSI_BG0_RED     AnsiCode = "\033[41m"
	ANSI_BG0_GREEN   AnsiCode = "\033[42m"
	ANSI_BG0_YELLOW  AnsiCode = "\033[43m"
	ANSI_BG0_BLUE    AnsiCode = "\033[44m"
	ANSI_BG0_MAGENTA AnsiCode = "\033[45m"
	ANSI_BG0_CYAN    AnsiCode = "\033[46m"
	ANSI_BG0_WHITE   AnsiCode = "\033[47m"
	ANSI_BG1_BLACK   AnsiCode = "\033[40;1m"
	ANSI_BG1_RED     AnsiCode = "\033[41;1m"
	ANSI_BG1_GREEN   AnsiCode = "\033[42;1m"
	ANSI_BG1_YELLOW  AnsiCode = "\033[43;1m"
	ANSI_BG1_BLUE    AnsiCode = "\033[44;1m"
	ANSI_BG1_MAGENTA AnsiCode = "\033[45;1m"
	ANSI_BG1_CYAN    AnsiCode = "\033[46;1m"
	ANSI_BG1_WHITE   AnsiCode = "\033[47;1m"

	ANSI_ERASE_END_LINE           AnsiCode = "\033[K"
	ANSI_ERASE_START_LINE         AnsiCode = "\033[1K"
	ANSI_ERASE_ALL_LINE           AnsiCode = "\033[2K"
	ANSI_ERASE_SCREEN_FROM_CURSOR AnsiCode = "\033[0J"
	ANSI_ERASE_SCREEN_TO_CURSOR   AnsiCode = "\033[1J"

	ANSI_CURSOR_UP        AnsiCode = "\033[A"
	ANSI_CURSOR_PREV_LINE AnsiCode = "\033[F"

	ANSI_BG_TRUECOLOR_FMT AnsiCode = "\033[48;2;%d;%d;%dm"
	ANSI_FG_TRUECOLOR_FMT AnsiCode = "\033[38;2;%d;%d;%dm"
)

var (
	ANSI_COLORS = [7]string{
		"black", "red", "green", "yellow", "blue", "magenta", "cyan",
	}

	ANSI_CODES = map[string]AnsiCode{
		"fg0_black":   ANSI_FG0_BLACK,
		"fg0_red":     ANSI_FG0_RED,
		"fg0_green":   ANSI_FG0_GREEN,
		"fg0_yellow":  ANSI_FG0_YELLOW,
		"fg0_blue":    ANSI_FG0_BLUE,
		"fg0_magenta": ANSI_FG0_MAGENTA,
		"fg0_cyan":    ANSI_FG0_CYAN,
		"fg0_white":   ANSI_FG0_WHITE,
		"fg1_black":   ANSI_FG1_BLACK,
		"fg1_red":     ANSI_FG1_RED,
		"fg1_green":   ANSI_FG1_GREEN,
		"fg1_yellow":  ANSI_FG1_YELLOW,
		"fg1_blue":    ANSI_FG1_BLUE,
		"fg1_magenta": ANSI_FG1_MAGENTA,
		"fg1_cyan":    ANSI_FG1_CYAN,
		"fg1_white":   ANSI_FG1_WHITE,
		"bg0_black":   ANSI_BG0_BLACK,
		"bg0_red":     ANSI_BG0_RED,
		"bg0_green":   ANSI_BG0_GREEN,
		"bg0_yellow":  ANSI_BG0_YELLOW,
		"bg0_blue":    ANSI_BG0_BLUE,
		"bg0_magenta": ANSI_BG0_MAGENTA,
		"bg0_cyan":    ANSI_BG0_CYAN,
		"bg0_white":   ANSI_BG0_WHITE,
		"bg1_black":   ANSI_BG1_BLACK,
		"bg1_red":     ANSI_BG1_RED,
		"bg1_green":   ANSI_BG1_GREEN,
		"bg1_yellow":  ANSI_BG1_YELLOW,
		"bg1_blue":    ANSI_BG1_BLUE,
		"bg1_magenta": ANSI_BG1_MAGENTA,
		"bg1_cyan":    ANSI_BG1_CYAN,
		"bg1_white":   ANSI_BG1_WHITE,
	}
)

func ansi_escaped_len(in string) int {
	if !enableAnsiColor {
		return len(in)
	}

	n := 0
	ignore := false
	for _, ch := range in {
		if ignore {
			ignore = (ch != 'm')
		} else {
			ignore = (ch == '\033')
			if !ignore && strconv.IsGraphic(ch) {
				n += 1
			}
		}
	}
	return n
}

func make_ansi_fg_truecolor(col ...uint8) string {
	return make_ansi_truecolor(ANSI_FG_TRUECOLOR_FMT, col[0], col[1], col[2])
}
func make_ansi_bg_truecolor(col ...uint8) string {
	return make_ansi_truecolor(ANSI_BG_TRUECOLOR_FMT, col[0], col[1], col[2])
}
func lerp_ansi_fg_truecolor(col0, col1 [3]uint8, f float64) string {
	return lerp_ansi_truecolor(ANSI_FG_TRUECOLOR_FMT, col0, col1, f)
}
func lerp_ansi_bg_truecolor(col0, col1 [3]uint8, f float64) string {
	return lerp_ansi_truecolor(ANSI_BG_TRUECOLOR_FMT, col0, col1, f)
}

func make_ansi_color(prefix string, color string) AnsiCode {
	if enableAnsiColor {
		return ANSI_CODES[prefix+"_"+color]
	} else {
		return AnsiCode("")
	}
}
func make_ansi_truecolor(fgOrbg AnsiCode, r, g, b uint8) string {
	if enableAnsiColor {
		return fmt.Sprintf(fgOrbg.String(), r, g, b)
	} else {
		return ""
	}
}
func lerp_color(col0, col1 [3]uint8, f float64) [3]uint8 {
	f = saturate(f)
	return [3]uint8{
		uint8(float64(col0[0])*(1.0-f) + float64(col1[0])*f),
		uint8(float64(col0[1])*(1.0-f) + float64(col1[1])*f),
		uint8(float64(col0[2])*(1.0-f) + float64(col1[2])*f)}
}
func lerp_ansi_truecolor(fgOrbg AnsiCode, col0, col1 [3]uint8, f float64) string {
	if enableAnsiColor {
		col := lerp_color(col0, col1, f)
		return make_ansi_truecolor(fgOrbg, col[0], col[1], col[2])
	} else {
		return ""
	}
}
func heatmap_truecolor(x float64) [3]uint8 {
	x = saturate(x)
	x1 := [4]float64{1, x, x * x, x * x * x}   // 1 x x2 x3
	x2 := [2]float64{x1[3] * x, x1[3] * x * x} // x4 x5
	dot := func(a, b []float64) (result float64) {
		for i, ai := range a {
			result += ai * b[i]
		}
		return
	}
	lin := [3]float64{
		dot(x1[:], []float64{-0.027780558, +1.228188385, +0.278906882, +3.892783760}) + dot(x2[:], []float64{-8.490712758, +4.069046086}),
		dot(x1[:], []float64{+0.014065206, +0.015360518, +1.605395918, -4.821108251}) + dot(x2[:], []float64{+8.389314011, -4.193858954}),
		dot(x1[:], []float64{-0.019628385, +3.122510347, -5.893222355, +2.798380308}) + dot(x2[:], []float64{-3.608884658, +4.324996022})}
	return [3]uint8{
		uint8(math.Floor(math.MaxUint8 * saturate(lin[0]))),
		uint8(math.Floor(math.MaxUint8 * saturate(lin[1]))),
		uint8(math.Floor(math.MaxUint8 * saturate(lin[2])))}
}
func smootherstep(x float64) float64 {
	x = saturate(x)
	return x * x * x * (x*(x*6.0-15.0) + 10.0)
}
func saturate(x float64) float64 {
	if x > 1 {
		return 1
	} else if x < 0 {
		return 0
	} else {
		return x
	}
}
func pastelizer_truecolor(f float64) [3]uint8 {
	_, h := math.Modf(f + 0.92620819117478)
	h = math.Abs(h) * 6.2831853071796
	cocg_x, cocg_y := 0.25*math.Cos(h), 0.25*math.Sin(h)
	br_x, br_y := -cocg_x-cocg_y, cocg_x-cocg_y
	c_x, c_y, c_z := 0.729+br_y, 0.729+cocg_y, 0.729+br_x
	return [3]uint8{
		uint8(math.Floor(math.MaxUint8 * saturate(c_x*c_x))),
		uint8(math.Floor(math.MaxUint8 * saturate(c_y*c_y))),
		uint8(math.Floor(math.MaxUint8 * saturate(c_z*c_z)))}
}
func expose_truecolor(c [3]uint8, f float64) [3]uint8 {
	brightness := math.Exp2((f*2.0 - 1.0) * 4.0)
	lin := [3]float64{
		float64(c[0]) * brightness,
		float64(c[1]) * brightness,
		float64(c[2]) * brightness}
	if lin[0] > math.MaxUint8 {
		lin[0] = float64(math.MaxUint8)
	}
	if lin[1] > math.MaxUint8 {
		lin[1] = float64(math.MaxUint8)
	}
	if lin[2] > math.MaxUint8 {
		lin[2] = float64(math.MaxUint8)
	}
	return [3]uint8{
		uint8(lin[0]),
		uint8(lin[1]),
		uint8(lin[2])}
}
