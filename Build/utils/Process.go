package utils

import (
	"bufio"
	"fmt"
	"io"
	"os/exec"
	"sort"
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

	if len(options.WorkingDir) > 0 {
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

		scanner := bufio.NewScanner(stdout)
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

type ProcessEnvironment map[EnvironmentVar]StringSet

func NewProcessEnvironment() ProcessEnvironment {
	return ProcessEnvironment(make(map[EnvironmentVar]StringSet))
}

func (x *ProcessEnvironment) Export() []string {
	result := make([]string, 0, len(*x))
	for name, values := range *x {
		if len(values) > 0 {
			result = append(result, fmt.Sprintf("%s=%s", name, values.Join(";")))
		} else {
			result = append(result, name.String())
		}
	}
	sort.Strings(result)
	return result
}
func (x *ProcessEnvironment) Append(name string, values ...string) {
	record := (*x)[EnvironmentVar(name)]
	record.Append(values...)
	(*x)[EnvironmentVar(name)] = record
}
func (x *ProcessEnvironment) Inherit(other ProcessEnvironment) {
	for name, values := range other {
		if present, ok := (*x)[name]; ok {
			(*x)[name] = append(present, values...)
		} else {
			(*x)[name] = values
		}
	}
}
func (x *ProcessEnvironment) Overwrite(other ProcessEnvironment) {
	for name, values := range other {
		if present, ok := (*x)[name]; ok {
			(*x)[name] = append(values, present...)
		} else {
			(*x)[name] = values
		}
	}
}
func (x *ProcessEnvironment) Serialize(ar Archive) {
	SerializeMap(ar, (*map[EnvironmentVar]StringSet)(x))
}
