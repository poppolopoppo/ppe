package utils

import (
	"bufio"
	"io"
	"os/exec"
	"strings"
	"time"
)

/***************************************
 * Process Options
 ***************************************/

type ProcessOptions struct {
	Environment   ProcessEnvironment
	WorkingDir    Directory
	CaptureOutput bool
	NoSpinner     bool
}

type ProcessOptionFunc func(*ProcessOptions)

func (x *ProcessOptions) Init(options ...ProcessOptionFunc) {
	for _, it := range options {
		it(x)
	}
}

func OptionProcessEnvironment(environment ProcessEnvironment) ProcessOptionFunc {
	return func(po *ProcessOptions) {
		po.Environment.Overwrite(environment)
	}
}
func OptionProcessExport(name string, values ...string) ProcessOptionFunc {
	return func(po *ProcessOptions) {
		po.Environment.Append(name, values...)
	}
}
func OptionProcessWorkingDir(value Directory) ProcessOptionFunc {
	return func(po *ProcessOptions) {
		po.WorkingDir = value
	}
}
func OptionProcessCaptureOutput(po *ProcessOptions) {
	po.CaptureOutput = true
}
func OptionProcessCaptureOutputIf(enabled bool) ProcessOptionFunc {
	return func(po *ProcessOptions) {
		po.CaptureOutput = enabled
	}
}
func OptionProcessNoSpinner(po *ProcessOptions) {
	po.NoSpinner = true
}
func OptionProcessNoSpinnerIf(enabled bool) ProcessOptionFunc {
	return func(po *ProcessOptions) {
		po.NoSpinner = enabled
	}
}

/***************************************
 * RunProcess
 ***************************************/

func RunProcess(executable Filename, arguments StringSet, userOptions ...ProcessOptionFunc) error {
	options := ProcessOptions{
		Environment: NewProcessEnvironment(),
	}
	options.Init(userOptions...)

	var pbar PinnedProgress
	if !options.NoSpinner {
		pbar = LogSpinner(strings.Join(arguments, " "))
		defer pbar.Close()
	}

	cmd := exec.Command(executable.String(), arguments...)
	LogTrace("run process: %v", cmd)

	defer cmd.Wait()

	if len(options.WorkingDir.Path) > 0 {
		cmd.Dir = options.WorkingDir.String()
	}

	cmd.Env = append(cmd.Env, options.Environment.Export()...)

	if options.CaptureOutput {
		var err error
		var stdout io.ReadCloser
		if stdout, err = cmd.StdoutPipe(); err != nil {
			return err
		}
		defer stdout.Close()

		cmd.Stderr = cmd.Stdout

		if err = cmd.Start(); err != nil {
			return err
		}

		const capacity = LARGE_PAGE_CAPACITY / 2
		buf := TransientLargePage.Allocate()
		defer TransientLargePage.Release(buf)

		scanner := bufio.NewScanner(stdout)
		scanner.Buffer(buf, capacity)

		done := make(chan struct{})
		go func() {
			defer close(done)

			for scanner.Scan() {
				line := scanner.Text()
				line = strings.TrimRight(line, "\r\n")
				line = strings.TrimSpace(line)
				if len(line) > 0 {
					LogForward(line)
				}
			}

			done <- struct{}{}
		}()

		for {
			select {
			case <-done:
				return cmd.Wait()
			case <-time.After(200 * time.Millisecond):
				if pbar != nil {
					pbar.Inc()
				}
			}
		}

	} else {
		outputForError := TransientBuffer.Allocate()
		defer TransientBuffer.Release(outputForError)

		cmd.Stderr = outputForError
		cmd.Stdout = outputForError
		if err := cmd.Run(); err != nil {
			// print output if the command failed
			LogForward(outputForError.String())
			return err
		}

		return nil
	}
}

/***************************************
 * Process Environment
 ***************************************/

type EnvironmentVar string

func (x EnvironmentVar) String() string { return (string)(x) }
func (x EnvironmentVar) Compare(other EnvironmentVar) int {
	return strings.Compare(x.String(), other.String())
}
func (x *EnvironmentVar) Serialize(ar Archive) {
	ar.String((*string)(x))
}

type EnvironmentDefinition struct {
	Name   EnvironmentVar
	Values StringSet
}

func (x EnvironmentDefinition) String() string {
	if len(x.Values) > 0 {
		sb := strings.Builder{}

		capacity := len(x.Name.String())
		for _, it := range x.Values {
			capacity += 1 + len(it)
		}
		sb.Grow(capacity)

		sb.WriteString(x.Name.String())
		sb.WriteRune('=')

		for i, it := range x.Values {
			if i > 0 {
				sb.WriteRune(';')
			}
			sb.WriteString(it)
		}

		return sb.String()
	} else {
		return x.Name.String()
	}
}
func (x *EnvironmentDefinition) Serialize(ar Archive) {
	ar.Serializable(&x.Name)
	ar.Serializable(&x.Values)
}

type ProcessEnvironment []EnvironmentDefinition

func NewProcessEnvironment() ProcessEnvironment {
	return ProcessEnvironment([]EnvironmentDefinition{})
}

func (x ProcessEnvironment) Export() []string {
	result := make([]string, len(x))
	for i, it := range x {
		result[i] = it.String()
	}
	return result
}
func (x ProcessEnvironment) IndexOf(name string) (int, bool) {
	for i, it := range x {
		if it.Name.String() == name {
			return i, true
		}
	}
	return len(x), false
}
func (x *ProcessEnvironment) Append(name string, values ...string) {
	if i, ok := x.IndexOf(name); ok {
		(*x)[i].Values.Append(values...)
	} else {
		*x = append(*x, EnvironmentDefinition{
			Name:   EnvironmentVar(name),
			Values: values,
		})
	}
}
func (x *ProcessEnvironment) Inherit(other ProcessEnvironment) {
	for _, it := range other {
		// #TODO: add precedence?
		x.Append(it.Name.String(), it.Values...)
	}
}
func (x *ProcessEnvironment) Overwrite(other ProcessEnvironment) {
	for _, it := range other {
		x.Append(it.Name.String(), it.Values...)
	}
}
func (x *ProcessEnvironment) Serialize(ar Archive) {
	SerializeSlice(ar, (*[]EnvironmentDefinition)(x))
}
