package compile

import (
	"build/utils"
	"bytes"
	"fmt"
)

type CustomRules struct {
	CustomName string

	Compiler Compiler
	Source   ModuleSource
	Facet
}

type Custom interface {
	GetCustom() *CustomRules
	utils.Digestable
	fmt.Stringer
}

func (rules *CustomRules) String() string {
	return rules.CustomName
}
func (rules *CustomRules) GetConfig() *CustomRules {
	return rules
}
func (rules *CustomRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *CustomRules) GetDigestable(o *bytes.Buffer) {
	o.WriteString(rules.CustomName)
	rules.Compiler.GetDigestable(o)
	rules.Source.GetDigestable(o)
	rules.Facet.GetDigestable(o)
}

type CustomList []Custom

func (list *CustomList) Append(it ...Custom) {
	*list = append(*list, it...)
}
func (list *CustomList) Prepend(it ...Custom) {
	*list = append(it, *list...)
}
func (list CustomList) GetDigestable(o *bytes.Buffer) {
	utils.MakeDigestable(o, list...)
}

type CustomUnit struct {
	Unit
}

type CustomUnitList []*CustomUnit

func (list *CustomUnitList) Append(it ...*CustomUnit) {
	*list = append(*list, it...)
}
func (list CustomUnitList) GetDigestable(o *bytes.Buffer) {
	utils.MakeDigestable(o, list...)
}
