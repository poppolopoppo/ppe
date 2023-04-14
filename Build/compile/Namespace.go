package compile

import (
	utils "build/utils"
	"fmt"
	"strings"
)

type Namespace interface {
	GetNamespace() *NamespaceRules
	utils.Buildable
	utils.Serializable
	fmt.Stringer
}

/***************************************
 * Namespace Alias
 ***************************************/

type NamespaceAlias struct {
	NamespaceName string
}

type NamespaceAliases = utils.SetT[NamespaceAlias]

func (x NamespaceAlias) Valid() bool {
	return len(x.NamespaceName) > 0
}
func (x NamespaceAlias) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Rules", "Namespace", x.String())
}
func (x *NamespaceAlias) Serialize(ar utils.Archive) {
	ar.String(&x.NamespaceName)
}
func (x NamespaceAlias) String() string {
	return x.NamespaceName
}
func (x NamespaceAlias) Compare(o NamespaceAlias) int {
	return strings.Compare(x.NamespaceName, o.NamespaceName)
}
func (x *NamespaceAlias) Set(in string) (err error) {
	x.NamespaceName = in
	return nil
}
func (x NamespaceAlias) MarshalText() ([]byte, error) {
	return []byte(x.String()), nil
}
func (x *NamespaceAlias) UnmarshalText(data []byte) error {
	return x.Set(string(data))
}

/***************************************
 * Namespace Rules
 ***************************************/

type NamespaceRules struct {
	NamespaceAlias NamespaceAlias

	NamespaceParent   NamespaceAlias
	NamespaceChildren utils.StringSet
	NamespaceDir      utils.Directory
	NamespaceModules  ModuleAliases

	Facet
}

func (rules *NamespaceRules) String() string {
	return rules.NamespaceAlias.String()
}

func (rules *NamespaceRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *NamespaceRules) GetNamespace() *NamespaceRules {
	return rules
}
func (rules *NamespaceRules) GetParentNamespace() Namespace {
	if namespace, err := GetBuildNamespace(rules.NamespaceParent); err == nil {
		return namespace
	} else {
		utils.LogPanicErr(err)
		return nil
	}
}
func (rules *NamespaceRules) Decorate(env *CompileEnv, unit *Unit) error {
	if rules.NamespaceParent.Valid() {
		parent := rules.GetParentNamespace()
		if err := parent.GetNamespace().Decorate(env, unit); err != nil {
			return err
		}
	}

	unit.Facet.Append(rules)
	return nil
}

func (rules *NamespaceRules) Serialize(ar utils.Archive) {
	ar.Serializable(&rules.NamespaceAlias)

	ar.Serializable(&rules.NamespaceParent)
	ar.Serializable(&rules.NamespaceChildren)
	ar.Serializable(&rules.NamespaceDir)
	utils.SerializeSlice(ar, rules.NamespaceModules.Ref())

	ar.Serializable(&rules.Facet)
}

/***************************************
 * Build Namespace
 ***************************************/

func (x *NamespaceRules) Alias() utils.BuildAlias {
	return x.GetNamespace().NamespaceAlias.Alias()
}
func (x *NamespaceRules) Build(bc utils.BuildContext) error {
	return nil
}

func GetBuildNamespace(namespaceAlias NamespaceAlias) (Namespace, error) {
	return utils.FindGlobalBuildable[Namespace](namespaceAlias)
}
