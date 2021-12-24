package compile

import (
	utils "build/utils"
	"bytes"
	"path"
)

type NamespaceRules struct {
	NamespaceName string

	NamespaceParent   Namespace
	NamespaceChildren utils.StringSet
	NamespaceDir      utils.Directory
	NamespaceModules  utils.StringSet

	Facet
}

type Namespace interface {
	GetNamespace() *NamespaceRules
	utils.Buildable
	utils.Digestable
}

func (rules *NamespaceRules) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias("Namespace", rules.String())
}
func (rules *NamespaceRules) String() string {
	return path.Join(rules.Path()...)
}
func (rules *NamespaceRules) Build(utils.BuildContext) (utils.BuildStamp, error) {
	return utils.MakeBuildStamp(rules)
}

func (rules *NamespaceRules) Path() []string {
	// ignores the first level (which is the common root for every other)
	if rules.NamespaceParent != nil && rules.NamespaceParent.GetNamespace().NamespaceParent != nil {
		return append(rules.NamespaceParent.GetNamespace().Path(), rules.NamespaceName)
	} else {
		return []string{rules.NamespaceName}
	}
}

func (rules *NamespaceRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *NamespaceRules) GetNamespace() *NamespaceRules {
	return rules
}
func (rules *NamespaceRules) Decorate(env *CompileEnv, unit *Unit) {
	if rules.NamespaceParent != nil {
		rules.NamespaceParent.GetNamespace().Decorate(env, unit)
	}

	unit.Facet.Append(rules)
}

func (rules *NamespaceRules) GetDigestable(o *bytes.Buffer) {
	o.WriteString(rules.NamespaceName)
	if rules.NamespaceParent != nil {
		rules.NamespaceParent.GetDigestable(o)
	} else {
		o.WriteString("%NO_NAMESPACEPARENT%")
	}
	rules.NamespaceChildren.GetDigestable(o)
	rules.NamespaceDir.GetDigestable(o)
	rules.NamespaceModules.GetDigestable(o)
	rules.Facet.GetDigestable(o)
}
