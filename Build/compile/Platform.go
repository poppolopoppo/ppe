package compile

import (
	. "build/utils"
	"fmt"
	"strings"
)

/***************************************
 * Plaform Alias
 ***************************************/

type PlatformAlias struct {
	PlatformName string
}

func NewPlatformAlias(platformName string) PlatformAlias {
	return PlatformAlias{PlatformName: platformName}
}
func (x PlatformAlias) Valid() bool {
	return len(x.PlatformName) > 0
}
func (x PlatformAlias) Alias() BuildAlias {
	return MakeBuildAlias("Rules", "Platform", x.String())
}
func (x PlatformAlias) String() string {
	Assert(func() bool { return x.Valid() })
	return x.PlatformName
}
func (x *PlatformAlias) Serialize(ar Archive) {
	ar.String(&x.PlatformName)
}
func (x PlatformAlias) Compare(o PlatformAlias) int {
	return strings.Compare(x.PlatformName, o.PlatformName)
}
func (x *PlatformAlias) Set(in string) (err error) {
	x.PlatformName = in
	return nil
}
func (x PlatformAlias) MarshalText() ([]byte, error) {
	return UnsafeBytesFromString(x.String()), nil
}
func (x *PlatformAlias) UnmarshalText(data []byte) error {
	return x.Set(UnsafeStringFromBytes(data))
}
func (x *PlatformAlias) AutoComplete(in AutoComplete) {
	AllPlatforms.Range(func(s string, p Platform) {
		in.Add(p.String())
	})
}

/***************************************
 * Plaform Rules
 ***************************************/

var AllPlatforms SharedMapT[string, Platform]

type Platform interface {
	GetCompiler() BuildFactoryTyped[Compiler]
	GetPlatform() *PlatformRules
	Buildable
	fmt.Stringer
}

type PlatformRules struct {
	PlatformAlias PlatformAlias

	Os   string
	Arch ArchType

	Facet
}

func (rules *PlatformRules) String() string {
	return rules.PlatformAlias.String()
}
func (rules *PlatformRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *PlatformRules) GetPlatform() *PlatformRules {
	return rules
}
func (rules *PlatformRules) Serialize(ar Archive) {
	ar.Serializable(&rules.PlatformAlias)

	ar.String(&rules.Os)
	ar.Serializable(&rules.Arch)

	ar.Serializable(&rules.Facet)
}

func (rules *PlatformRules) Decorate(_ *CompileEnv, unit *Unit) error {
	unit.Facet.Defines.Append("TARGET_PLATFORM=" + rules.Os)
	return nil
}

var Platform_X86 = &PlatformRules{
	PlatformAlias: NewPlatformAlias("x86"),
	Arch:          ARCH_X86,
	Facet: Facet{
		Defines: []string{"ARCH_X86", "ARCH_32BIT"},
	},
}
var Platform_X64 = &PlatformRules{
	PlatformAlias: NewPlatformAlias("x64"),
	Arch:          ARCH_X64,
	Facet: Facet{
		Defines: []string{"ARCH_X64", "ARCH_64BIT"},
	},
}
var Platform_ARM = &PlatformRules{
	PlatformAlias: NewPlatformAlias("arm"),
	Arch:          ARCH_ARM,
	Facet: Facet{
		Defines: []string{"ARCH_ARM", "ARCH_64BIT"},
	},
}

/***************************************
 * Build Platform Factory
 ***************************************/

func (x *PlatformRules) Alias() BuildAlias {
	return x.GetPlatform().PlatformAlias.Alias()
}
func (x *PlatformRules) Build(bc BuildContext) error {
	return nil
}

func GetBuildPlatform(platformAlias PlatformAlias) BuildFactoryTyped[Platform] {
	return func(bi BuildInitializer) (Platform, error) {
		if plaform, ok := AllPlatforms.Get(platformAlias.String()); ok {
			return plaform, nil
		} else {
			return nil, fmt.Errorf("compile: unknown platform name %q", platformAlias.String())
		}
	}
}

func ForeachBuildPlatform(each func(BuildFactoryTyped[Platform]) error) error {
	for _, platformName := range AllPlatforms.Keys() {
		platformAlias := NewPlatformAlias(platformName)
		if err := each(GetBuildPlatform(platformAlias)); err != nil {
			return err
		}
	}
	return nil
}

var GetLocalHostPlatformAlias = Memoize(func() PlatformAlias {
	arch := CurrentArch()
	for _, platform := range AllPlatforms.Values() {
		if platform.GetPlatform().Arch == arch {
			return platform.GetPlatform().PlatformAlias
		}
	}
	UnreachableCode()
	return PlatformAlias{}
})

func GeLocalHostBuildPlatform() BuildFactoryTyped[Platform] {
	return GetBuildPlatform(GetLocalHostPlatformAlias())
}
