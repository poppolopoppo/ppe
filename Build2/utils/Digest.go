package utils

import (
	"bufio"
	"bytes"
	"fmt"
	"hash"
	"hash/fnv"
	"io"
	"runtime"
	"runtime/debug"

	"github.com/minio/sha256-simd"
)

/***************************************
 * Main module signature
 ***************************************/

var MAIN_MODULEBASE string
var MAIN_MODULEVER string
var MAIN_MODULESUM string
var MAIN_MODULEPATH Filename
var MAIN_SIGNATURE string = makeModuleSignature()

func makeModuleSignature() string {
	if x, ok := debug.ReadBuildInfo(); ok {
		if x.Main.Path != "" {
			MAIN_MODULESUM = x.Main.Sum
			MAIN_MODULEVER = x.Main.Version
			MAIN_MODULEPATH = MakeFilename(x.Main.Path)
		} else {
			MAIN_MODULESUM = "XXXXXXXX"
			MAIN_MODULEVER = "devel"
			MAIN_MODULEPATH = UFS.Output.File("Build.go")
		}
		MAIN_MODULEBASE = MAIN_MODULEPATH.TrimExt()
		return fmt.Sprintf("%v@%v", MAIN_MODULEBASE, MAIN_MODULEVER)
	} else {
		panic("no module build info!")
	}
}

var Seed Digest = make_digester(RawBytes("Build/Seed")).
	Write([]byte(`f3eca51682e2af5f3127ed7676567682425c4a482f40b1d92144f6dd326a6817c5aafe121781bc27c5e3864a13d9bc4ea7f14f0b7b2ba752f9b3e3adbe63bccf
			 f2037bed8ffb67115d27398e04fa3657f3b7c750d038b58adcb17ea8acb4f5a496390b310d8baf10dbe05244f212c5cb0ba3bf3ea27dd32e8cdf22c3ad77cc78
			 aa51f4d00f98bf74bf3141e6d3c276abd850f498a787547a5521c101118101f02d675769b2554342ab6176a42f010d4d0ac56b550405e38e40dc9a30e4187165
			 de73196b80c8046ddd3b2f2ca9903472e82a4498c38c0deb913af15d7b40e8150a1db1f7162ea9576cd4c4ae58ed799416516f1c9eb80d2e21d9e1179f461835
			 dcfe47a4a409b87c33ea33c5e00228e75865bae241b1cfd57b89891ba60b48af38b3bca271186d84a47e41134634f655fc3b0ea79a7eaae4b20176f49c9658c7`)).
	Write([]byte(MAIN_SIGNATURE)).
	Write([]byte(runtime.GOOS)).
	Write([]byte(runtime.GOARCH)).
	Write([]byte(runtime.Version())).Finalize()

func ReflectSourceFile() Filename {
	if _, filename, _, ok := runtime.Caller(1); ok {
		return MakeFilename(filename)
	} else {
		panic("invalid source file")
	}
}
func ReflectSourceDir() Directory {
	return ReflectSourceFile().Dirname
}

/***************************************
 * Digest
 ***************************************/

type RawBytes []byte

type Digest [sha256.Size]byte

type Digestable interface {
	GetDigestable(*bytes.Buffer)
}
type Digester interface {
	Append(Digestable) Digester
	Write(data []byte) Digester
	Finalize() Digest
}

type digester struct {
	intern hash.Hash
	buf    *bytes.Buffer
	cnt    int
}

func (digest Digest) Slice() []byte {
	return digest[:]
}
func (digest Digest) GetDigestable(o *bytes.Buffer) {
	o.Write(digest[:])
}
func (raw RawBytes) GetDigestable(o *bytes.Buffer) {
	o.Write(raw)
}

func make_digester(seed []byte) (result digester) {
	result = digester{
		intern: sha256.New(),
		buf:    &bytes.Buffer{},
		cnt:    0,
	}
	result.buf.Grow(1024)
	result.Write(seed)
	return result
}
func (hash digester) Append(x Digestable) Digester {
	if x != nil {
		hash.buf.Reset()
		x.GetDigestable(hash.buf)
		return hash.Write(hash.buf.Bytes())
	} else {
		return hash.Write(Seed[:])
	}
}
func (hash digester) Write(data []byte) Digester {
	hash.intern.Write(data)
	return hash
}
func (hash digester) Finalize() (result Digest) {
	copy(result[:], hash.intern.Sum(nil))
	return result
}

func MakeDigester() Digester {
	return make_digester(Seed[:])
}
func MakeDigest(data ...Digestable) Digest {
	hash := MakeDigester()
	for _, x := range data {
		hash.Append(x)
	}
	return hash.Finalize()
}
func MapDigest(count int, input func(int) []byte) Digest {
	hash := MakeDigester()
	for i := 0; i < count; i += 1 {
		hash.Write(input(i))
	}
	return hash.Finalize()
}

func MapDigestable(o *bytes.Buffer, count int, input func(int) Digestable) {
	for i := 0; i < count; i += 1 {
		if x := input(i); x != nil {
			x.GetDigestable(o)
		} else {
			o.Write(Seed[:])
		}
	}
}
func MakeDigestable[T Digestable](o *bytes.Buffer, elts ...T) {
	MapDigestable(o, len(elts), func(i int) Digestable {
		return elts[i]
	})
}
func GetDigestable(o *bytes.Buffer, elts ...Digestable) {
	MapDigestable(o, len(elts), func(i int) Digestable {
		return elts[i]
	})
}

func FNV32a(in string) uint32 {
	h := fnv.New32a()
	h.Write([]byte(in))
	return h.Sum32()
}

func FileDigest(src Filename) Future[Digest] {
	return MakeFuture(func() (Digest, error) {
		digester := MakeDigester()
		err := UFS.Open(src, func(rd io.Reader) error {
			const capacity = 64 << 20
			scanner := bufio.NewScanner(rd)
			scanner.Buffer(make([]byte, capacity), capacity)
			for scanner.Scan() {
				digester.Append(RawBytes(scanner.Bytes()))
			}
			return scanner.Err()
		})
		return digester.Finalize(), err
	})
}

type DigestWriter struct {
	digester Digester
	wrapped  io.Writer
}

func NewDigestWriter(dst io.Writer) *DigestWriter {
	return &DigestWriter{
		digester: MakeDigester(),
		wrapped:  dst,
	}
}
func (dw *DigestWriter) Write(data []byte) (int, error) {
	dw.digester.Write(data)
	return dw.wrapped.Write(data)
}
func (dw *DigestWriter) Finalize() Digest {
	return dw.digester.Finalize()
}
