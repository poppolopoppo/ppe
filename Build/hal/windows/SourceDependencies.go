package windows

import (
	//lint:ignore ST1001 ignore dot imports warning
	. "build/compile"
	//lint:ignore ST1001 ignore dot imports warning
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
	Includes            FileSet
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
	result = x.Data.Includes.Normalize()
	if x.Data.PCH.Valid() {
		result = append(result, x.Data.PCH.Normalize())
	}
	for _, module := range x.Data.ImportedModules {
		result = append(result, module.BMI.Normalize())
	}
	for _, header := range x.Data.ImportedHeaderUnits {
		result = append(result, header.Header.Normalize(), header.BMI.Normalize())
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

func (x *MsvcSourceDependenciesAction) Alias() BuildAlias {
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
	LogDebug(LogWindows, "sourceDependencies: parsed output in %q\n%v", x.SourceDependenciesFile, MakeStringer(func() string {
		return PrettyPrint(dependentFiles)
	}))

	if flags := GetActionFlags(); flags.ShowFiles.Get() {
		for _, file := range dependentFiles {
			LogForwardf("%v: [%s]  %s", MakeStringer(func() string {
				return x.Alias().String()
			}), FILEACCESS_READ, file)
		}
	}

	return bc.NeedFile(dependentFiles...)
}
func (x *MsvcSourceDependenciesAction) Serialize(ar Archive) {
	ar.Serializable(&x.ActionRules)
	ar.Serializable(&x.SourceDependenciesFile)
}
