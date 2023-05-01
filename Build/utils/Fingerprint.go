package utils

import (
	"encoding/hex"
	"fmt"
	"io"
	"runtime/debug"
	"time"

	"github.com/minio/sha256-simd"
)

/***************************************
 * Fingerprint
 ***************************************/

type Fingerprint [sha256.Size]byte

func (x *Fingerprint) Serialize(ar Archive) {
	ar.Raw(x[:])
}
func (x Fingerprint) Slice() []byte {
	return x[:]
}
func (x Fingerprint) String() string {
	return hex.EncodeToString(x[:])
}
func (x Fingerprint) ShortString() string {
	return hex.EncodeToString(x[:8])
}
func (x Fingerprint) Valid() bool {
	for _, it := range x {
		if it != 0 {
			return true
		}
	}
	return false
}
func (d *Fingerprint) Set(str string) (err error) {
	var data []byte
	if data, err = hex.DecodeString(str); err == nil {
		if len(data) == sha256.Size {
			copy(d[:], data)
			return nil
		} else {
			err = fmt.Errorf("fingerprint: unexpected string length '%s'", str)
		}
	}
	return err
}
func (x Fingerprint) MarshalText() ([]byte, error) {
	buf := [sha256.Size * 2]byte{}
	Assert(func() bool { return hex.EncodedLen(len(x[:])) == len(buf) })
	hex.Encode(buf[:], x[:])
	return buf[:], nil
}
func (x *Fingerprint) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

/***************************************
 * Serializable Fingerprint
 ***************************************/

func SerializeAnyFingerprint(any func(ar Archive) error, seed Fingerprint) (result Fingerprint, err error) {
	digester := sha256.New()
	if _, err = digester.Write(seed[:]); err != nil {
		return
	}

	ar := NewArchiveBinaryWriter(digester)
	defer ar.Close()

	if err = any(&ar); err != nil {
		return
	}
	LogPanicIfFailed(ar.Error())

	copy(result[:], digester.Sum(nil))
	return
}

func ReaderFingerprint(rd io.Reader, seed Fingerprint) (result Fingerprint, err error) {
	buffer := TransientBytes.Allocate()
	defer TransientBytes.Release(buffer)

	digester := sha256.New()
	if _, err = digester.Write(seed[:]); err != nil {
		return
	}

	for err == nil {
		var len int
		if len, err = rd.Read(buffer); err == nil && len > 0 {
			_, err = digester.Write(buffer[:len])
		}
	}
	if err == io.EOF {
		err = nil
	}

	copy(result[:], digester.Sum(nil))
	return
}

func FileFingerprint(src Filename, seed Fingerprint) (Fingerprint, error) {
	var result Fingerprint
	err := UFS.Open(src, func(rd io.Reader) (err error) {
		result, err = ReaderFingerprint(rd, seed)
		return
	})
	return result, err
}

func StringFingerprint(in string) Fingerprint {
	tmp := TransientBuffer.Allocate()
	defer TransientBuffer.Release(tmp)
	tmp.WriteString(in) // avoid converting string to []byte
	return sha256.Sum256(tmp.Bytes())
}

func SerializeFingerpint(value Serializable, seed Fingerprint) Fingerprint {
	fingerprint, err := SerializeAnyFingerprint(func(ar Archive) error {
		ar.Serializable(value)
		return nil
	}, seed)
	LogPanicIfFailed(err)
	return fingerprint
}

func SerializeDeepEqual(a, b Serializable) bool {
	return SerializeFingerpint(a, Fingerprint{}) == SerializeFingerpint(b, Fingerprint{})
}

/***************************************
 * Process Fingerprint
 ***************************************/

type ProcessInfo struct {
	Path      Filename
	Version   string
	Timestamp time.Time
	Checksum  Future[Fingerprint]
}

func (x ProcessInfo) String() string {
	return fmt.Sprintf("%v-%v-%v", x.Path, x.Version, x.Checksum.Join().Success().ShortString())
}

var PROCESS_INFO = getExecutableInfo()

var GetProcessSeed = Memoize(func() Fingerprint {
	result := PROCESS_INFO.Checksum.Join()
	LogPanicIfFailed(result.Failure())
	return result.Success()
})

func getExecutableInfo_FromFile() (result ProcessInfo) {
	if x, ok := debug.ReadBuildInfo(); ok {

		if x.Main.Path != "" {
			result.Path = MakeFilename(x.Main.Path)
			result.Version = x.Main.Version
			result.Timestamp = UFS.MTime(result.Path)
			result.Checksum = MakeFuture(func() (Fingerprint, error) {
				return StringFingerprint(x.Main.Sum), nil
			})
		} else {
			result.Path = UFS.Executable
			result.Version = "0.60"
			result.Timestamp = UFS.MTime(result.Path)
			result.Checksum = MakeFuture(func() (Fingerprint, error) {
				return FileFingerprint(result.Path, Fingerprint{})
			})
		}
	} else {
		LogPanic("no module build info!")
	}
	// round up timestamp to millisecond, see ArchiveBinaryReader/Writer.Time()
	result.Timestamp = time.UnixMilli(result.Timestamp.UnixMilli())
	return
}

// can disable executable seed for debugging
const process_enable_executable_seed = true

func getExecutableInfo() (result ProcessInfo) {
	if process_enable_executable_seed {
		result = getExecutableInfo_FromFile()
	}
	return result
}
