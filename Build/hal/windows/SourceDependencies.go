package windows

import (
	. "build/compile"
	. "build/utils"
	"io"
)

/***************************************
 * MsvcSourceDependencies
 ***************************************/

// https://learn.microsoft.com/en-us/cpp/build/reference/sourcedependencies?view=msvc-170

type MsvcSourceDependenciesImportModule struct {
	Name string
	BMI  Filename
}
type MsvcSourceDependenciesImportHeaderUnit struct {
	Header Filename
	BMI    Filename
}
type MsvcSourceDependenciesData struct {
	Source              Filename
	ProvidedModule      string
	PCH                 Filename
	Includes            []Filename
	ImportedModules     []MsvcSourceDependenciesImportModule
	ImportedHeaderUnits []MsvcSourceDependenciesImportHeaderUnit
}
type MsvcSourceDependencies struct {
	Version string
	Data    MsvcSourceDependenciesData
}

func (x *MsvcSourceDependencies) Load(r io.Reader) error {
	return JsonDeserialize(x, r)
}
func (x MsvcSourceDependencies) Files() (result []Filename) {
	result = x.Data.Includes
	if x.Data.PCH.Valid() {
		result = append(result, x.Data.PCH)
	}
	for _, module := range x.Data.ImportedModules {
		result = append(result, module.BMI)
	}
	for _, header := range x.Data.ImportedHeaderUnits {
		result = append(result, header.Header, header.BMI)
	}
	return
}

/***************************************
 * MsvcSourceDependenciesAction
 ***************************************/

type MsvcSourceDependenciesAction struct {
	ActionRules
	SourceDependenciesFile Filename
}

func NewMsvcSourceDependenciesAction(rules *ActionRules, output Filename) *MsvcSourceDependenciesAction {
	result := &MsvcSourceDependenciesAction{
		ActionRules:            *rules,
		SourceDependenciesFile: output,
	}
	result.Arguments.Append("/sourceDependencies", MakeLocalFilename(output))
	result.Extras.Append(output)
	return result
}

func (x MsvcSourceDependenciesAction) Alias() BuildAlias {
	return MakeBuildAlias("Action", "Msvc", x.Outputs.Join(";"))
}
func (x *MsvcSourceDependenciesAction) Build(bc BuildContext) error {
	// compile the action with /sourceDependencies
	if err := x.ActionRules.Build(bc); err != nil {
		return err
	}

	// track json file as an output dependency (check file exists)
	if err := bc.OutputFile(x.SourceDependenciesFile); err != nil {
		return err
	}

	// parse source dependencies outputted by cl.exe
	var sourceDeps MsvcSourceDependencies
	if err := UFS.OpenBuffered(x.SourceDependenciesFile, sourceDeps.Load); err != nil {
		return err
	}

	// add all parsed filenames as dynamic dependencies: when a header is modified, this action will have to be rebuild
	dependentFiles := sourceDeps.Files()
	LogDebug("sourceDependencies: parsed output in %q\n%v", x.SourceDependenciesFile, MakeStringer(func() string {
		return PrettyPrint(dependentFiles)
	}))
	return bc.NeedFile(dependentFiles...)
}
func (x *MsvcSourceDependenciesAction) Serialize(ar Archive) {
	ar.Serializable(&x.ActionRules)
	ar.Serializable(&x.SourceDependenciesFile)
}
