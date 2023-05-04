package compile

import (
	utils "build/utils"
	"fmt"
	"runtime"
	"strconv"
	"strings"
)

/***************************************
 * ArchType
 ***************************************/

type ArchType int32

const (
	ARCH_X86 ArchType = iota
	ARCH_X64
	ARCH_ARM
	ARCH_ARM64
)

var CurrentArch = utils.Memoize(func() ArchType {
	switch runtime.GOARCH {
	case "386":
		return ARCH_X86
	case "amd64":
		return ARCH_X64
	case "arm":
		return ARCH_ARM
	case "arm64":
		return ARCH_ARM64
	default:
		utils.UnexpectedValue(runtime.GOARCH)
		return ARCH_ARM
	}
})

func ArchTypes() []ArchType {
	return []ArchType{
		ARCH_X86,
		ARCH_X64,
		ARCH_ARM,
		ARCH_ARM64,
	}
}
func (x ArchType) Equals(o ArchType) bool {
	return (x == o)
}
func (x ArchType) String() string {
	switch x {
	case ARCH_X86:
		return "x86"
	case ARCH_X64:
		return "x64"
	case ARCH_ARM:
		return "ARM"
	case ARCH_ARM64:
		return "ARM64"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *ArchType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case ARCH_X86.String():
		*x = ARCH_X86
	case ARCH_X64.String():
		*x = ARCH_X64
	case ARCH_ARM.String():
		*x = ARCH_ARM
	case ARCH_ARM64.String():
		*x = ARCH_ARM64
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *ArchType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x ArchType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *ArchType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *ArchType) AutoComplete(in utils.AutoComplete) {
	for _, it := range ArchTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * CompilerFeatures
 ***************************************/

type CompilerFeature int32

type CompilerFeatureFlags = utils.EnumSet[CompilerFeature, *CompilerFeature]

const (
	COMPILER_ALLOW_CACHING CompilerFeature = iota
	COMPILER_ALLOW_DISTRIBUTION
	COMPILER_ALLOW_RESPONSEFILE
	COMPILER_ALLOW_SOURCEMAPPING
)

func CompilerFeatures() []CompilerFeature {
	return []CompilerFeature{
		COMPILER_ALLOW_CACHING,
		COMPILER_ALLOW_DISTRIBUTION,
		COMPILER_ALLOW_RESPONSEFILE,
		COMPILER_ALLOW_SOURCEMAPPING,
	}
}
func (x CompilerFeature) Ord() int32       { return int32(x) }
func (x *CompilerFeature) FromOrd(v int32) { *x = CompilerFeature(v) }
func (x CompilerFeature) String() string {
	switch x {
	case COMPILER_ALLOW_CACHING:
		return "COMPILER_ALLOW_CACHING"
	case COMPILER_ALLOW_DISTRIBUTION:
		return "COMPILER_ALLOW_DISTRIBUTION"
	case COMPILER_ALLOW_RESPONSEFILE:
		return "COMPILER_ALLOW_RESPONSEFILE"
	case COMPILER_ALLOW_SOURCEMAPPING:
		return "COMPILER_ALLOW_SOURCEMAPPING"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *CompilerFeature) Set(in string) error {
	switch strings.ToUpper(in) {
	case COMPILER_ALLOW_CACHING.String():
		*x = COMPILER_ALLOW_CACHING
	case COMPILER_ALLOW_DISTRIBUTION.String():
		*x = COMPILER_ALLOW_DISTRIBUTION
	case COMPILER_ALLOW_RESPONSEFILE.String():
		*x = COMPILER_ALLOW_RESPONSEFILE
	case COMPILER_ALLOW_SOURCEMAPPING.String():
		*x = COMPILER_ALLOW_SOURCEMAPPING
	default:
		return utils.MakeUnexpectedValueError(x, in)
	}
	return nil
}
func (x *CompilerFeature) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x CompilerFeature) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *CompilerFeature) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *CompilerFeature) AutoComplete(in utils.AutoComplete) {
	for _, it := range CompilerFeatures() {
		in.Add(it.String())
	}
}

