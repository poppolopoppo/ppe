package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"io"
)

type GeneratedFile struct {
	Output utils.Filename
	Target TargetAlias
	Generated
}

func (x *GeneratedFile) Alias() utils.BuildAlias {
	return utils.MakeBuildAlias(x.Target.String(), x.GetGenerated().GeneratedName)
}
func (x *GeneratedFile) Build(bc utils.BuildContext) (utils.BuildStamp, error) {
	unit := utils.Where(func(u *Unit) bool {
		return u.Target == x.Target
	}, BuildTranslatedUnits.Need(bc).Slice()...)

	var fingerprint utils.Digest
	err := utils.UFS.Create(x.Output, func(dst io.Writer) error {
		tmp := bytes.Buffer{}
		if err := x.GetGenerated().Generator.Generate(bc, x, unit, &tmp); err != nil {
			return err
		}
		buf := utils.RawBytes(tmp.Bytes())
		fingerprint = utils.MakeDigest(buf)
		_, err := dst.Write(buf)
		return err
	})
	if err != nil {
		return utils.BuildStamp{}, err
	}

	bc.DependsOn(x.Output)
	return utils.MakeBuildStamp(fingerprint)
}

type Generator interface {
	Generate(utils.BuildContext, *GeneratedFile, *Unit, io.Writer) error
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
