package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"sort"
)

var AllConfigurations utils.SharedMapT[string, Configuration]

type ConfigRules struct {
	ConfigName string
	ConfigType ConfigType

	CppRules
	Facet
}

type Configuration interface {
	GetConfig() *ConfigRules
	utils.Digestable
	fmt.Stringer
}

func (rules *ConfigRules) String() string {
	return rules.ConfigName
}

func (rules *ConfigRules) GetConfig() *ConfigRules {
	return rules
}
func (rules *ConfigRules) GetCpp() *CppRules {
	return rules.CppRules.GetCpp()
}
func (rules *ConfigRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *ConfigRules) GetDigestable(o *bytes.Buffer) {
	o.WriteString(rules.ConfigName)
	rules.ConfigType.GetDigestable(o)
	rules.CppRules.GetDigestable(o)
}

func (rules *ConfigRules) Decorate(_ *CompileEnv, unit *Unit) {
	switch unit.Payload {
	case PAYLOAD_HEADERS:
	case PAYLOAD_EXECUTABLE, PAYLOAD_OBJECTLIST, PAYLOAD_STATICLIB:
		unit.Facet.Defines.Append("BUILD_LINK_STATIC")
	case PAYLOAD_SHAREDLIB:
		unit.Facet.Defines.Append("BUILD_LINK_DYNAMIC")
	default:
		utils.UnreachableCode()
	}
}

var Configuration_Debug = &ConfigRules{
	ConfigName: "Debug",
	ConfigType: CONFIG_DEBUG,
	CppRules: CppRules{
		CppRtti:      CPPRTTI_ENABLED,
		DebugSymbols: DEBUG_SYMBOLS,
		Exceptions:   EXCEPTION_ENABLED,
		Link:         LINK_STATIC,
		PCH:          PCH_MONOLITHIC,
		Sanitizer:    SANITIZER_NONE,
		Unity:        UNITY_AUTOMATIC,
	},
	Facet: Facet{
		Defines: []string{"DEBUG", "_DEBUG"},
		Tags:    MakeTagFlags(TAG_DEBUG),
	},
}
var Configuration_FastDebug = &ConfigRules{
	ConfigName: "FastDebug",
	ConfigType: CONFIG_FASTDEBUG,
	CppRules: CppRules{
		CppRtti:      CPPRTTI_ENABLED,
		DebugSymbols: DEBUG_HOTRELOAD,
		Exceptions:   EXCEPTION_ENABLED,
		Link:         LINK_DYNAMIC,
		PCH:          PCH_MONOLITHIC,
		Sanitizer:    SANITIZER_NONE,
		Unity:        UNITY_DISABLED,
	},
	Facet: Facet{
		Defines: []string{"DEBUG", "_DEBUG", "FASTDEBUG"},
		Tags:    MakeTagFlags(TAG_FASTDEBUG, TAG_DEBUG),
	},
}
var Configuration_Devel = &ConfigRules{
	ConfigName: "Devel",
	ConfigType: CONFIG_DEVEL,
	CppRules: CppRules{
		CppRtti:      CPPRTTI_DISABLED,
		DebugSymbols: DEBUG_SYMBOLS,
		Exceptions:   EXCEPTION_ENABLED,
		Link:         LINK_STATIC,
		PCH:          PCH_MONOLITHIC,
		Sanitizer:    SANITIZER_NONE,
		Unity:        UNITY_AUTOMATIC,
	},
	Facet: Facet{
		Defines: []string{"RELEASE", "NDEBUG"},
		Tags:    MakeTagFlags(TAG_DEVEL, TAG_NDEBUG),
	},
}
var Configuration_Test = &ConfigRules{
	ConfigName: "Test",
	ConfigType: CONFIG_TEST,
	CppRules: CppRules{
		CppRtti:      CPPRTTI_DISABLED,
		DebugSymbols: DEBUG_SYMBOLS,
		Exceptions:   EXCEPTION_ENABLED,
		Link:         LINK_STATIC,
		PCH:          PCH_MONOLITHIC,
		Sanitizer:    SANITIZER_NONE,
		Unity:        UNITY_AUTOMATIC,
	},
	Facet: Facet{
		Defines: []string{"RELEASE", "NDEBUG", "PROFILING_ENABLED"},
		Tags:    MakeTagFlags(TAG_TEST, TAG_NDEBUG, TAG_PROFILING),
	},
}
var Configuration_Shipping = &ConfigRules{
	ConfigName: "Shipping",
	ConfigType: CONFIG_SHIPPING,
	CppRules: CppRules{
		CppRtti:      CPPRTTI_DISABLED,
		DebugSymbols: DEBUG_SYMBOLS,
		Exceptions:   EXCEPTION_ENABLED,
		Link:         LINK_STATIC,
		PCH:          PCH_MONOLITHIC,
		Sanitizer:    SANITIZER_NONE,
		Unity:        UNITY_AUTOMATIC,
	},
	Facet: Facet{
		Defines: []string{"RELEASE", "NDEBUG", "FINAL_RELEASE"},
		Tags:    MakeTagFlags(TAG_SHIPPING, TAG_NDEBUG),
	},
}

type BuildConfigsT struct {
	Values []Configuration
}

func (x *BuildConfigsT) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Data", "BuildConfigs")
}
func (x *BuildConfigsT) Build(bc utils.BuildContext) (utils.BuildStamp, error) {
	x.Values = AllConfigurations.Values()
	sort.Slice(x.Values, func(i, j int) bool {
		return x.Values[i].String() < x.Values[j].String()
	})

	digester := utils.MakeDigester()
	for _, config := range x.Values {
		digester.Append(config)
	}

	return utils.BuildStamp{
		Content: digester.Finalize(),
	}, nil
}

var BuildConfigs = utils.MakeBuildable(func(utils.BuildInit) *BuildConfigsT {
	return &BuildConfigsT{}
})
