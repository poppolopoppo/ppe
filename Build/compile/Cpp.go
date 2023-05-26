package compile

import (
	//lint:ignore ST1001 ignore dot imports warning
	. "build/utils"
)

type CppRules struct {
	CppStd       CppStdType
	CppRtti      CppRttiType
	DebugSymbols DebugType
	Exceptions   ExceptionType
	PCH          PrecompiledHeaderType
	Link         LinkType
	Sanitizer    SanitizerType
	Unity        UnityType

	SizePerUnity    IntVar
	AdaptiveUnity   BoolVar
	Benchmark       BoolVar
	Deterministic   BoolVar
	DebugFastLink   BoolVar
	LTO             BoolVar
	Incremental     BoolVar
	RuntimeChecks   BoolVar
	CompilerVerbose BoolVar
	LinkerVerbose   BoolVar
}

type Cpp interface {
	GetCpp() *CppRules
	Serializable
}

func (rules *CppRules) GetCpp() *CppRules {
	return rules
}
func (rules *CppRules) Serialize(ar Archive) {
	ar.Serializable(&rules.CppStd)
	ar.Serializable(&rules.CppRtti)
	ar.Serializable(&rules.DebugSymbols)
	ar.Serializable(&rules.Exceptions)
	ar.Serializable(&rules.PCH)
	ar.Serializable(&rules.Link)
	ar.Serializable(&rules.Sanitizer)
	ar.Serializable(&rules.Unity)

	ar.Serializable(&rules.SizePerUnity)
	ar.Serializable(&rules.AdaptiveUnity)
	ar.Serializable(&rules.Benchmark)
	ar.Serializable(&rules.Deterministic)
	ar.Serializable(&rules.DebugFastLink)
	ar.Serializable(&rules.LTO)
	ar.Serializable(&rules.Incremental)
	ar.Serializable(&rules.RuntimeChecks)
	ar.Serializable(&rules.CompilerVerbose)
	ar.Serializable(&rules.LinkerVerbose)
}
func (rules *CppRules) Inherit(other *CppRules) {
	Inherit(&rules.CppStd, other.CppStd)
	Inherit(&rules.CppRtti, other.CppRtti)
	Inherit(&rules.DebugSymbols, other.DebugSymbols)
	Inherit(&rules.Exceptions, other.Exceptions)
	Inherit(&rules.PCH, other.PCH)
	Inherit(&rules.Link, other.Link)
	Inherit(&rules.Sanitizer, other.Sanitizer)
	Inherit(&rules.Unity, other.Unity)

	Inherit(&rules.SizePerUnity, other.SizePerUnity)
	Inherit(&rules.AdaptiveUnity, other.AdaptiveUnity)
	Inherit(&rules.Benchmark, other.Benchmark)
	Inherit(&rules.Deterministic, other.Deterministic)
	Inherit(&rules.DebugFastLink, other.DebugFastLink)
	Inherit(&rules.LTO, other.LTO)
	Inherit(&rules.Incremental, other.Incremental)
	Inherit(&rules.RuntimeChecks, other.RuntimeChecks)
	Inherit(&rules.CompilerVerbose, other.CompilerVerbose)
	Inherit(&rules.LinkerVerbose, other.LinkerVerbose)
}
func (rules *CppRules) Overwrite(other *CppRules) {
	Overwrite(&rules.CppStd, other.CppStd)
	Overwrite(&rules.CppRtti, other.CppRtti)
	Overwrite(&rules.DebugSymbols, other.DebugSymbols)
	Overwrite(&rules.Exceptions, other.Exceptions)
	Overwrite(&rules.PCH, other.PCH)
	Overwrite(&rules.Link, other.Link)
	Overwrite(&rules.Sanitizer, other.Sanitizer)
	Overwrite(&rules.Unity, other.Unity)

	Overwrite(&rules.SizePerUnity, other.SizePerUnity)
	Overwrite(&rules.AdaptiveUnity, other.AdaptiveUnity)
	Overwrite(&rules.Benchmark, other.Benchmark)
	Overwrite(&rules.Deterministic, other.Deterministic)
	Overwrite(&rules.DebugFastLink, other.DebugFastLink)
	Overwrite(&rules.LTO, other.LTO)
	Overwrite(&rules.Incremental, other.Incremental)
	Overwrite(&rules.RuntimeChecks, other.RuntimeChecks)
	Overwrite(&rules.CompilerVerbose, other.CompilerVerbose)
	Overwrite(&rules.LinkerVerbose, other.LinkerVerbose)
}
