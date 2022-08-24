package linux

import (
	. "build/compile"
	. "build/utils"
	"bytes"
	"strings"
)

/***************************************
 * Compiler type
 ***************************************/

type CompilerType int32

const (
	COMPILER_CLANG CompilerType = iota
	COMPILER_GCC
)

func CompilerTypes() []CompilerType {
	return []CompilerType{
		COMPILER_CLANG,
		COMPILER_GCC,
	}
}
func (x CompilerType) String() string {
	switch x {
	case COMPILER_CLANG:
		return "CLANG"
	case COMPILER_GCC:
		return "GCC"
	default:
		UnexpectedValue(x)
		return ""
	}
}
func (x *CompilerType) Set(in string) error {
	switch strings.ToUpper(in) {
	case COMPILER_CLANG.String():
		*x = COMPILER_CLANG
	case COMPILER_GCC.String():
		*x = COMPILER_GCC
	default:
		UnexpectedValue(in)
	}
	return nil
}
func (x CompilerType) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.String())
}
func (x CompilerType) MarshalJSON() ([]byte, error) {
	return MarshalJSON(x)
}
func (x *CompilerType) UnmarshalJSON(data []byte) error {
	return UnmarshalJSON(x, data)
}

/***************************************
 * LLVM Version
 ***************************************/

type LlvmVersion int32

const (
	llvm_any    LlvmVersion = -1
	LLVM_LATEST LlvmVersion = 0
	LLVM_16     LlvmVersion = 16
	LLVM_15     LlvmVersion = 15
	LLVM_14     LlvmVersion = 14
	LLVM_13     LlvmVersion = 13
	LLVM_12     LlvmVersion = 12
	LLVM_11     LlvmVersion = 11
	LLVM_10     LlvmVersion = 10
	LLVM_9      LlvmVersion = 9
)

func LlvmVersions() []LlvmVersion {
	return []LlvmVersion{
		LLVM_16,
		LLVM_15,
		LLVM_14,
		LLVM_13,
		LLVM_12,
		LLVM_11,
		LLVM_10,
		LLVM_9,
	}
}
func (v LlvmVersion) Equals(o LlvmVersion) bool {
	return (v == o)
}
func (v LlvmVersion) String() string {
	switch v {
	case llvm_any:
		return "ANY"
	case LLVM_LATEST:
		return "LATEST"
	case LLVM_16:
		return "16"
	case LLVM_15:
		return "15"
	case LLVM_14:
		return "14"
	case LLVM_13:
		return "13"
	case LLVM_12:
		return "12"
	case LLVM_11:
		return "11"
	case LLVM_10:
		return "10"
	case LLVM_9:
		return "9"
	default:
		UnreachableCode()
		return ""
	}
}
func (v *LlvmVersion) Set(in string) error {
	switch in {
	case LLVM_LATEST.String():
		*v = LLVM_LATEST
	case LLVM_16.String():
		*v = LLVM_16
	case LLVM_15.String():
		*v = LLVM_15
	case LLVM_14.String():
		*v = LLVM_14
	case LLVM_13.String():
		*v = LLVM_13
	case LLVM_12.String():
		*v = LLVM_12
	case LLVM_11.String():
		*v = LLVM_11
	case LLVM_10.String():
		*v = LLVM_10
	case LLVM_9.String():
		*v = LLVM_9
	default:
		UnexpectedValue(in)
	}
	return nil
}
func (v LlvmVersion) GetDigestable(o *bytes.Buffer) {
	o.WriteString(v.String())
}

func getCppStdFromLlvm(ver LlvmVersion) CppStdType {
	switch ver {
	case LLVM_16:
		return CPPSTD_20
	case LLVM_15, LLVM_14, LLVM_13, LLVM_12, LLVM_11:
		return CPPSTD_17
	case LLVM_10:
		return CPPSTD_14
	case LLVM_9:
		return CPPSTD_11
	default:
		UnexpectedValue(ver)
		return CPPSTD_11
	}
}
