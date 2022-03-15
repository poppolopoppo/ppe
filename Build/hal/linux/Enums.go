package linux

import (
	"build/utils"
	"bytes"
	"strings"
)

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
		utils.UnexpectedValue(x)
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
		utils.UnexpectedValue(in)
	}
	return nil
}
func (x CompilerType) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.String())
}
func (x CompilerType) MarshalJSON() ([]byte, error) {
	return utils.MarshalJSON(x)
}
func (x *CompilerType) UnmarshalJSON(data []byte) error {
	return utils.UnmarshalJSON(x, data)
}
