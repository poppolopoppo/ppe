package cluster

import (
	. "build/utils"
	"crypto/ed25519"
	"encoding/base64"
	"fmt"
	"io"
	"net"
	"os"
	"runtime"
	"strings"

	"go.cryptoscope.co/secretstream/secrethandshake"
)

/***************************************
 * Peer version
 ***************************************/

type PeerVersion = string

const (
	PEERVERSION_1_0 PeerVersion = "1.0"
)

/***************************************
 * Peer mode
 ***************************************/

type PeerMode int32

const (
	PEERMODE_DISABLED PeerMode = iota
	PEERMODE_IDLE
	PEERMODE_DEDICATED
	PEERMODE_PROPORTIONAL
)

func PeerModes() []PeerMode {
	return []PeerMode{
		PEERMODE_DISABLED,
		PEERMODE_IDLE,
		PEERMODE_DEDICATED,
		PEERMODE_PROPORTIONAL,
	}
}
func (x PeerMode) Equals(o PeerMode) bool {
	return (x == o)
}
func (x PeerMode) String() string {
	switch x {
	case PEERMODE_DISABLED:
		return "DISABLED"
	case PEERMODE_IDLE:
		return "IDLE"
	case PEERMODE_DEDICATED:
		return "DEDICATED"
	case PEERMODE_PROPORTIONAL:
		return "PROPORTIONAL"
	default:
		UnexpectedValue(x)
		return ""
	}
}
func (x *PeerMode) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case PEERMODE_DISABLED.String():
		*x = PEERMODE_DISABLED
	case PEERMODE_IDLE.String():
		*x = PEERMODE_IDLE
	case PEERMODE_DEDICATED.String():
		*x = PEERMODE_DEDICATED
	case PEERMODE_PROPORTIONAL.String():
		*x = PEERMODE_PROPORTIONAL
	default:
		err = MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *PeerMode) Serialize(ar Archive) {
	ar.Int32((*int32)(x))
}
func (x PeerMode) MarshalText() ([]byte, error) {
	return UnsafeBytesFromString(x.String()), nil
}
func (x *PeerMode) UnmarshalText(data []byte) error {
	return x.Set(UnsafeStringFromBytes(data))
}
func (x *PeerMode) AutoComplete(in AutoComplete) {
	for _, it := range PeerModes() {
		in.Add(it.String())
	}
}

/***************************************
 * Peer flags
 ***************************************/

type PeerFlags struct {
	Mode          PeerMode
	MaxThreads    IntVar
	MinFreeMemory IntVar
	Port          IntVar
}

var GetPeerFlags = NewCommandParsableFlags(&PeerFlags{
	Mode:          PEERMODE_PROPORTIONAL,
	MaxThreads:    InheritableInt(runtime.NumCPU() / 2),
	MinFreeMemory: 1024 << 20,
	Port:          9085,
})

func (x *PeerFlags) Flags(cfv CommandFlagsVisitor) {
	cfv.Persistent("MaxThreads", "set peer maximum CPU usage", &x.MaxThreads)
	cfv.Persistent("MinFreeMemory", "set peer minimum memory available to accept a job", &x.MinFreeMemory)
	cfv.Persistent("PeerMode", "set peer mode ["+JoinString(",", PeerModes()...)+"]", &x.Mode)
	cfv.Persistent("PeerPort", "set peer TCP port used for communicating with cluster", &x.Port)
}

/***************************************
 * Peer public/private keys
 ***************************************/

const (
	PeerPublicKeyLen = ed25519.PublicKeySize
	PeerSecretKeyLen = ed25519.PrivateKeySize
)

type PeerPublicKey [PeerPublicKeyLen]byte

func (x *PeerPublicKey) Serialize(ar Archive) {
	ar.Raw((*x)[:])
}
func (x PeerPublicKey) String() string {
	return base64.RawStdEncoding.EncodeToString(x[:])
}
func (x *PeerPublicKey) Set(in string) error {
	if base64.RawStdEncoding.EncodedLen(PeerPublicKeyLen) != len(in) {
		return fmt.Errorf("invalid public key len")
	} else if raw, err := base64.RawStdEncoding.DecodeString(in); err == nil {
		copy((*x)[:], raw)
		return nil
	} else {
		return err
	}
}

type PeerSecretKey [PeerSecretKeyLen]byte

func (x *PeerSecretKey) Serialize(ar Archive) {
	ar.Raw((*x)[:])
}
func (x PeerSecretKey) String() string {
	return base64.RawStdEncoding.EncodeToString(x[:])
}
func (x *PeerSecretKey) Set(in string) error {
	if base64.RawStdEncoding.EncodedLen(PeerSecretKeyLen) != len(in) {
		return fmt.Errorf("invalid private key len")
	} else if raw, err := base64.RawStdEncoding.DecodeString(in); err == nil {
		copy((*x)[:], raw)
		return nil
	} else {
		return err
	}
}

/***************************************
 * Local Peer
 ***************************************/

type LocalPeer struct {
	PeerInfo
	PublicKey PeerPublicKey
	SecretKey PeerSecretKey
}

func NewLocalPeer() (result LocalPeer, err error) {
	if result.PeerInfo, err = CurrentPeerInfo(); err != nil {
		return
	}
	workerFlags := GetWorkerFlags()
	result.PublicKey = workerFlags.PublicKey
	result.SecretKey = workerFlags.SecretKey
	return
}
func (x *LocalPeer) EdKeyPair() (result secrethandshake.EdKeyPair) {
	result.Public = x.PublicKey[:]
	result.Secret = x.SecretKey[:]
	return
}

/***************************************
 * Peer informations
 ***************************************/

type PeerInfo struct {
	Version  PeerVersion
	IPs      []net.IP
	Host     string
	Domain   string
	Hardware PeerHardware

	*PeerFlags
}

func CurrentPeerInfo() (peer PeerInfo, err error) {
	peer.Version = PEERVERSION_1_0
	peer.PeerFlags = GetPeerFlags()

	// Retrieve the FQDN (Fully Qualified Domain Name)
	if peer.Host, err = os.Hostname(); err != nil {
		return
	}

	if peer.IPs, err = net.LookupIP(peer.Host); err != nil {
		return
	}

	peer.IPs = RemoveUnless(func(ip net.IP) bool {
		if ip.IsLoopback() || ip.IsUnspecified() {
			return false
		}
		ptr, err := net.LookupAddr(ip.String())
		if err == nil && len(ptr) > 0 {
			peer.Domain = ptr[0]
		}
		return true
	}, peer.IPs...)

	// Retrieve hardware survey
	if peer.Hardware, err = CurrentPeerHardware(); err != nil {
		return
	}

	return
}

func (x *PeerInfo) FullyQualifiedAddress() string {
	if len(x.Domain) > 0 {
		return fmt.Sprintf("%s.%s:%s", x.Host, x.Domain, x.Port)
	} else {
		return fmt.Sprintf("%s:%s", x.Host, x.Port)
	}

}
func (x *PeerInfo) FullyQualifiedDomainName() string {
	if len(x.Domain) > 0 {
		return fmt.Sprintf("%s.%s", x.Host, x.Domain)
	} else {
		return x.Host
	}
}

func (x *PeerInfo) Load(rd io.Reader) error {
	return JsonDeserialize(x, rd)
}
func (x *PeerInfo) Save(wr *os.File) error {
	if err := wr.Chmod(0644); err != nil {
		return err
	}
	return JsonSerialize(x, wr)
}
