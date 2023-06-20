module ppe

go 1.18

replace build => ./Build

require build v0.0.0-00010101000000-000000000000

replace go.cryptoscope.co/secretstream => github.com/cryptoscope/secretstream v1.2.10

replace go.cryptoscope.co/netwrap => github.com/cryptoscope/netwrap v0.1.0

require (
	filippo.io/edwards25519 v1.0.0-rc.1 // indirect
	github.com/djherbis/times v1.5.0 // indirect
	github.com/felixge/fgprof v0.9.3 // indirect
	github.com/go-ole/go-ole v1.2.6 // indirect
	github.com/goccy/go-json v0.10.2 // indirect
	github.com/google/pprof v0.0.0-20230429030804-905365eefe3e // indirect
	github.com/klauspost/compress v1.16.5 // indirect
	github.com/klauspost/cpuid/v2 v2.2.4 // indirect
	github.com/minio/sha256-simd v1.0.0 // indirect
	github.com/pierrec/lz4/v4 v4.1.17 // indirect
	github.com/pkg/profile v1.7.0 // indirect
	github.com/shirou/gopsutil v3.21.11+incompatible // indirect
	github.com/tklauser/go-sysconf v0.3.11 // indirect
	github.com/tklauser/numcpus v0.6.0 // indirect
	github.com/yusufpapurcu/wmi v1.2.3 // indirect
	go.cryptoscope.co/secretstream v1.2.10 // indirect
	golang.org/x/crypto v0.0.0-20210616213533-5ff15b29337e // indirect
	golang.org/x/sys v0.7.0 // indirect
)
