package compile

import (
	utils "build/utils"
	"bytes"
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
	GetPlatform() *PlatformRules
	utils.Buildable
	utils.Digestable
}

func (rules *PlatformRules) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Platform", rules.String())
}
func (rules *PlatformRules) String() string {
	return rules.PlatformName
}
func (rules *PlatformRules) Build(utils.BuildContext) (utils.BuildStamp, error) {
	return utils.MakeBuildStamp(rules)
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
	off := 0
	x.Values = make([]Platform, AllPlatforms.Len())
	AllPlatforms.Range(func(_ string, rules Platform) {
		x.Values[off] = rules
		off += 1
	})
	sort.Slice(x.Values, func(i, j int) bool {
		return x.Values[i].Alias() < x.Values[j].Alias()
	})
	digester := utils.MakeDigester()
	for _, platform := range x.Values {
		digester.Append(platform)
	}
	return utils.BuildStamp{
		Content: digester.Finalize(),
	}, nil
}

var BuildPlatforms = utils.MakeBuildable(func(utils.BuildInit) *BuildPlatformsT {
	return &BuildPlatformsT{}
})
