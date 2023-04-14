package compile

import (
	"build/utils"
	"fmt"
)

type CustomRules struct {
	CustomName string

	CompilerAlias CompilerAlias
	Source        ModuleSource
	Facet
}

type Custom interface {
	GetCustom() *CustomRules
	utils.Serializable
	fmt.Stringer
}

func (rules *CustomRules) String() string {
	return rules.CustomName
}
func (rules *CustomRules) GetConfig() *CustomRules {
	return rules
}
func (rules *CustomRules) GetCompiler() Compiler {
	compiler, err := utils.FindGlobalBuildable[Compiler](rules.CompilerAlias)
	utils.LogPanicIfFailed(err)
	return compiler
}
func (rules *CustomRules) GetFacet() *Facet {
	return rules.Facet.GetFacet()
}
func (rules *CustomRules) Serialize(ar utils.Archive) {
	ar.String(&rules.CustomName)

	ar.Serializable(&rules.CompilerAlias)
	ar.Serializable(&rules.Source)
	ar.Serializable(&rules.Facet)
}

type CustomList []Custom

func (list *CustomList) Append(it ...Custom) {
	*list = append(*list, it...)
}
func (list *CustomList) Prepend(it ...Custom) {
	*list = append(it, *list...)
}
func (list *CustomList) Serialize(ar utils.Archive) {
	utils.SerializeMany(ar, func(it *Custom) {
		utils.SerializeExternal(ar, it)
	}, (*[]Custom)(list))
}

type CustomUnit struct {
	Unit
}

type CustomUnitList []CustomUnit

func (list *CustomUnitList) Append(it ...CustomUnit) {
	*list = append(*list, it...)
}
func (list *CustomUnitList) Serialize(ar utils.Archive) {
	utils.SerializeSlice(ar, (*[]CustomUnit)(list))
}
