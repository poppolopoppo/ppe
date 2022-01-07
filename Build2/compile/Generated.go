package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"io"
)

type Generator interface {
	Generate(utils.BuildContext, *CompileEnv, *Unit, io.Writer) error
	utils.Digestable
}

type GeneratedRules struct {
	GeneratedName string
	Visibility    VisibilityType
	Generator
}

type Generated interface {
	GetGenerated() *GeneratedRules
	utils.Digestable
	fmt.Stringer
}

func (rules *GeneratedRules) String() string {
	return rules.GeneratedName
}
func (rules *GeneratedRules) GetGenerated() *GeneratedRules {
	return rules
}
func (rules *GeneratedRules) GetDigestable(o *bytes.Buffer) {
	o.WriteString(rules.GeneratedName)
	rules.Visibility.GetDigestable(o)
	rules.Generator.GetDigestable(o)
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

func (rules *GeneratedRules) Generate(bc utils.BuildContext, env *CompileEnv, unit *Unit) error {
	outputFile := rules.GetGenerateFile(unit)
	err := utils.UFS.LazyCreate(outputFile, func(w io.Writer) error {
		return rules.Generator.Generate(bc, env, unit, w)
	})
	if err == nil {
		bc.OutputFile(outputFile)
		unit.GeneratedFiles.Append(outputFile)
		return nil
	}
	return err
}

type GeneratedList []Generated

func (list *GeneratedList) Append(it ...Generated) {
	*list = append(*list, it...)
}
func (list GeneratedList) GetDigestable(o *bytes.Buffer) {
	utils.MakeDigestable(o, list...)
}
