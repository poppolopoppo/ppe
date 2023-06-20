package cluster

import (
	. "build/utils"
	"crypto/rand"
	"fmt"
	"io"
	"math/big"
	"os"
	"sync"
)

/***************************************
 * Peer discovery
 ***************************************/

type DiscoveredPeer struct {
	PublicKey PeerPublicKey
	PeerInfo
}

type PeerDiscovery struct {
	BrokeragePath Directory
	Peers         []DiscoveredPeer

	barrier sync.RWMutex
}

func NewPeerDiscovery(brokeragePath Directory) PeerDiscovery {
	LogVerbose(LogCluster, "new peer discovery with %q brokerage path", brokeragePath)
	return PeerDiscovery{
		BrokeragePath: brokeragePath,
		Peers:         []DiscoveredPeer{},
	}
}

func (x *PeerDiscovery) RandomPeer() (*DiscoveredPeer, bool) {
	x.barrier.RLock()
	defer x.barrier.RUnlock()

	if len(x.Peers) == 0 {
		return nil, false
	}

	index, err := rand.Int(rand.Reader, big.NewInt(int64(len(x.Peers))))
	LogPanicIfFailed(LogCluster, err)

	peer := &x.Peers[index.Int64()]
	LogVerbose(LogCluster, "selected random peer %v/%v", peer.FullyQualifiedAddress(), peer.PublicKey)
	return peer, true
}

func (x *PeerDiscovery) Announce(publicKey PeerPublicKey, peer *PeerInfo) error {
	x.barrier.Lock()
	defer x.barrier.Unlock()

	announceFile := x.BrokeragePath.File(publicKey.String())
	LogVerbose(LogCluster, "announce peer on brokerage %q", announceFile)
	UFS.Mkdir(announceFile.Dirname)
	return UFS.CreateFile(announceFile, func(f *os.File) error {
		return peer.Save(f)
	})
}

func (x *PeerDiscovery) Disapear(publicKey PeerPublicKey) error {
	x.barrier.Lock()
	defer x.barrier.Unlock()

	LogVerbose(LogCluster, "remove %v peer from brokerage %q", publicKey, x.BrokeragePath)
	return UFS.Remove(x.BrokeragePath.File(publicKey.String()))
}

func (x *PeerDiscovery) Discover() (int, error) {
	x.barrier.Lock()
	defer x.barrier.Unlock()

	x.Peers = x.Peers[:0]

	if !x.BrokeragePath.Exists() {
		return 0, fmt.Errorf("invalid brokerage path: %q", x.BrokeragePath)
	}

	for _, file := range x.BrokeragePath.Files() {
		var publicKey PeerPublicKey
		if err := publicKey.Set(file.Basename); err != nil {
			LogWarning(LogCluster, "ignore invalid peer filename: %q", file.Basename)
			continue
		}

		var peer PeerInfo
		if err := UFS.Open(file, func(r io.Reader) error {
			return peer.Load(r)
		}); err != nil {
			LogWarning(LogCluster, "ignore invalid peer info %q: %v", file, err)
			continue
		}

		x.Peers = append(x.Peers, DiscoveredPeer{
			PublicKey: publicKey,
			PeerInfo:  peer,
		})
	}

	LogVerbose(LogCluster, "discovered %d peers", len(x.Peers))
	return len(x.Peers), nil
}
