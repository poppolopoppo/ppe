package cluster

import (
	. "build/utils"
	"bytes"
	"context"
	"fmt"
	"net"
	"time"
)

/***************************************
 * Message Loop
 ***************************************/

func MessageLoop(tunnel *Tunnel, ctx context.Context, timeout time.Duration, inbox <-chan MessageBody) error {
	buf := bytes.Reader{}
	ar := NewArchiveBinaryReader(&buf)
	defer ar.Close()

	wr := NewMessageWriter(tunnel)
	defer wr.Close()

	pingTicker := time.NewTicker(timeout)
	defer pingTicker.Stop()

	var err error
	defer func() {
		if err != nil {
			LogError(LogCluster, "tunnel %v failed with: %v", tunnel.conn.RemoteAddr(), err)
		}
	}()
	for {
		var raw []byte
		select {
		case <-ctx.Done():
			return ctx.Err()

		case msg := <-inbox:
			if err = wr.Write(msg); err != nil {
				return err
			}
			continue

		case <-pingTicker.C:
			if err = wr.Write(NewMessagePing()); err != nil {
				return err
			}
			continue

		default:
			if raw, err = tunnel.ReadMessage(); err != nil {
				return err
			}
		}

		buf.Reset(raw)

		var msg MessageType
		ar.Serializable(&msg)
		if err = ar.Error(); err != nil {
			break
		}

		if err = msg.Body(func(body MessageBody) (err error) {
			ar.Serializable(body)
			if err = ar.Error(); err == nil {
				LogTrace(LogCluster, "%v: rean %v", tunnel.conn.RemoteAddr(), PrettyPrint(body))
				err = body.Accept(&wr)
			}
			return
		}); err != nil {
			break
		}
	}

	return err
}

/***************************************
 * Message
 ***************************************/

type MessageType int32

const (
	MSG_PING MessageType = iota
	MSG_PONG
	MSG_TASK_DISPATCH
	MSG_TASK_START
	MSG_TASK_FILEACCESS
	MSG_TASK_OUTPUT
	MSG_TASK_STOP
)

var MessageTypes = []MessageType{
	MSG_PING,
	MSG_PONG,
	MSG_TASK_DISPATCH,
	MSG_TASK_START,
	MSG_TASK_FILEACCESS,
	MSG_TASK_OUTPUT,
	MSG_TASK_STOP,
}

func (x MessageType) Body(body func(MessageBody) error) error {
	switch x {
	case MSG_PING:
		return body(&MessagePing{})
	case MSG_PONG:
		return body(&MessagePong{})
	case MSG_TASK_DISPATCH:
		return body(&MessageTaskDispatch{})
	case MSG_TASK_START:
		return body(&MessageTaskStart{})
	case MSG_TASK_FILEACCESS:
		return body(&MessageTaskFileAccess{})
	case MSG_TASK_OUTPUT:
		return body(&MessageTaskOutput{})
	case MSG_TASK_STOP:
		return body(&MessageTaskStop{})
	default:
		return MakeUnexpectedValueError(x, x)
	}
}
func (x MessageType) String() string {
	switch x {
	case MSG_PING:
		return "PING"
	case MSG_PONG:
		return "PONG"
	case MSG_TASK_DISPATCH:
		return "TASK_DISPATCH"
	case MSG_TASK_START:
		return "TASK_START"
	case MSG_TASK_FILEACCESS:
		return "TASK_FILEACCESS"
	case MSG_TASK_OUTPUT:
		return "TASK_PROGRESS"
	case MSG_TASK_STOP:
		return "TASK_STOP"
	default:
		UnexpectedValuePanic(x, x)
		return ""
	}
}
func (x *MessageType) Serialize(ar Archive) {
	ar.Int32((*int32)(x))
}

/***************************************
 * Message Writer
 ***************************************/

type MessageWriter struct {
	tunnel *Tunnel
	buf    *bytes.Buffer
	ar     ArchiveBinaryWriter
}

