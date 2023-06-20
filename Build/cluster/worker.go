package cluster

import (
	. "build/utils"
	"context"
	"crypto/rand"
	"fmt"
	"net"
	"sync"
	"time"

	"go.cryptoscope.co/secretstream/secrethandshake"
)

/***************************************
 * Worker Flags
 ***************************************/

var APPKEY_DEFAULT PeerSecretKey = PeerSecretKey{
	0x2d, 0x3f, 0x8c, 0xe5, 0x57, 0xe6, 0x76, 0x9b, 0x1d, 0x7b, 0x15, 0xb0, 0xb7, 0xe0, 0x38, 0x2a,
	0x9f, 0x60, 0xc1, 0xd9, 0x91, 0x17, 0x9e, 0x48, 0x71, 0x93, 0x96, 0x62, 0x27, 0xe4, 0x4c, 0xc6,
	0x2d, 0x9e, 0x6a, 0x11, 0x1c, 0x1d, 0x60, 0x17, 0xd0, 0x55, 0x82, 0xa9, 0x02, 0x4d, 0x42, 0xb1,
	0xb6, 0x64, 0x26, 0x4d, 0x41, 0xe0, 0x84, 0x53, 0xc4, 0x5e, 0x51, 0xd5, 0xb9, 0x20, 0x67, 0xf4,
}

type WorkerFlags struct {
	AppKey        PeerSecretKey
	BrokeragePath Directory
	Broadcast     IntVar
	Timeout       IntVar
	MaxPeers      IntVar
	PublicKey     PeerPublicKey
	SecretKey     PeerSecretKey
}

var GetWorkerFlags = NewCommandParsableFlags(func() *WorkerFlags {
	secret, err := secrethandshake.GenEdKeyPair(nil)
	LogPanicIfFailed(LogCluster, err)

	result := WorkerFlags{}
	result.Broadcast = 2
	result.Timeout = 3
	result.MaxPeers = 32
	result.BrokeragePath = UFS.Transient.Folder("Brokerage")

	copy(result.PublicKey[:], secret.Public)
	copy(result.SecretKey[:], secret.Secret)

	_, err = rand.Read(result.AppKey[:])
	LogPanicIfFailed(LogCluster, err)
	return &result
}())

func (x *WorkerFlags) Flags(cfv CommandFlagsVisitor) {
	cfv.Persistent("AppKey", "set peer cluster application key", &x.AppKey)
	cfv.Persistent("BrokeragePath", "set peer discovery brokerage path", &x.BrokeragePath)
	cfv.Persistent("Broadcast", "set worker broadcast delay in seconds", &x.Broadcast)
	cfv.Persistent("Timeout", "set peer tunnel timeout in seconds", &x.Timeout)
	cfv.Persistent("MaxPeers", "set maximum number of connected peers allowed", &x.Timeout)
	cfv.Persistent("PublicKey", "set peer public key used by worker handshake", &x.PublicKey)
	cfv.Persistent("SecretKey", "set peer secret key used by worker handshake", &x.SecretKey)
}

/***************************************
 * Worker
 ***************************************/

type Worker struct {
	Cluster *Cluster
	await   Future[int]

	LocalPeer
}

func NewWorker(cluster *Cluster) (*Worker, error) {
	localPeer, err := NewLocalPeer()
	if err != nil {
		return nil, err
	}
	return &Worker{
		Cluster:   cluster,
		LocalPeer: localPeer,
	}, nil
}

func (x *Worker) Start() (context.CancelFunc, error) {
	LogClaim(LogCluster, "start worker, listening on TCP:%v", x.Port)
	listener, err := net.Listen("tcp", fmt.Sprintf(":%v", x.Port))
	if err != nil {
		return nil, err
	}

	if err = x.Cluster.Announce(x.PublicKey, &x.PeerInfo); err != nil {
		return nil, err
	}

	flags := GetWorkerFlags()
	broadcastTicker := time.NewTicker(time.Duration(flags.Broadcast) * time.Second)
	defer broadcastTicker.Stop()

	ctx, cancel := context.WithCancel(x.Cluster.Context)

	x.await = MakeFuture[int](func() (int, error) {
		defer cancel()
		defer listener.Close()

		wg := sync.WaitGroup{}
		defer wg.Wait()

		defer x.Cluster.Disapear(x.PublicKey)

		for {
			select {
			case <-ctx.Done():
				return -1, ctx.Err()
			case <-broadcastTicker.C:
				if err := x.Cluster.Announce(x.PublicKey, &x.PeerInfo); err != nil {
					LogError(LogCluster, "failed to announce worker: %v", err)
					return -2, err
				}
			default:
				if conn, err := listener.Accept(); err == nil {
					wg.Add(1)
					go func() {
						defer wg.Done()
						if err := x.Accept(ctx, conn); err != nil {
							LogError(LogCluster, "worker handshake with %v failed: %v", conn.RemoteAddr(), err)
						}
					}()
				} else {
					LogWarning(LogCluster, "TCP accept %v failed: %v", conn.RemoteAddr(), err)
				}
			}
		}
	})

	return cancel, nil
}
func (x *Worker) Close() error {
	return x.await.Join().Failure()
}

func (x *Worker) Accept(ctx context.Context, conn net.Conn) error {
	timeout := time.Second * time.Duration(GetWorkerFlags().Timeout.Get())
	tunnel, err := NewListenTunnel(x.Cluster, x, conn, timeout)
	if err != nil {
		return err
	}
	defer tunnel.Close()
	LogInfo(LogCluster, "worker accept connection from %v", conn.RemoteAddr())
	return MessageLoop(tunnel, ctx, timeout, make(<-chan MessageBody, 0))
}
