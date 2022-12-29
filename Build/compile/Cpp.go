package compile

import (
	"build/utils"
	"bytes"
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

	SizePerUnity  utils.IntVar
	AdaptiveUnity utils.BoolVar
	Benchmark     utils.BoolVar
	LTO           utils.BoolVar
	Incremental   utils.BoolVar
	RuntimeChecks utils.BoolVar
}

type Cpp interface {
	GetCpp() *CppRules
	utils.Digestable
}

func (rules *CppRules) GetCpp() *CppRules {
	return rules
}
func (rules *CppRules) GetDigestable(o *bytes.Buffer) {
	rules.CppStd.GetDigestable(o)
	rules.CppRtti.GetDigestable(o)
	rules.DebugSymbols.GetDigestable(o)
	rules.Exceptions.GetDigestable(o)
	rules.PCH.GetDigestable(o)
	rules.Link.GetDigestable(o)
	rules.Sanitizer.GetDigestable(o)
	rules.Unity.GetDigestable(o)

	rules.SizePerUnity.GetDigestable(o)
	rules.AdaptiveUnity.GetDigestable(o)
	rules.Benchmark.GetDigestable(o)
	rules.LTO.GetDigestable(o)
	rules.Incremental.GetDigestable(o)
	rules.RuntimeChecks.GetDigestable(o)
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
	utils.Inherit(&rules.LTO, other.LTO)
	utils.Inherit(&rules.Incremental, other.Incremental)
	utils.Inherit(&rules.RuntimeChecks, other.RuntimeChecks)
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
	utils.Overwrite(&rules.LTO, other.LTO)
	utils.Overwrite(&rules.Incremental, other.Incremental)
	utils.Overwrite(&rules.RuntimeChecks, other.RuntimeChecks)
}
