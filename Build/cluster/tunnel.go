package cluster

import (
	. "build/utils"

	"net"
	"time"

	"go.cryptoscope.co/secretstream/boxstream"
	"go.cryptoscope.co/secretstream/secrethandshake"
)

/***************************************
 * Tunnel
 ***************************************/

type Tunnel struct {
	conn     net.Conn
	ping     time.Duration
	timeout  time.Duration
	lastSeen time.Time

	state   *secrethandshake.State
	boxer   *boxstream.Boxer
	unboxer *boxstream.Unboxer

	OnError          PublicEvent[error]
	OnTaskStart      PublicEvent[*MessageTaskStart]
	OnTaskStop       PublicEvent[*MessageTaskStop]
	OnTaskFileAccess PublicEvent[*MessageTaskFileAccess]
	OnTaskOutput     PublicEvent[*MessageTaskOutput]
}

func NewDialTunnel(cluster *Cluster, client *LocalPeer, remotePublic PeerPublicKey, timeout time.Duration) (tunnel *Tunnel, err error) {
	dialer, err := net.Dial("tcp", client.FullyQualifiedAddress())
	if err != nil {
		return nil, err
	}

	state, err := secrethandshake.NewClientState(cluster.AppKey, client.EdKeyPair(), remotePublic[:])
	if err == nil {
		if err = secrethandshake.Client(state, dialer); err == nil {
			return newTunnel(dialer, state, timeout), nil
		}
	}
	return nil, err
}
func NewListenTunnel(cluster *Cluster, worker *Worker, conn net.Conn, timeout time.Duration) (*Tunnel, error) {
	state, err := secrethandshake.NewServerState(cluster.AppKey, worker.EdKeyPair())
	if err == nil {
		if err = secrethandshake.Server(state, conn); err == nil {
			return newTunnel(conn, state, timeout), nil
		}
	}
	return nil, err
}

func newTunnel(conn net.Conn, state *secrethandshake.State, timeout time.Duration) *Tunnel {
	tunnel := &Tunnel{
		conn:     conn,
		state:    state,
		lastSeen: time.Now(),
		ping:     timeout,
		timeout:  timeout,
	}

	enKey, enNonce := state.GetBoxstreamEncKeys()
	tunnel.boxer = boxstream.NewBoxer(conn, &enNonce, &enKey)

	deKey, deNonce := state.GetBoxstreamDecKeys()
	tunnel.unboxer = boxstream.NewUnboxer(conn, &deNonce, &deKey)

	return tunnel
}
func (x *Tunnel) ReadMessage() ([]byte, error) {
	if err := x.conn.SetReadDeadline(time.Now().Add(x.timeout)); err != nil {
		return nil, err
	}
	return x.unboxer.ReadMessage()
}
func (x *Tunnel) WriteMessage(msg []byte) error {
	if err := x.conn.SetWriteDeadline(time.Now().Add(x.timeout)); err != nil {
		return err
	}
	return x.boxer.WriteMessage(msg)
}
func (x *Tunnel) WriteGoodbye() error {
	if err := x.conn.SetWriteDeadline(time.Now().Add(x.timeout)); err != nil {
		return err
	}
	return x.boxer.WriteGoodbye()
}
func (x *Tunnel) Close() error {
	errA := x.boxer.WriteGoodbye()
	errB := x.conn.Close()

	x.conn = nil
	x.state = nil
	x.boxer = nil
	x.unboxer = nil

	var err error
	if errB != nil {
		err = errB
	} else {
		err = errA
	}

	x.OnError.Invoke(err)
	return err
}