func NewMessageWriter(tunnel *Tunnel) (result MessageWriter) {
	result.tunnel = tunnel
	result.buf = TransientBuffer.Allocate()
	result.ar = NewArchiveBinaryWriter(result.buf)
	return
}
func (x *MessageWriter) UpdateLatency(remoteUTC time.Time) {
	x.tunnel.lastSeen = time.Now().UTC()
	x.tunnel.ping = x.tunnel.lastSeen.Sub(remoteUTC)
}
func (x *MessageWriter) Write(body MessageBody) error {
	LogTrace(LogCluster, "%v: write %v", x.tunnel.conn.RemoteAddr(), PrettyPrint(body))
	defer x.buf.Reset()

	header := body.Header()
	x.ar.Serializable(&header)
	x.ar.Serializable(body)

	if err := x.ar.Error(); err == nil {
		return x.tunnel.WriteMessage(x.buf.Bytes())
	} else {
		return err
	}
}
func (x *MessageWriter) Close() error {
	TransientBuffer.Release(x.buf)
	x.buf = nil
	x.tunnel = nil
	return nil
}

/***************************************
 * Message Body
 ***************************************/

type MessageBody interface {
	Header() MessageType
	Accept(*MessageWriter) error
	Serializable
}

type timedMessageBody struct {
	Timestamp time.Time
}

func newTimedMessageBody() timedMessageBody {
	return timedMessageBody{
		Timestamp: time.Now().UTC(),
	}
}
func (x *timedMessageBody) Accept(wr *MessageWriter) error {
	wr.UpdateLatency(x.Timestamp)
	return nil
}
func (x *timedMessageBody) Serialize(ar Archive) {
	ar.Time(&x.Timestamp)
}

/***************************************
 * Message Ping/Pong
 ***************************************/

type MessagePing struct {
	timedMessageBody
}

func NewMessagePing() *MessagePing {
	return &MessagePing{newTimedMessageBody()}
}
func (x *MessagePing) Header() MessageType { return MSG_PING }
func (x *MessagePing) Accept(wr *MessageWriter) error {
	if err := x.timedMessageBody.Accept(wr); err != nil {
		return err
	}
	return wr.Write(NewMessagePong())
}

type MessagePong struct {
	timedMessageBody
}

func NewMessagePong() *MessagePong {
	return &MessagePong{newTimedMessageBody()}
}
func (x *MessagePong) Header() MessageType { return MSG_PONG }

/***************************************
 * Message Task
 ***************************************/

type RemoteTaskTimeout struct {
}

func (x RemoteTaskTimeout) Error() string {
	return "remote task timeout"
}

type RemoteTaskError struct {
	Remote  net.Addr
	Message string
}

func (x RemoteTaskError) Error() string {
	return fmt.Sprintf("remote task failed with: %s (%v)", x.Message, x.Remote)
}

type MessageTaskDispatch struct {
	Executable  Filename
	Arguments   StringSet
	Environment ProcessEnvironment
	WorkingDir  Directory
	timedMessageBody
}

func NewMessageTaskDispatch(executable Filename, arguments StringSet, workingDir Directory, env ProcessEnvironment) *MessageTaskDispatch {
	return &MessageTaskDispatch{
		Executable:       executable,
		Arguments:        arguments,
		WorkingDir:       workingDir,
		Environment:      env,
		timedMessageBody: newTimedMessageBody(),
	}
}
func (x *MessageTaskDispatch) Header() MessageType { return MSG_TASK_DISPATCH }
func (x *MessageTaskDispatch) Accept(wr *MessageWriter) error {
	if err := x.timedMessageBody.Accept(wr); err != nil {
		return err
	}
	if err := wr.Write(NewMessageTaskStart()); err != nil {
		return err
	}

	err := RunProcess(x.Executable, x.Arguments,
		OptionProcessCaptureOutput,
		OptionProcessEnvironment(x.Environment),
		OptionProcessWorkingDir(x.WorkingDir),
		OptionProcessFileAccess(func(far FileAccessRecord) error {
			return wr.Write(NewMessageTaskFileAccess(far))
		}),
		OptionProcessOutput(func(s string) error {
			return wr.Write(NewMessageTaskOutput(s))
		}))

	return wr.Write(NewMessageTaskStop(err))
}
func (x *MessageTaskDispatch) Serialize(ar Archive) {
	x.timedMessageBody.Serialize(ar)
	ar.Serializable(&x.Environment)
	ar.Serializable(&x.Executable)
	ar.Serializable(&x.Arguments)
	ar.Serializable(&x.WorkingDir)
}

