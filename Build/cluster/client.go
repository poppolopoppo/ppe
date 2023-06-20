package cluster

import (
	. "build/utils"
	"context"
	"sync"
	"sync/atomic"
	"time"
)

/***************************************
 * Client
 ***************************************/

type RemoteWorker struct {
	Available atomic.Bool
	*Tunnel
	Inbox chan MessageBody
}

type Client struct {
	Cluster *Cluster
	Workers map[PeerPublicKey]*RemoteWorker

	LocalPeer

	barrier    sync.Mutex
	cancel     context.CancelFunc
	context    context.Context
	numWorkers atomic.Int32
}

func NewClient(cluster *Cluster) (*Client, error) {
	localPeer, err := NewLocalPeer()
	if err != nil {
		return nil, err
	}
	return &Client{
		Cluster:   cluster,
		Workers:   make(map[PeerPublicKey]*RemoteWorker),
		LocalPeer: localPeer,
	}, nil
}

func (x *Client) Start() (context.CancelFunc, error) {
	x.context, x.cancel = context.WithCancel(x.Cluster.Context)

	flags := GetWorkerFlags()
	discoverTicker := time.NewTicker(time.Duration(flags.Broadcast) * time.Second)

	if err := x.DiscoverWorkers(flags); err != nil {
		x.cancel()
		return nil, err
	}

	go func() {
		defer x.cancel()

		select {
		case <-x.context.Done():
			x.Close()
			return

		case <-discoverTicker.C:
			if err := x.DiscoverWorkers(flags); err != nil {
				return
			}
		}
	}()

	return x.cancel, nil
}

func (x *Client) DispatchTask(executable Filename, arguments StringSet, options ProcessOptions) Future[int] {
	worker, ok := x.findAvailableWorker()
	if !ok {
		return MakeFutureError[int](RemoteTaskTimeout{})
	}

	return MakeFuture[int](func() (int, error) {
		defer func() {
			worker.Tunnel.OnTaskStart.Clear()
			worker.Tunnel.OnTaskFileAccess.Clear()
			worker.Tunnel.OnTaskOutput.Clear()
			worker.Tunnel.OnTaskStop.Clear()
			worker.Available.Store(true)
		}()

		chanErr := AnyChannels.Allocate()
		defer AnyChannels.Release(chanErr)

		worker.Tunnel.OnTaskStart.Add(func(mts *MessageTaskStart) error {
			return nil
		})
		worker.Tunnel.OnTaskStop.Add(func(mts *MessageTaskStop) error {
			chanErr <- mts.Err()
			return nil
		})

		if options.OnOutput.Bound() {
			worker.Tunnel.OnTaskOutput.Add(func(mto *MessageTaskOutput) error {
				return options.OnOutput.Invoke(mto.Output)
			})
		}

		if options.OnFileAccess.Bound() {
			worker.Tunnel.OnTaskFileAccess.Add(func(mtfa *MessageTaskFileAccess) error {
				return options.OnFileAccess.Invoke(mtfa.Record)
			})
		}

		worker.Inbox <- NewMessageTaskDispatch(executable, arguments, options.WorkingDir, options.Environment)

		timeoutTicker := time.NewTicker(worker.timeout)
		defer timeoutTicker.Stop()

		for {
			select {
			case <-timeoutTicker.C:
				if time.Now().UTC().Sub(worker.lastSeen) > worker.timeout {
					worker.Inbox <- NewMessagePing()
				}

			case er := <-chanErr:
				if IsNil(er) {
					return 0, nil
				} else {
					return -1, er.(error)
				}
			}
		}
	})
}

func (x *Client) Close() (err error) {
	x.cancel()

	x.barrier.Lock()
	defer x.barrier.Lock()

	for _, peer := range x.Workers {
		if er := peer.Close(); er != nil {
			err = er
		}
	}
	return err
}

func (x *Client) DiscoverWorkers(flags *WorkerFlags) error {
	n, err := x.Cluster.Discover()
	if err != nil {
		LogError(LogCluster, "peer discovery failed with: %v", err)
		return err
	}

	x.barrier.Lock()
	defer x.barrier.Lock()

	if n > flags.MaxPeers.Get() {
		n = flags.MaxPeers.Get()
	}

	for i := 0; i < n && len(x.Workers) < n; i += 1 {
		peer, ok := x.Cluster.RandomPeer()
		if !ok {
			break
		}
		if _, ok = x.Workers[peer.PublicKey]; ok {
			continue
		}
		if _, err := x.connectToWorker_AssumeLocked(peer); err != nil {
			continue
		}
	}

	return nil
}

func (x *Client) findAvailableWorker() (*RemoteWorker, bool) {
	x.barrier.Lock()
	defer x.barrier.Unlock()

	for publicKey, worker := range x.Workers {
		if worker.Available.CompareAndSwap(true, false) {
			LogInfo(LogCluster, "dispatch task to remove worker %v/%v", worker.conn.RemoteAddr(), publicKey)
			return worker, true
		}
	}

	return nil, false
}

func (x *Client) connectToWorker_AssumeLocked(peer *DiscoveredPeer) (*RemoteWorker, error) {
	if remotePeer, ok := x.Workers[peer.PublicKey]; ok {
		return remotePeer, nil
	}

	LogInfo(LogCluster, "dialing remote worker %s/%v", peer.FullyQualifiedAddress(), peer.PublicKey)

	timeout := time.Duration(GetWorkerFlags().Timeout.Get()) * time.Second
	tunnel, err := NewDialTunnel(x.Cluster, &x.LocalPeer, peer.PublicKey, timeout)
	if err != nil {
		return nil, err
	}

	LogVerbose(LogCluster, "connected to remove worker %s/%v:\n%v", peer.FullyQualifiedAddress(), peer.PublicKey, peer.Hardware)

	remotePeer := &RemoteWorker{
		Tunnel: tunnel,
		Inbox:  make(chan MessageBody, 1),
	}

	x.numWorkers.Add(1)
	x.Workers[peer.PublicKey] = remotePeer

	go func() {
		defer func() {
			remotePeer.Available.Store(false)

			x.numWorkers.Add(-1)
			remotePeer.Tunnel.Close()
			close(remotePeer.Inbox)

			x.barrier.Lock()
			defer x.barrier.Unlock()
			delete(x.Workers, peer.PublicKey)
		}()

		remotePeer.Available.Store(true)
		MessageLoop(remotePeer.Tunnel, x.context, timeout, remotePeer.Inbox)
	}()

	return remotePeer, nil
}
