package windows

import (
	. "build/compile"
	. "build/utils"
	"io"
)

/***************************************
 * SourceDependencies
 ***************************************/

// https://learn.microsoft.com/en-us/cpp/build/reference/sourcedependencies?view=msvc-170

type SourceDependenciesImportModule struct {
	Name string
	BMI  Filename
}
type SourceDependenciesImportHeaderUnit struct {
	Header Filename
	BMI    Filename
}
type SourceDependenciesData struct {
	Source              Filename
	ProvidedModule      string
	Includes            []Filename
	ImportedModules     []SourceDependenciesImportModule
	ImportedHeaderUnits []SourceDependenciesImportHeaderUnit
}
type SourceDependencies struct {
	Version string
	Data    SourceDependenciesData
}

func (x *SourceDependencies) Load(r io.Reader) error {
	return JsonDeserialize(x, r)
}
func (x SourceDependencies) Files() (result []Filename) {
	result = x.Data.Includes
	for _, module := range x.Data.ImportedModules {
		result = append(result, module.BMI)
	}
	for _, header := range x.Data.ImportedHeaderUnits {
		result = append(result, header.Header, header.BMI)
	}
	return
}

/***************************************
 * SourceDependenciesAction
 ***************************************/

type SourceDependenciesAction struct {
	ActionRules
	SourceDependenciesFile Filename
}

func NewSourceDependenciesAction(rules *ActionRules, output Filename) *SourceDependenciesAction {
	rules.Arguments.Append("/sourceDependencies", MakeLocalFilename(output))
	rules.Extras.Append(output)

	return &SourceDependenciesAction{
		ActionRules:            *rules,
		SourceDependenciesFile: output,
	}
}
func (x *SourceDependenciesAction) Build(bc BuildContext) error {
	// compile the action with /sourceDependencies
	if err := x.ActionRules.Build(bc); err != nil {
		return err
	}

	// track json file as an output dependency
	// #TODO: necessary?
	bc.OutputFile(x.SourceDependenciesFile)

	// parse source dependencies outputted by cl.exe
	var sourceDeps SourceDependencies
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
func (x *SourceDependenciesAction) Serialize(ar Archive) {
	ar.Serializable(&x.ActionRules)
	ar.Serializable(&x.SourceDependenciesFile)
}
