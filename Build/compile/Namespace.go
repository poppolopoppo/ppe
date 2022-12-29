package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"path"
)

type NamespaceRules struct {
	NamespaceName string

	NamespaceParent   Namespace
	NamespaceChildren utils.StringSet
	NamespaceDir      utils.Directory
	NamespaceModules  ModuleAliases

	Facet
}

type Namespace interface {
	GetNamespace() *NamespaceRules
	utils.Digestable
	fmt.Stringer
}

func (rules *NamespaceRules) String() string {
	return path.Join(rules.Path()...)
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
	rules.Facet.GetDigestable(o)
	utils.MakeDigestable(o, rules.NamespaceModules.Slice()...)
}
