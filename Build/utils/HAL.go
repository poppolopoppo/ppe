package utils

import (
	"bytes"
	"fmt"
	"os"
	"os/signal"
	"strings"
	"syscall"
)

type HostId string

const (
	HOST_WINDOWS HostId = "WINDOWS"
	HOST_LINUX   HostId = "LINUX"
	HOST_DARWIN  HostId = "DARWIN"
)

func (id HostId) String() string {
	return string(id)
}
func (x *HostId) Set(in string) error {
	switch strings.ToUpper(in) {
	case HOST_WINDOWS.String():
		*x = HOST_WINDOWS
	case HOST_LINUX.String():
		*x = HOST_LINUX
	case HOST_DARWIN.String():
		*x = HOST_DARWIN
	default:
		UnexpectedValue(in)
	}
	return nil
}
func (id HostId) GetDigestable(o *bytes.Buffer) {
	o.WriteString(id.String())
}

type HostPlatform struct {
	Id   HostId
	Name string
}

func (x HostPlatform) GetDigestable(o *bytes.Buffer) {
	MakeDigestable[Digestable](o, x.Id, RawBytes(x.Name))
}
func (x HostPlatform) String() string {
	return fmt.Sprint(x.Id, x.Name)
}

var currentHost *HostPlatform

func CurrentHost() *HostPlatform {
	return currentHost
}
func SetCurrentHost(host *HostPlatform) {
	currentHost = host
}

func IfWindows(block func()) {
	if CurrentHost().Id == HOST_WINDOWS {
		block()
	}
}
func IfLinux(block func()) {
	if CurrentHost().Id == HOST_LINUX {
		block()
	}
}
func IfDarwin(block func()) {
	if CurrentHost().Id == HOST_DARWIN {
		block()
	}
}

// setupCloseHandler creates a 'listener' on a new goroutine which will notify the
// program if it receives an interrupt from the OS. We then handle this by calling
// our clean up procedure and exiting the program.
func setupCloseHandler() {
	c := make(chan os.Signal)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-c
		LogWarning("\r- Ctrl+C pressed in Terminal")
		PurgePinnedLogs()
		os.Exit(0)
	}()
}