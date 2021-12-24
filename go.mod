module ppe

go 1.18

replace build => ./Build2

require build v0.0.0-00010101000000-000000000000

require (
	github.com/klauspost/cpuid/v2 v2.0.4 // indirect
	github.com/minio/sha256-simd v1.0.0 // indirect
	github.com/pkg/profile v1.6.0 // indirect
)
