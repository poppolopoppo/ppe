package compile

import (
	utils "build/utils"
	"fmt"
	"strings"
)

/***************************************
 * Configuration Alias
 ***************************************/

type ConfigurationAlias struct {
	ConfigName string
}

func NewConfigurationAlias(configName string) ConfigurationAlias {
	return ConfigurationAlias{ConfigName: configName}
}
func (x ConfigurationAlias) Valid() bool {
	return len(x.ConfigName) > 0
}
func (x ConfigurationAlias) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Rules", "Config", x.String())
}
func (x ConfigurationAlias) String() string {
	utils.Assert(func() bool { return x.Valid() })
	return x.ConfigName
}
func (x *ConfigurationAlias) Serialize(ar utils.Archive) {
	ar.String(&x.ConfigName)
}
func (x ConfigurationAlias) Compare(o ConfigurationAlias) int {
	return strings.Compare(x.ConfigName, o.ConfigName)
}
func (x *ConfigurationAlias) Set(in string) (err error) {
	x.ConfigName = in
	return nil
}
func (x ConfigurationAlias) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *ConfigurationAlias) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}
func (x *ConfigurationAlias) AutoComplete(in utils.AutoComplete) {
	AllConfigurations.Range(func(s string, c Configuration) {
		in.Add(c.String())
	})
}

/***************************************
 * Configuration Rules
 ***************************************/

var AllConfigurations utils.SharedMapT[string, Configuration]

type ConfigRules struct {
	ConfigurationAlias ConfigurationAlias
	ConfigType         ConfigType

	CppRules
	Facet
}

type Configuration interface {
	GetConfig() *ConfigRules
	utils.Serializable
	fmt.Stringer
}

func (rules *ConfigRules) String() string {
	return rules.ConfigurationAlias.String()
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
func (rules *ConfigRules) Serialize(ar utils.Archive) {
	ar.Serializable(&rules.ConfigurationAlias)
	ar.Serializable(&rules.ConfigType)

	ar.Serializable(&rules.CppRules)
	ar.Serializable(&rules.Facet)
}

func (rules *ConfigRules) Decorate(_ *CompileEnv, unit *Unit) error {
	switch unit.Payload {
	case PAYLOAD_HEADERS:
	case PAYLOAD_EXECUTABLE, PAYLOAD_OBJECTLIST, PAYLOAD_STATICLIB:
		unit.Facet.Defines.Append("BUILD_LINK_STATIC")
	case PAYLOAD_SHAREDLIB:
		unit.Facet.Defines.Append("BUILD_LINK_DYNAMIC")
	default:
		utils.UnreachableCode()
	}
	return nil
}

var Configuration_Debug = &ConfigRules{
	ConfigurationAlias: NewConfigurationAlias("Debug"),
	ConfigType:         CONFIG_DEBUG,
	CppRules: CppRules{
		CppRtti:      CPPRTTI_ENABLED,
		DebugSymbols: DEBUG_EMBEDDED,
		Exceptions:   EXCEPTION_ENABLED,
		Link:         LINK_STATIC,
		PCH:          PCH_MONOLITHIC,
		Sanitizer:    SANITIZER_NONE,
		Unity:        UNITY_AUTOMATIC,
	},
	Facet: Facet{
		Defines: []string{"DEBUG", "_DEBUG"},
		Tags:    utils.MakeEnumSet(TAG_DEBUG),
	},
}
var Configuration_FastDebug = &ConfigRules{
	ConfigurationAlias: NewConfigurationAlias("FastDebug"),
	ConfigType:         CONFIG_FASTDEBUG,
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
		Tags:    utils.MakeEnumSet(TAG_FASTDEBUG, TAG_DEBUG),
	},
}
var Configuration_Devel = &ConfigRules{
	ConfigurationAlias: NewConfigurationAlias("Devel"),
	ConfigType:         CONFIG_DEVEL,
	CppRules: CppRules{
		CppRtti:      CPPRTTI_DISABLED,
		DebugSymbols: DEBUG_EMBEDDED,
		Exceptions:   EXCEPTION_ENABLED,
		Link:         LINK_STATIC,
		PCH:          PCH_MONOLITHIC,
		Sanitizer:    SANITIZER_NONE,
		Unity:        UNITY_AUTOMATIC,
	},
	Facet: Facet{
		Defines: []string{"RELEASE", "NDEBUG"},
		Tags:    utils.MakeEnumSet(TAG_DEVEL, TAG_NDEBUG),
	},
}
var Configuration_Test = &ConfigRules{
	ConfigurationAlias: NewConfigurationAlias("Test"),
	ConfigType:         CONFIG_TEST,
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
		Tags:    utils.MakeEnumSet(TAG_TEST, TAG_NDEBUG, TAG_PROFILING),
	},
}
var Configuration_Shipping = &ConfigRules{
	ConfigurationAlias: NewConfigurationAlias("Shipping"),
	ConfigType:         CONFIG_SHIPPING,
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
		Tags:    utils.MakeEnumSet(TAG_SHIPPING, TAG_NDEBUG),
	},
}

/***************************************
 * Build Configuration Factory
 ***************************************/

type BuildConfig struct {
	Configuration
}

func (x *BuildConfig) Alias() utils.BuildAlias {
	return x.GetConfig().ConfigurationAlias.Alias()
}
func (x *BuildConfig) Build(bc utils.BuildContext) error {
	return nil
}
func (x *BuildConfig) Serialize(ar utils.Archive) {
	utils.SerializeExternal(ar, &x.Configuration)
}

func GetBuildConfig(configAlias ConfigurationAlias) utils.BuildFactoryTyped[*BuildConfig] {
	return func(bi utils.BuildInitializer) (*BuildConfig, error) {
		if config, ok := AllConfigurations.Get(configAlias.String()); ok {
			return &BuildConfig{config}, nil
		} else {
			return nil, fmt.Errorf("compile: unknown configuration name %q", configAlias.String())
		}
	}
}

func ForeachBuildConfig(each func(utils.BuildFactoryTyped[*BuildConfig]) error) error {
	for _, configName := range AllConfigurations.Keys() {
		configAlias := NewConfigurationAlias(configName)
		if err := each(GetBuildConfig(configAlias)); err != nil {
			return err
		}
	}
	return nil
}
