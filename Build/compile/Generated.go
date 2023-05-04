package compile

import (
	. "build/utils"
	"io"
)

/***************************************
 * Generated
 ***************************************/

func MakeGeneratedAlias(output Filename) BuildAlias {
	return MakeBuildAlias("Generated", output.String())
}

type Generated interface {
	Generate(BuildContext, *BuildGenerated, io.Writer) error
	Serializable
}

type BuildGenerated struct {
	OutputFile Filename
	Generated
}

func (x *BuildGenerated) Alias() BuildAlias {
	return MakeGeneratedAlias(x.OutputFile)
}
func (x *BuildGenerated) Build(bc BuildContext) error {
	err := UFS.SafeCreate(x.OutputFile, func(w io.Writer) error {
		return x.Generate(bc, x, w)
	})
	if err == nil {
		err = bc.OutputFile(x.OutputFile)
	}
	return err
}
func (x *BuildGenerated) Serialize(ar Archive) {
	ar.Serializable(&x.OutputFile)
	SerializeExternal(ar, &x.Generated)
}

/***************************************
 * Generator
 ***************************************/

type Generator interface {
	CreateGenerated(unit *Unit, output Filename) Generated
	Serializable
}

type GeneratorList []GeneratorRules

func (list *GeneratorList) Append(it ...GeneratorRules) {
	*list = append(*list, it...)
}
func (list *GeneratorList) Prepend(it ...GeneratorRules) {
	*list = append(it, *list...)
}
func (list *GeneratorList) Serialize(ar Archive) {
	SerializeSlice(ar, (*[]GeneratorRules)(list))
}

type GeneratorRules struct {
	GeneratedName string
	Visibility    VisibilityType
	Generator
}

func (rules *GeneratorRules) GetGenerator() *GeneratorRules {
	return rules
}
func (rules *GeneratorRules) Serialize(ar Archive) {
	ar.String(&rules.GeneratedName)
	ar.Serializable(&rules.Visibility)
	SerializeExternal(ar, &rules.Generator)
}
func (rules *GeneratorRules) GetGenerateDir(unit *Unit) Directory {
	result := unit.GeneratedDir
	switch rules.Visibility {
	case PRIVATE:
		result = result.Folder("Private")
	case PUBLIC, RUNTIME:
		result = result.Folder("Public")
	default:
		UnexpectedValue(rules.Visibility)
	}
	return result
}
func (rules *GeneratorRules) GetGenerateFile(unit *Unit) Filename {
	return rules.GetGenerateDir(unit).AbsoluteFile(rules.GeneratedName)
}

func (rules *GeneratorRules) CreateGenerated(bc BuildContext, module Module, unit *Unit) (*BuildGenerated, error) {
	outputFile := rules.GetGenerateFile(unit)
	generated := &BuildGenerated{
		OutputFile: outputFile,
		Generated:  rules.Generator.CreateGenerated(unit, outputFile),
	}

	err := bc.OutputNode(MakeBuildFactory(func(bi BuildInitializer) (*BuildGenerated, error) {
		return generated, nil
	}))
	return generated, err
}
