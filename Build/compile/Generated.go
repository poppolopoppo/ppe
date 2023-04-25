package compile

import (
	utils "build/utils"
	"io"
)

/***************************************
 * Generated
 ***************************************/

func MakeGeneratedAlias(output utils.Filename) utils.BuildAlias {
	return utils.MakeBuildAlias("Generated", output.String())
}

type Generated interface {
	Generate(utils.BuildContext, *BuildGenerated, io.Writer) error
	utils.Serializable
}

type BuildGenerated struct {
	OutputFile utils.Filename
	Generated
}

func (x *BuildGenerated) Alias() utils.BuildAlias {
	return MakeGeneratedAlias(x.OutputFile)
}
func (x *BuildGenerated) Build(bc utils.BuildContext) error {
	err := utils.UFS.SafeCreate(x.OutputFile, func(w io.Writer) error {
		return x.Generate(bc, x, w)
	})
	if err == nil {
		err = bc.OutputFile(x.OutputFile)
	}
	return err
}
func (x *BuildGenerated) Serialize(ar utils.Archive) {
	ar.Serializable(&x.OutputFile)
	utils.SerializeExternal(ar, &x.Generated)
}

/***************************************
 * Generator
 ***************************************/

type Generator interface {
	CreateGenerated(unit *Unit, output utils.Filename) Generated
	utils.Serializable
}

type GeneratorList []GeneratorRules

func (list *GeneratorList) Append(it ...GeneratorRules) {
	*list = append(*list, it...)
}
func (list *GeneratorList) Prepend(it ...GeneratorRules) {
	*list = append(it, *list...)
}
func (list *GeneratorList) Serialize(ar utils.Archive) {
	utils.SerializeSlice(ar, (*[]GeneratorRules)(list))
}

type GeneratorRules struct {
	GeneratedName string
	Visibility    VisibilityType
	Generator
}

func (rules *GeneratorRules) GetGenerator() *GeneratorRules {
	return rules
}
func (rules *GeneratorRules) Serialize(ar utils.Archive) {
	ar.String(&rules.GeneratedName)
	ar.Serializable(&rules.Visibility)
	utils.SerializeExternal(ar, &rules.Generator)
}
func (rules *GeneratorRules) GetGenerateDir(unit *Unit) utils.Directory {
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
func (rules *GeneratorRules) GetGenerateFile(unit *Unit) utils.Filename {
	return rules.GetGenerateDir(unit).AbsoluteFile(rules.GeneratedName)
}

func (rules *GeneratorRules) CreateGenerated(bc utils.BuildContext, module Module, unit *Unit) *BuildGenerated {
	outputFile := rules.GetGenerateFile(unit)
	generated := &BuildGenerated{
		OutputFile: outputFile,
		Generated:  rules.Generator.CreateGenerated(unit, outputFile),
	}

	bc.OutputNode(utils.MakeBuildFactory(func(bi utils.BuildInitializer) (*BuildGenerated, error) {
		return generated, nil
	}))

	return generated
}