type MessageTaskStart struct {
	timedMessageBody
}

func NewMessageTaskStart() *MessageTaskStart {
	return &MessageTaskStart{newTimedMessageBody()}
}
func (x *MessageTaskStart) Header() MessageType { return MSG_TASK_START }
func (x *MessageTaskStart) Accept(wr *MessageWriter) (err error) {
	if err = x.timedMessageBody.Accept(wr); err == nil {
		err = wr.tunnel.OnTaskStart.Invoke(x)
	}
	return
}

type MessageTaskFileAccess struct {
	timedMessageBody
	Record FileAccessRecord
}

func NewMessageTaskFileAccess(far FileAccessRecord) *MessageTaskFileAccess {
	return &MessageTaskFileAccess{
		Record:           far,
		timedMessageBody: newTimedMessageBody(),
	}
}
func (x *MessageTaskFileAccess) Header() MessageType { return MSG_TASK_OUTPUT }
func (x *MessageTaskFileAccess) Accept(wr *MessageWriter) (err error) {
	if err = x.timedMessageBody.Accept(wr); err == nil {
		err = wr.tunnel.OnTaskFileAccess.Invoke(x)
	}
	return
}
func (x *MessageTaskFileAccess) Serialize(ar Archive) {
	x.timedMessageBody.Serialize(ar)
	ar.Serializable(&x.Record)
}

type MessageTaskOutput struct {
	timedMessageBody
	Output string
}

func NewMessageTaskOutput(output string) *MessageTaskOutput {
	return &MessageTaskOutput{
		Output:           output,
		timedMessageBody: newTimedMessageBody(),
	}
}
func (x *MessageTaskOutput) Header() MessageType { return MSG_TASK_OUTPUT }
func (x *MessageTaskOutput) Accept(wr *MessageWriter) (err error) {
	if err = x.timedMessageBody.Accept(wr); err == nil {
		err = wr.tunnel.OnTaskOutput.Invoke(x)
	}
	return
}
func (x *MessageTaskOutput) Serialize(ar Archive) {
	x.timedMessageBody.Serialize(ar)
	ar.String(&x.Output)
}

type MessageTaskStop struct {
	Error string
	timedMessageBody
}

func NewMessageTaskStop(err error) *MessageTaskStop {
	var errMsg string
	if err != nil {
		errMsg = err.Error()
	}
	return &MessageTaskStop{
		Error:            errMsg,
		timedMessageBody: newTimedMessageBody(),
	}
}
func (x *MessageTaskStop) Header() MessageType { return MSG_TASK_STOP }
func (x *MessageTaskStop) Accept(wr *MessageWriter) (err error) {
	if err = x.timedMessageBody.Accept(wr); err == nil {
		err = wr.tunnel.OnTaskStop.Invoke(x)
	}
	return
}
func (x *MessageTaskStop) Err() error {
	if len(x.Error) > 0 {
		return RemoteTaskError{
			Message: x.Error,
		}
	}
	return nil
}
func (x *MessageTaskStop) Serialize(ar Archive) {
	x.timedMessageBody.Serialize(ar)
	ar.String(&x.Error)
}
