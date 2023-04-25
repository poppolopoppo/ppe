package compile

import (
	"build/utils"
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

	SizePerUnity    utils.IntVar
	AdaptiveUnity   utils.BoolVar
	Benchmark       utils.BoolVar
	Deterministic   utils.BoolVar
	LTO             utils.BoolVar
	Incremental     utils.BoolVar
	RuntimeChecks   utils.BoolVar
	CompilerVerbose utils.BoolVar
	LinkerVerbose   utils.BoolVar
}

type Cpp interface {
	GetCpp() *CppRules
	utils.Serializable
}

func (rules *CppRules) GetCpp() *CppRules {
	return rules
}
func (rules *CppRules) Serialize(ar utils.Archive) {
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
	ar.Serializable(&rules.LTO)
	ar.Serializable(&rules.Incremental)
	ar.Serializable(&rules.RuntimeChecks)
	ar.Serializable(&rules.CompilerVerbose)
	ar.Serializable(&rules.LinkerVerbose)
}
func (rules *CppRules) Inherit(other *CppRules) {
	utils.Inherit(&rules.CppStd, other.CppStd)
	utils.Inherit(&rules.CppRtti, other.CppRtti)
	utils.Inherit(&rules.DebugSymbols, other.DebugSymbols)
	utils.Inherit(&rules.Exceptions, other.Exceptions)
	utils.Inherit(&rules.PCH, other.PCH)
	utils.Inherit(&rules.Link, other.Link)
	utils.Inherit(&rules.Sanitizer, other.Sanitizer)
	utils.Inherit(&rules.Unity, other.Unity)

	utils.Inherit(&rules.SizePerUnity, other.SizePerUnity)
	utils.Inherit(&rules.AdaptiveUnity, other.AdaptiveUnity)
	utils.Inherit(&rules.Benchmark, other.Benchmark)
	utils.Inherit(&rules.Deterministic, other.Deterministic)
	utils.Inherit(&rules.LTO, other.LTO)
	utils.Inherit(&rules.Incremental, other.Incremental)
	utils.Inherit(&rules.RuntimeChecks, other.RuntimeChecks)
	utils.Inherit(&rules.CompilerVerbose, other.CompilerVerbose)
	utils.Inherit(&rules.LinkerVerbose, other.LinkerVerbose)
}
func (rules *CppRules) Overwrite(other *CppRules) {
	utils.Overwrite(&rules.CppStd, other.CppStd)
	utils.Overwrite(&rules.CppRtti, other.CppRtti)
	utils.Overwrite(&rules.DebugSymbols, other.DebugSymbols)
	utils.Overwrite(&rules.Exceptions, other.Exceptions)
	utils.Overwrite(&rules.PCH, other.PCH)
	utils.Overwrite(&rules.Link, other.Link)
	utils.Overwrite(&rules.Sanitizer, other.Sanitizer)
	utils.Overwrite(&rules.Unity, other.Unity)

	utils.Overwrite(&rules.SizePerUnity, other.SizePerUnity)
	utils.Overwrite(&rules.AdaptiveUnity, other.AdaptiveUnity)
	utils.Overwrite(&rules.Benchmark, other.Benchmark)
	utils.Overwrite(&rules.Deterministic, other.Deterministic)
	utils.Overwrite(&rules.LTO, other.LTO)
	utils.Overwrite(&rules.Incremental, other.Incremental)
	utils.Overwrite(&rules.RuntimeChecks, other.RuntimeChecks)
	utils.Overwrite(&rules.CompilerVerbose, other.CompilerVerbose)
	utils.Overwrite(&rules.LinkerVerbose, other.LinkerVerbose)
}
