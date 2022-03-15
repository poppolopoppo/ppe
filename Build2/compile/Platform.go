package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"sort"
)

var AllPlatforms utils.SharedMapT[string, Platform]

type PlatformRules struct {
	PlatformName string

	Os   string
	Arch ArchType

	Facet
}

type Platform interface {
	GetCompiler(utils.BuildContext) Compiler
	GetPlatform() *PlatformRules
	utils.Digestable
	fmt.Stringer
}

func (rules *PlatformRules) String() string {
	return rules.PlatformName
}
func (rules *PlatformRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *PlatformRules) GetPlatform() *PlatformRules {
	return rules
}
func (rules *PlatformRules) GetDigestable(o *bytes.Buffer) {
	o.WriteString(rules.PlatformName)
	o.WriteString(rules.Os)
	rules.Arch.GetDigestable(o)
	rules.Facet.GetDigestable(o)
}

func (rules *PlatformRules) Decorate(_ *CompileEnv, unit *Unit) {
	unit.Facet.Defines.Append("TARGET_PLATFORM=" + rules.Os)
}

var Platform_X86 = &PlatformRules{
	PlatformName: "x86",
	Arch:         ARCH_X86,
	Facet: Facet{
		Defines: []string{"ARCH_X86", "ARCH_32BIT"},
	},
}
var Platform_X64 = &PlatformRules{
	PlatformName: "x64",
	Arch:         ARCH_X64,
	Facet: Facet{
		Defines: []string{"ARCH_X64", "ARCH_64BIT"},
	},
}
var Platform_ARM = &PlatformRules{
	PlatformName: "arm",
	Arch:         ARCH_ARM,
	Facet: Facet{
		Defines: []string{"ARCH_ARM", "ARCH_64BIT"},
	},
}

type BuildPlatformsT struct {
	Values []Platform
}

func (x *BuildPlatformsT) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Data", "BuildPlatforms")
}
func (x *BuildPlatformsT) Build(bc utils.BuildContext) (utils.BuildStamp, error) {
	x.Values = AllPlatforms.Values()
	sort.Slice(x.Values, func(i, j int) bool {
		return x.Values[i].String() < x.Values[j].String()
	})

	return utils.MakeBuildStamp(x)
}
func (x *BuildPlatformsT) Current() Platform {
	arch := CurrentArch()
	for _, platform := range x.Values {
		if platform.GetPlatform().Arch == arch {
			return platform
		}
	}
	utils.UnreachableCode()
	return nil
}
func (x *BuildPlatformsT) GetDigestable(o *bytes.Buffer) {
	for _, platform := range x.Values {
		platform.GetDigestable(o)
	}
}

var BuildPlatforms = utils.MakeBuildable(func(utils.BuildInit) *BuildPlatformsT {
	return &BuildPlatformsT{}
})
