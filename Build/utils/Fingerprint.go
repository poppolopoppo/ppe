package utils

import (
	"encoding/hex"
	"fmt"
	"hash"
	"io"
	"os"
	"runtime/debug"
	"time"

	"github.com/minio/sha256-simd"
)

var LogFingerprint = NewLogCategory("Fingerprint")

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
func (x *Fingerprint) UnmarshalText(data []byte) (err error) {
	n, err := hex.Decode(x.Slice(), data)
	if err == nil && n != sha256.Size {
		err = fmt.Errorf("fingerprint: unexpected string length '%s'", data)
	}
	return err
}

/***************************************
 * Serializable Fingerprint
 ***************************************/

var digesterPool = NewRecycler(
	func() hash.Hash {
		return sha256.New()
	},
	func(digester hash.Hash) {
		digester.Reset()
	})

func SerializeAnyFingerprint(any func(ar Archive) error, seed Fingerprint) (result Fingerprint, err error) {
	digester := digesterPool.Allocate()
	defer digesterPool.Release(digester)

	if _, err = digester.Write(seed[:]); err != nil {
		return
	}

	ar := NewArchiveBinaryWriter(digester, AR_DETERMINISM)
	defer ar.Close()

	if err = any(&ar); err != nil {
		return
	}
	LogPanicIfFailed(LogFingerprint, ar.Error())

	copy(result[:], digester.Sum(nil))
	return
}

func ReaderFingerprint(rd io.Reader, seed Fingerprint) (result Fingerprint, err error) {
	digester := digesterPool.Allocate()
	defer digesterPool.Release(digester)

	buffer := TransientLargePage.Allocate()
	defer TransientLargePage.Release(buffer)

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
	err := UFS.OpenFile(src, func(rd *os.File) (err error) {
		result, err = ReaderFingerprint(rd, seed)
		return
	})
	return result, err
}

func StringFingerprint(in string) Fingerprint {
	tmp := TransientBuffer.Allocate()
	defer TransientBuffer.Release(tmp)
	tmp.WriteString(in) // avoid allocating a new slice to convert string to []byte
	return sha256.Sum256(tmp.Bytes())
}

func SerializeFingerpint(value Serializable, seed Fingerprint) Fingerprint {
	fingerprint, err := SerializeAnyFingerprint(func(ar Archive) error {
		ar.Serializable(value)
		return nil
	}, seed)
	LogPanicIfFailed(LogFingerprint, err)
	return fingerprint
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
	LogPanicIfFailed(LogFingerprint, result.Failure())
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
			result.Version = "0.70"
			result.Timestamp = UFS.MTime(result.Path)
			result.Checksum = MakeFuture(func() (Fingerprint, error) {
				return FileFingerprint(result.Path, Fingerprint{})
			})
		}
	} else {
		LogPanic(LogFingerprint, "no module build info!")
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
