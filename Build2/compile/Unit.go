package compile

import (
	utils "build/utils"
	"bytes"
	"fmt"
	"path"
)

type TargetAlias struct {
	NamespaceName string
	ModuleName    string
	PlatformName  string
	ConfigName    string
}

func NewTargetAlias(module Module, platform Platform, config Configuration) TargetAlias {
	return TargetAlias{
		NamespaceName: module.GetNamespace().String(),
		ModuleName:    module.GetModule().ModuleName,
		PlatformName:  platform.GetPlatform().PlatformName,
		ConfigName:    config.GetConfig().ConfigName,
	}
}
func (x TargetAlias) GetDigestable(o *bytes.Buffer) {
	o.WriteString(x.NamespaceName)
	o.WriteString(x.ModuleName)
	o.WriteString(x.PlatformName)
	o.WriteString(x.ConfigName)
}
func (x TargetAlias) UnitAlias() utils.BuildAlias {
	return utils.BuildAlias(fmt.Sprintf("%v-%v-%v", x.ModuleAlias(), x.PlatformName, x.ConfigName))
}
func (x TargetAlias) ModuleAlias() utils.BuildAlias {
	return utils.BuildAlias(path.Join(x.NamespaceName, x.ModuleName))
}
func (x TargetAlias) NamespaceAlias() utils.BuildAlias {
	return utils.BuildAlias(x.NamespaceName)
}
func (x TargetAlias) String() string {
	return string(x.UnitAlias())
}

type UnitDecorator interface {
	Decorate(env *CompileEnv, unit *Unit)
}

type Unit struct {
	Target TargetAlias

	Ordinal int

	CppRtti    CppRttiType
	CppStd     CppStdType
	Debug      DebugType
	Exceptions ExceptionType
	PCH        PrecompiledHeaderType
	Link       LinkType
	Sanitizer  SanitizerType
	Unity      UnityType
	Payload    PayloadType

	OutputFile utils.Filename

	ModuleDir       utils.Directory
	GeneratedDir    utils.Directory
	IntermediateDir utils.Directory

	GeneratedFiles utils.FileSet

	PrecompiledHeader utils.Filename
	PrecompiledSource utils.Filename
	PrecompiledObject utils.Filename

	CompileDependencies utils.SetT[TargetAlias]
	LinkDependencies    utils.SetT[TargetAlias]
	RuntimeDependencies utils.SetT[TargetAlias]

	Compiler     Compiler
	Preprocessor Compiler

	Source          ModuleSource
	TransitiveFacet Facet // append in case of public dependency
	Facet
}

func (unit *Unit) String() string {
	return unit.Target.String()
}

func (unit *Unit) GetCompiler() *CompilerRules {
	if unit.Compiler != nil {
		return unit.Compiler.GetCompiler()
	} else {
		return nil
	}
}
func (unit *Unit) GetFacet() *Facet {
	return &unit.Facet
}

func (unit *Unit) DebugString() string {
	return utils.PrettyPrint(unit)
}
func (unit *Unit) Decorate(env *CompileEnv, decorator ...UnitDecorator) {
	for _, x := range decorator {
		utils.LogVeryVerbose("unit %v: decorate with %v", unit.Target, x)
		x.Decorate(env, unit)
	}
}

func (unit *Unit) GetDigestable(o *bytes.Buffer) {
	o.WriteString("Unit")
	unit.Target.GetDigestable(o)
	o.WriteString(fmt.Sprint(unit.Ordinal))
	unit.CppRtti.GetDigestable(o)
	unit.CppStd.GetDigestable(o)
	unit.Debug.GetDigestable(o)
	unit.Exceptions.GetDigestable(o)
	unit.PCH.GetDigestable(o)
	unit.Link.GetDigestable(o)
	unit.Sanitizer.GetDigestable(o)
	unit.Unity.GetDigestable(o)
	unit.Payload.GetDigestable(o)
	unit.OutputFile.GetDigestable(o)
	unit.ModuleDir.GetDigestable(o)
	unit.GeneratedDir.GetDigestable(o)
	unit.IntermediateDir.GetDigestable(o)
	utils.MakeDigestable(o, unit.GeneratedFiles...)
	unit.PrecompiledHeader.GetDigestable(o)
	unit.PrecompiledSource.GetDigestable(o)
	unit.PrecompiledObject.GetDigestable(o)
	utils.MakeDigestable(o, unit.CompileDependencies.Slice()...)
	utils.MakeDigestable(o, unit.LinkDependencies.Slice()...)
	utils.MakeDigestable(o, unit.RuntimeDependencies.Slice()...)
	unit.Compiler.GetDigestable(o)
	if unit.Preprocessor != nil {
		unit.Preprocessor.GetDigestable(o)
	} else {
		o.WriteString("%no-preprocessor%")
	}
	unit.Source.GetDigestable(o)
	unit.TransitiveFacet.GetDigestable(o)
	unit.Facet.GetDigestable(o)
}
