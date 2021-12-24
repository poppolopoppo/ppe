package windows

import (
	"build/utils"
	"bytes"
	"strings"
)

type CompilerType int32

const (
	COMPILER_MSVC CompilerType = iota
	COMPILER_CLANGCL
)

func CompilerTypes() []CompilerType {
	return []CompilerType{
		COMPILER_MSVC,
		COMPILER_CLANGCL,
	}
}
func (x CompilerType) String() string {
	switch x {
	case COMPILER_MSVC:
		return "MSVC"
	case COMPILER_CLANGCL:
		return "CLANGCL"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *CompilerType) Set(in string) error {
	switch strings.ToUpper(in) {
	case COMPILER_MSVC.String():
		*x = COMPILER_MSVC
	case COMPILER_CLANGCL.String():
		*x = COMPILER_CLANGCL
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
