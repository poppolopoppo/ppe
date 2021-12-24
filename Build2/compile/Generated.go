package compile

import (
	utils "build/utils"
	"bytes"
)

type GeneratorFunc func(u Unit, rules GeneratedRules, dst utils.Filename) error

type GeneratedRules struct {
	GeneratedName string
	Visibility    VisibilityType
}

type Generated interface {
	GetGenerated() *GeneratedRules
	utils.Buildable
	utils.Digestable
}

func (rules *GeneratedRules) String() string {
	return rules.GeneratedName
}
func (rules *GeneratedRules) GetDigestable(o *bytes.Buffer) {
	o.WriteString(rules.GeneratedName)
	rules.Visibility.GetDigestable(o)
}
func (rules *GeneratedRules) GetGenerateDir(unit *Unit) utils.Directory {
	result := unit.GeneratedDir
	switch rules.Visibility {
	case PRIVATE:
		result = result.Folder("Private")
	case PUBLIC, RUNTIME:
		result = result.Folder("Public")
	default:
		utils.UnexpectedValue(rules.Visibility)
	}
	return result
}
func (rules *GeneratedRules) GetGenerateFile(unit *Unit) utils.Filename {
	return rules.GetGenerateDir(unit).AbsoluteFile(rules.GeneratedName)
}

type GeneratedList []Generated

func (list *GeneratedList) Append(it ...Generated) {
	*list = append(*list, it...)
}
func (list GeneratedList) GetDigestable(o *bytes.Buffer) {
	utils.MakeDigestable(o, list...)
}
