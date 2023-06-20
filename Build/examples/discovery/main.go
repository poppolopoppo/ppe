package main

import (
	"cluster/cluster"
	"context"
	"encoding/hex"
	"log"
	"os"
	"strings"
	"time"
)

const APPKEY_HEX = "b59fccefaaf6f1ac55d98e2d9fe13a5e2bcbf2563b901a393bd3d5fb55260ba3"

func main() {
	const port = 43873

	worker := false
	for _, a := range os.Args[1:] {
		if strings.ToLower(a) == "worker" {
			worker = true
		}
	}

	var appKey [32]byte
	hex.Decode(appKey[:], cluster.UnsafeBytesFromString(APPKEY_HEX))
	log.Printf("app key: %q", hex.EncodeToString(appKey[:]))

	ctx, cancel_peer_connection := context.WithCancel(context.Background())

	peer, err := cluster.NewPeerClient(appKey[:],
		cluster.PeerAllowSelf(worker),
		cluster.PeerBroadcast(!worker),
		cluster.PeerBroadcastDelay(3*time.Second),
		cluster.PeerContext(ctx),
		cluster.PeerPort(port),
		cluster.PeerTimeout(10*time.Second),
		cluster.PeerNotifyNewPeer(func(pr *cluster.PeerRemote) {
			log.Printf("NEW PEER: %v: -> %v (invited:%t)", pr.Addr(), pr.PublicKey(), pr.WasInvited())
		}),
		cluster.PeerNotifyPeerLost(func(pr *cluster.PeerRemote) {
			log.Printf("PEER LOST: %v: -> %v (%v)", pr.Addr(), pr.PublicKey(), time.Since(pr.TimeLastSeen()))
		}))

	if err != nil {
		log.Fatalln(err)
	}
	defer peer.Close()

	expireTime := time.Now().Add(10 * time.Minute)
	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()
	for {
		if time.Now().After(expireTime) {
			log.Println("client timeout")
			break
		}

		<-ticker.C
	}

	cancel_peer_connection()
}