/***************************************
 * ConfigType
 ***************************************/

type ConfigType int32

const (
	CONFIG_DEBUG ConfigType = iota
	CONFIG_FASTDEBUG
	CONFIG_DEVEL
	CONFIG_TEST
	CONFIG_SHIPPING
)

func ConfigTypes() []ConfigType {
	return []ConfigType{
		CONFIG_DEBUG,
		CONFIG_FASTDEBUG,
		CONFIG_DEVEL,
		CONFIG_TEST,
		CONFIG_SHIPPING,
	}
}
func (x ConfigType) String() string {
	switch x {
	case CONFIG_DEBUG:
		return "DEBUG"
	case CONFIG_FASTDEBUG:
		return "FASTDEBUG"
	case CONFIG_DEVEL:
		return "DEVEL"
	case CONFIG_TEST:
		return "TEST"
	case CONFIG_SHIPPING:
		return "SHIPPING"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *ConfigType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case CONFIG_DEBUG.String():
		*x = CONFIG_DEBUG
	case CONFIG_FASTDEBUG.String():
		*x = CONFIG_FASTDEBUG
	case CONFIG_DEVEL.String():
		*x = CONFIG_DEVEL
	case CONFIG_TEST.String():
		*x = CONFIG_TEST
	case CONFIG_SHIPPING.String():
		*x = CONFIG_SHIPPING
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *ConfigType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x ConfigType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *ConfigType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *ConfigType) AutoComplete(in utils.AutoComplete) {
	for _, it := range ConfigTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * CppRttiType
 ***************************************/

type CppRttiType int32

const (
	CPPRTTI_INHERIT CppRttiType = iota
	CPPRTTI_ENABLED
	CPPRTTI_DISABLED
)

func CppRttiTypes() []CppRttiType {
	return []CppRttiType{
		CPPRTTI_INHERIT,
		CPPRTTI_ENABLED,
		CPPRTTI_DISABLED,
	}
}
func (x CppRttiType) String() string {
	switch x {
	case CPPRTTI_INHERIT:
		return "INHERIT"
	case CPPRTTI_ENABLED:
		return "ENABLED"
	case CPPRTTI_DISABLED:
		return "DISABLED"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x CppRttiType) IsInheritable() bool {
	return x == CPPRTTI_INHERIT
}
func (x *CppRttiType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case CPPRTTI_INHERIT.String():
		*x = CPPRTTI_INHERIT
	case CPPRTTI_ENABLED.String():
		*x = CPPRTTI_ENABLED
	case CPPRTTI_DISABLED.String():
		*x = CPPRTTI_DISABLED
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *CppRttiType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x CppRttiType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *CppRttiType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *CppRttiType) AutoComplete(in utils.AutoComplete) {
	for _, it := range CppRttiTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * CppStd
 ***************************************/

type CppStdType int32

const (
	CPPSTD_INHERIT CppStdType = iota
	CPPSTD_LATEST
	CPPSTD_11
	CPPSTD_14
	CPPSTD_17
	CPPSTD_20
)

func CppStdTypes() []CppStdType {
	return []CppStdType{
		CPPSTD_INHERIT,
		CPPSTD_LATEST,
		CPPSTD_20,
		CPPSTD_17,
		CPPSTD_14,
		CPPSTD_11,
	}
}
func (x CppStdType) String() string {
	switch x {
	case CPPSTD_INHERIT:
		return "INHERIT"
	case CPPSTD_LATEST:
		return "LATEST"
	case CPPSTD_20:
		return "C++20"
	case CPPSTD_17:
		return "C++17"
	case CPPSTD_14:
		return "C++14"
	case CPPSTD_11:
		return "C++11"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *CppStdType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case CPPSTD_INHERIT.String():
		*x = CPPSTD_INHERIT
	case CPPSTD_LATEST.String():
		*x = CPPSTD_LATEST
	case CPPSTD_20.String():
		*x = CPPSTD_20
	case CPPSTD_17.String():
		*x = CPPSTD_17
	case CPPSTD_14.String():
		*x = CPPSTD_14
	case CPPSTD_11.String():
		*x = CPPSTD_11
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x CppStdType) IsInheritable() bool {
	return x == CPPSTD_INHERIT
}
func (x *CppStdType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x CppStdType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *CppStdType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *CppStdType) AutoComplete(in utils.AutoComplete) {
	for _, it := range CppStdTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * DebugType
 ***************************************/

type DebugType int32

const (
	DEBUG_INHERIT DebugType = iota
	DEBUG_DISABLED
	DEBUG_EMBEDDED
	DEBUG_SYMBOLS
	DEBUG_HOTRELOAD
)

func DebugTypes() []DebugType {
	return []DebugType{
		DEBUG_INHERIT,
		DEBUG_DISABLED,
		DEBUG_EMBEDDED,
		DEBUG_SYMBOLS,
		DEBUG_HOTRELOAD,
	}
}
func (x DebugType) String() string {
	switch x {
	case DEBUG_INHERIT:
		return "INHERIT"
	case DEBUG_DISABLED:
		return "DISABLED"
	case DEBUG_EMBEDDED:
		return "EMBEDDED"
	case DEBUG_SYMBOLS:
		return "SYMBOLS"
	case DEBUG_HOTRELOAD:
		return "HOTRELOAD"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x DebugType) IsInheritable() bool {
	return x == DEBUG_INHERIT
}
func (x *DebugType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case DEBUG_INHERIT.String():
		*x = DEBUG_INHERIT
	case DEBUG_DISABLED.String():
		*x = DEBUG_DISABLED
	case DEBUG_EMBEDDED.String():
		*x = DEBUG_EMBEDDED
	case DEBUG_SYMBOLS.String():
		*x = DEBUG_SYMBOLS
	case DEBUG_HOTRELOAD.String():
		*x = DEBUG_HOTRELOAD
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *DebugType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x DebugType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *DebugType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *DebugType) AutoComplete(in utils.AutoComplete) {
	for _, it := range DebugTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * Exceptions
 ***************************************/

type ExceptionType int32

const (
	EXCEPTION_INHERIT ExceptionType = iota
	EXCEPTION_DISABLED
	EXCEPTION_ENABLED
)

func ExceptionTypes() []ExceptionType {
	return []ExceptionType{
		EXCEPTION_INHERIT,
		EXCEPTION_DISABLED,
		EXCEPTION_ENABLED,
	}
}
func (x ExceptionType) String() string {
	switch x {
	case EXCEPTION_INHERIT:
		return "INHERIT"
	case EXCEPTION_DISABLED:
		return "DISABLED"
	case EXCEPTION_ENABLED:
		return "ENABLED"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x ExceptionType) IsInheritable() bool {
	return x == EXCEPTION_INHERIT
}
func (x *ExceptionType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case EXCEPTION_INHERIT.String():
		*x = EXCEPTION_INHERIT
	case EXCEPTION_DISABLED.String():
		*x = EXCEPTION_DISABLED
	case EXCEPTION_ENABLED.String():
		*x = EXCEPTION_ENABLED
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *ExceptionType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x ExceptionType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *ExceptionType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *ExceptionType) AutoComplete(in utils.AutoComplete) {
	for _, it := range ExceptionTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * LinkType
 ***************************************/

type LinkType int32

const (
	LINK_INHERIT LinkType = iota
	LINK_STATIC
	LINK_DYNAMIC
)

func LinkTypes() []LinkType {
	return []LinkType{
		LINK_INHERIT,
		LINK_STATIC,
		LINK_DYNAMIC,
	}
}
func (x LinkType) String() string {
	switch x {
	case LINK_INHERIT:
		return "INHERIT"
	case LINK_STATIC:
		return "STATIC"
	case LINK_DYNAMIC:
		return "DYNAMIC"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x LinkType) IsInheritable() bool {
	return x == LINK_INHERIT
}
func (x *LinkType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case LINK_INHERIT.String():
		*x = LINK_INHERIT
	case LINK_STATIC.String():
		*x = LINK_STATIC
	case LINK_DYNAMIC.String():
		*x = LINK_DYNAMIC
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *LinkType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x LinkType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *LinkType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *LinkType) AutoComplete(in utils.AutoComplete) {
	for _, it := range LinkTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * ModuleType
 ***************************************/

type ModuleType int32

const (
	MODULE_PROGRAM ModuleType = iota
	MODULE_LIBRARY
	MODULE_EXTERNAL
	MODULE_HEADERS
)

func ModuleTypes() []ModuleType {
	return []ModuleType{
		MODULE_PROGRAM,
		MODULE_LIBRARY,
		MODULE_EXTERNAL,
		MODULE_HEADERS,
	}
}
func (x ModuleType) String() string {
	switch x {
	case MODULE_PROGRAM:
		return "PROGRAM"
	case MODULE_LIBRARY:
		return "LIBRARY"
	case MODULE_EXTERNAL:
		return "EXTERNAL"
	case MODULE_HEADERS:
		return "HEADERS"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *ModuleType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case MODULE_PROGRAM.String():
		*x = MODULE_PROGRAM
	case MODULE_LIBRARY.String():
		*x = MODULE_LIBRARY
	case MODULE_EXTERNAL.String():
		*x = MODULE_EXTERNAL
	case MODULE_HEADERS.String():
		*x = MODULE_HEADERS
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *ModuleType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x ModuleType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *ModuleType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *ModuleType) AutoComplete(in utils.AutoComplete) {
	for _, it := range ModuleTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * PrecompiledHeaderType
 ***************************************/

type PrecompiledHeaderType int32

const (
	PCH_INHERIT PrecompiledHeaderType = iota
	PCH_DISABLED
	PCH_MONOLITHIC
	PCH_SHARED
)

func PrecompiledHeaderTypes() []PrecompiledHeaderType {
	return []PrecompiledHeaderType{
		PCH_INHERIT,
		PCH_DISABLED,
		PCH_MONOLITHIC,
		PCH_SHARED,
	}
}
func (x PrecompiledHeaderType) String() string {
	switch x {
	case PCH_INHERIT:
		return "INHERIT"
	case PCH_DISABLED:
		return "DISABLED"
	case PCH_MONOLITHIC:
		return "MONOLITHIC"
	case PCH_SHARED:
		return "SHARED"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x PrecompiledHeaderType) IsInheritable() bool {
	return x == PCH_INHERIT
}
func (x *PrecompiledHeaderType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case PCH_INHERIT.String():
		*x = PCH_INHERIT
	case PCH_DISABLED.String():
		*x = PCH_DISABLED
	case PCH_MONOLITHIC.String():
		*x = PCH_MONOLITHIC
	case PCH_SHARED.String():
		*x = PCH_SHARED
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *PrecompiledHeaderType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x PrecompiledHeaderType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *PrecompiledHeaderType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *PrecompiledHeaderType) AutoComplete(in utils.AutoComplete) {
	for _, it := range PrecompiledHeaderTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * PayloadType
 ***************************************/

type PayloadType int32

const (
	PAYLOAD_EXECUTABLE PayloadType = iota
	PAYLOAD_OBJECTLIST
	PAYLOAD_STATICLIB
	PAYLOAD_SHAREDLIB
	PAYLOAD_PRECOMPILEDHEADER
	PAYLOAD_HEADERS
	PAYLOAD_DEBUGSYMBOLS
	PAYLOAD_DEPENDENCIES
)

const NumPayloadTypes = int32(PAYLOAD_HEADERS) + 1

func PayloadTypes() []PayloadType {
	return []PayloadType{
		PAYLOAD_EXECUTABLE,
		PAYLOAD_OBJECTLIST,
		PAYLOAD_STATICLIB,
		PAYLOAD_SHAREDLIB,
		PAYLOAD_PRECOMPILEDHEADER,
		PAYLOAD_HEADERS,
		PAYLOAD_DEBUGSYMBOLS,
		PAYLOAD_DEPENDENCIES,
	}
}
func (x PayloadType) String() string {
	switch x {
	case PAYLOAD_EXECUTABLE:
		return "EXECUTABLE"
	case PAYLOAD_OBJECTLIST:
		return "OBJECTLIST"
	case PAYLOAD_STATICLIB:
		return "STATICLIB"
	case PAYLOAD_SHAREDLIB:
		return "SHAREDLIB"
	case PAYLOAD_PRECOMPILEDHEADER:
		return "PRECOMPILEDHEADER"
	case PAYLOAD_HEADERS:
		return "HEADERS"
	case PAYLOAD_DEBUGSYMBOLS:
		return "DEBUGSYMBOLS"
	case PAYLOAD_DEPENDENCIES:
		return "DEPENDENCIES"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *PayloadType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case PAYLOAD_EXECUTABLE.String():
		*x = PAYLOAD_EXECUTABLE
	case PAYLOAD_OBJECTLIST.String():
		*x = PAYLOAD_OBJECTLIST
	case PAYLOAD_STATICLIB.String():
		*x = PAYLOAD_STATICLIB
	case PAYLOAD_SHAREDLIB.String():
		*x = PAYLOAD_SHAREDLIB
	case PAYLOAD_PRECOMPILEDHEADER.String():
		*x = PAYLOAD_PRECOMPILEDHEADER
	case PAYLOAD_HEADERS.String():
		*x = PAYLOAD_HEADERS
	case PAYLOAD_DEBUGSYMBOLS.String():
		*x = PAYLOAD_DEBUGSYMBOLS
	case PAYLOAD_DEPENDENCIES.String():
		*x = PAYLOAD_DEPENDENCIES
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x PayloadType) Compare(o PayloadType) int {
	if x < o {
		return -1
	} else if x == o {
		return 0
	} else {
		return 1
	}
}
func (x *PayloadType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x PayloadType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *PayloadType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *PayloadType) AutoComplete(in utils.AutoComplete) {
	for _, it := range PayloadTypes() {
		in.Add(it.String())
	}
}

func (x PayloadType) HasLinker() bool {
	switch x {
	case PAYLOAD_EXECUTABLE, PAYLOAD_SHAREDLIB:
		return true
	case PAYLOAD_OBJECTLIST, PAYLOAD_STATICLIB:
	case PAYLOAD_HEADERS, PAYLOAD_PRECOMPILEDHEADER, PAYLOAD_DEBUGSYMBOLS, PAYLOAD_DEPENDENCIES:
	default:
		utils.UnexpectedValue(x)
	}
	return false
}
func (x PayloadType) HasOutput() bool {
	switch x {
	case PAYLOAD_EXECUTABLE, PAYLOAD_OBJECTLIST, PAYLOAD_STATICLIB, PAYLOAD_SHAREDLIB:
		return true
	case PAYLOAD_HEADERS, PAYLOAD_PRECOMPILEDHEADER, PAYLOAD_DEBUGSYMBOLS, PAYLOAD_DEPENDENCIES:
	default:
		utils.UnexpectedValue(x)
	}
	return false
}
func (x PayloadType) HasMultipleInput() bool {
	switch x {
	case PAYLOAD_EXECUTABLE, PAYLOAD_STATICLIB, PAYLOAD_SHAREDLIB:
		return true
	case PAYLOAD_OBJECTLIST, PAYLOAD_HEADERS, PAYLOAD_PRECOMPILEDHEADER, PAYLOAD_DEBUGSYMBOLS, PAYLOAD_DEPENDENCIES:
	default:
		utils.UnexpectedValue(x)
	}
	return false
}

/***************************************
 * SanitizerType
 ***************************************/

type SanitizerType int32

const (
	SANITIZER_INHERIT SanitizerType = iota
	SANITIZER_NONE
	SANITIZER_ADDRESS
	SANITIZER_THREAD
	SANITIZER_UNDEFINED_BEHAVIOR
)

func SanitizerTypes() []SanitizerType {
	return []SanitizerType{
		SANITIZER_INHERIT,
		SANITIZER_NONE,
		SANITIZER_ADDRESS,
		SANITIZER_THREAD,
		SANITIZER_UNDEFINED_BEHAVIOR,
	}
}
func (x SanitizerType) String() string {
	switch x {
	case SANITIZER_INHERIT:
		return "INHERIT"
	case SANITIZER_NONE:
		return "NONE"
	case SANITIZER_ADDRESS:
		return "ADDRESS"
	case SANITIZER_THREAD:
		return "THREAD"
	case SANITIZER_UNDEFINED_BEHAVIOR:
		return "UNDEFINED_BEHAVIOR"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x SanitizerType) IsInheritable() bool {
	return x == SANITIZER_INHERIT
}
func (x *SanitizerType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case SANITIZER_INHERIT.String():
		*x = SANITIZER_INHERIT
	case SANITIZER_NONE.String():
		*x = SANITIZER_NONE
	case SANITIZER_ADDRESS.String():
		*x = SANITIZER_ADDRESS
	case SANITIZER_THREAD.String():
		*x = SANITIZER_THREAD
	case SANITIZER_UNDEFINED_BEHAVIOR.String():
		*x = SANITIZER_UNDEFINED_BEHAVIOR
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *SanitizerType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x SanitizerType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *SanitizerType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *SanitizerType) AutoComplete(in utils.AutoComplete) {
	for _, it := range SanitizerTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * TagType
 ***************************************/

type TagType int32

type TagFlags = utils.EnumSet[TagType, *TagType]

const (
	TAG_DEBUG TagType = iota
	TAG_NDEBUG
	TAG_PROFILING
	TAG_SHIPPING
	TAG_DEVEL
	TAG_TEST
	TAG_FASTDEBUG
)

func TagTypes() []TagType {
	return []TagType{
		TAG_DEBUG,
		TAG_NDEBUG,
		TAG_PROFILING,
		TAG_SHIPPING,
		TAG_DEVEL,
		TAG_TEST,
		TAG_FASTDEBUG,
	}
}
func (x TagType) Ord() int32           { return int32(x) }
func (x *TagType) FromOrd(value int32) { *x = TagType(value) }
func (x TagType) String() string {
	switch x {
	case TAG_DEBUG:
		return "DEBUG"
	case TAG_NDEBUG:
		return "NDEBUG"
	case TAG_PROFILING:
		return "PROFILING"
	case TAG_SHIPPING:
		return "SHIPPING"
	case TAG_DEVEL:
		return "DEVEL"
	case TAG_TEST:
		return "TEST"
	case TAG_FASTDEBUG:
		return "FASTDEBUG"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *TagType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case TAG_DEBUG.String():
		*x = TAG_DEBUG
	case TAG_NDEBUG.String():
		*x = TAG_NDEBUG
	case TAG_PROFILING.String():
		*x = TAG_PROFILING
	case TAG_SHIPPING.String():
		*x = TAG_SHIPPING
	case TAG_DEVEL.String():
		*x = TAG_DEVEL
	case TAG_TEST.String():
		*x = TAG_TEST
	case TAG_FASTDEBUG.String():
		*x = TAG_FASTDEBUG
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *TagType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x TagType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *TagType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *TagType) AutoComplete(in utils.AutoComplete) {
	for _, it := range TagTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * Unity
 ***************************************/

type UnityType int32

const (
	UNITY_INHERIT   UnityType = 0
	UNITY_AUTOMATIC UnityType = -1
	UNITY_DISABLED  UnityType = -2
)

func (x UnityType) Ord() int32 {
	return int32(x)
}
func UnityTypes() []UnityType {
	return []UnityType{
		UNITY_INHERIT,
		UNITY_AUTOMATIC,
		UNITY_DISABLED,
	}
}
func (x UnityType) String() string {
	switch x {
	case UNITY_INHERIT:
		return "INHERIT"
	case UNITY_AUTOMATIC:
		return "AUTOMATIC"
	case UNITY_DISABLED:
		return "DISABLED"
	default:
		if x <= 0 {
			utils.LogPanic("invalid unity type: %v", x)
		}
		return fmt.Sprint(int32(x))
	}
}
func (x UnityType) IsInheritable() bool {
	return x == UNITY_INHERIT
}
func (x *UnityType) Set(in string) error {
	switch strings.ToUpper(in) {
	case UNITY_INHERIT.String():
		*x = UNITY_INHERIT
	case UNITY_AUTOMATIC.String():
		*x = UNITY_AUTOMATIC
	case UNITY_DISABLED.String():
		*x = UNITY_DISABLED
	default:
		if i, err := strconv.Atoi(in); err == nil {
			*x = UnityType(i) // explicit number
		} else {
			return err
		}
	}
	return nil
}
func (x *UnityType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x UnityType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *UnityType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *UnityType) AutoComplete(in utils.AutoComplete) {
	for _, it := range UnityTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * VisibilityType
 ***************************************/

type VisibilityType int32

const (
	PRIVATE VisibilityType = iota
	PUBLIC
	RUNTIME
)

func VisibilityTypes() []VisibilityType {
	return []VisibilityType{
		PRIVATE,
		PUBLIC,
		RUNTIME,
	}
}
func (x VisibilityType) String() string {
	switch x {
	case PRIVATE:
		return "PRIVATE"
	case PUBLIC:
		return "PUBLIC"
	case RUNTIME:
		return "RUNTIME"
	default:
		utils.UnexpectedValue(x)
		return ""
	}
}
func (x *VisibilityType) Set(in string) (err error) {
	switch strings.ToUpper(in) {
	case PRIVATE.String():
		*x = PRIVATE
	case PUBLIC.String():
		*x = PUBLIC
	case RUNTIME.String():
		*x = RUNTIME
	default:
		err = utils.MakeUnexpectedValueError(x, in)
	}
	return err
}
func (x *VisibilityType) Serialize(ar utils.Archive) {
	ar.Int32((*int32)(x))
}
func (x VisibilityType) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *VisibilityType) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
func (x *VisibilityType) AutoComplete(in utils.AutoComplete) {
	for _, it := range VisibilityTypes() {
		in.Add(it.String())
	}
}

/***************************************
 * VisibilityMask
 ***************************************/

type VisibilityMask uint32

const (
	VIS_EVERYTHING VisibilityMask = 0xFFFFFFFF
	VIS_NOTHING    VisibilityMask = 0
)

func MakeVisibilityMask(it ...VisibilityType) (result VisibilityMask) {
	return *result.Append(it...)
}
func (m *VisibilityMask) Append(it ...VisibilityType) *VisibilityMask {
	result := int32(*m)
	for _, x := range it {
		result = result | (int32(1) << int32(x))
	}
	*m = VisibilityMask(result)
	return m
}
func (m *VisibilityMask) Clear() {
	*m = VisibilityMask(0)
}
func (m VisibilityMask) All(o VisibilityMask) bool {
	return (m & o) == o
}
func (m VisibilityMask) Any(o VisibilityMask) bool {
	return (m & o) != 0
}
func (m VisibilityMask) Has(flag VisibilityType) bool {
	return m.All(MakeVisibilityMask(flag))
}
func (m VisibilityMask) Public() bool  { return m.Has(PUBLIC) }
func (m VisibilityMask) Private() bool { return m.Has(PRIVATE) }
func (m VisibilityMask) Runtime() bool { return m.Has(RUNTIME) }
func (m *VisibilityMask) Set(in string) error {
	m.Clear()
	for _, x := range strings.Split(in, "|") {
		var vis VisibilityType
		if err := vis.Set(x); err == nil {
			m.Append(vis)
		} else {
			return err
		}
	}
	return nil
}
func (m VisibilityMask) String() (result string) {
	var notFirst bool
	for i := 0; i < 0xFFFFFFFF; i += 1 {
		flag := int32(1) << int32(i)
		if (int32(m) & flag) == flag {
			if notFirst {
				result += "|"
			}
			result += VisibilityType(i).String()
			notFirst = true
		}
	}
	return result
}
func (x *VisibilityMask) Serialize(ar utils.Archive) {
	ar.UInt32((*uint32)(x))
}
func (x VisibilityMask) MarshalText() ([]byte, error) {
	return utils.UnsafeBytesFromString(x.String()), nil
}
func (x *VisibilityMask) UnmarshalText(data []byte) error {
	return x.Set(utils.UnsafeStringFromBytes(data))
}
