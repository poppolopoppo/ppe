package cluster

import (
	. "build/utils"
	"context"
)

var LogCluster = NewLogCategory("Cluster")

/***************************************
 * Cluster
 ***************************************/

type Cluster struct {
	PeerDiscovery
	ClusterOptions
}

func NewCluster(options ...ClusterOption) Cluster {
	settings := NewClusterOptions(options...)
	return Cluster{
		PeerDiscovery:  NewPeerDiscovery(settings.BrokeragePath),
		ClusterOptions: settings,
	}
}

func (x *Cluster) StartClient() (client *Client, cancel context.CancelFunc, err error) {
	client, err = NewClient(x)
	if err == nil {
		cancel, err = client.Start()
	}
	return
}
func (x *Cluster) StartWorker() (worker *Worker, cancel context.CancelFunc, err error) {
	worker, err = NewWorker(x)
	if err == nil {
		cancel, err = worker.Start()
	}
	return
}

/***************************************
 * Cluster options
 ***************************************/

type ClusterOptions struct {
	AppKey        []byte
	Context       context.Context
	BrokeragePath Directory
}

type ClusterOption = func(*ClusterOptions)

func NewClusterOptions(options ...ClusterOption) (result ClusterOptions) {
	flags := GetWorkerFlags()
	result.AppKey = flags.AppKey[:]
	result.BrokeragePath = flags.BrokeragePath
	result.Context = context.Background()
	for _, opt := range options {
		opt(&result)
	}
	return
}

func ClusterOptionContext(ctx context.Context) ClusterOption {
	return func(co *ClusterOptions) {
		co.Context = ctx
	}
}

func ClusterOptionBrokeragePath(path Directory) ClusterOption {
	return func(co *ClusterOptions) {
		co.BrokeragePath = path
	}
}
