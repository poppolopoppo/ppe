package cmd

import (
	. "build/compile"
	. "build/utils"
)

type BuildCommand struct {
	Targets []StringVar
	Clean   BoolVar
	Glob    BoolVar
	Rebuild BoolVar
}

var CommandBuild = NewCommandable(
	"Compilation",
	"build",
	"launch action compilation process",
	&BuildCommand{
		Clean:   INHERITABLE_FALSE,
		Glob:    INHERITABLE_FALSE,
		Rebuild: INHERITABLE_FALSE,
	})

func (x *BuildCommand) Flags(cfv CommandFlagsVisitor) {
	cfv.Variable("Clean", "erase all by files outputted by selected actions", &x.Clean)
	cfv.Variable("Glob", "treat provided targets as glob expressions", &x.Glob)
	cfv.Variable("Rebuild", "rebuild selected actions, same as building after a clean", &x.Rebuild)
	GetActionFlags().Flags(cfv)
}
func (x *BuildCommand) Init(ci CommandContext) error {
	ci.Options(
		OptionCommandParsableFlags("CommandBuild", "control compilation actions execution", x),
		OptionCommandAllCompilationFlags(),
		OptionCommandConsumeMany("TargetAlias", "build all targets specified as argument", &x.Targets),
	)
	return nil
}
func (x *BuildCommand) Run(cc CommandContext) error {
	LogClaim("build <%v>...", JoinString(">, <", x.Targets...))

	targets := SetT[*TargetActions]{}

	bg := CommandEnv.BuildGraph()

	// select target that match input by globbing
	if x.Glob.Get() {
		targets := []StringVar{}
		for _, input := range x.Targets {
			re := MakeGlobRegexp(input.Get())
			err := ForeachBuildTargets(func(bft BuildFactoryTyped[*BuildTargets]) error {
				result := bft.Build(bg)
				if err := result.Failure(); err == nil {
					for target := range result.Success().Targets {
						if re.MatchString(target.String()) {
							targets = append(targets, StringVar(target.String()))
						}
					}
					return nil
				} else {
					return err
				}
			})
			if err != nil {
				return err
			}
		}

		// overwrite user input with targets found
		x.Targets = targets
	}

	// select target that exactly match input
	for _, input := range x.Targets {
		var target TargetAlias
		if err := target.Set(input.Get()); err != nil {
			return err
		}
		ret := GetTargetActions(target).Build(bg)
		if err := ret.Failure(); err != nil {
			return err
		}
		targets = append(targets, ret.Success())
	}

	if IsLogLevelActive(LOG_VERBOSE) {
		for _, t := range targets {
			for _, a := range t.GetOutputAliases() {
				LogVerbose("build: selected '%v' action", a)
			}
		}
	}

	if x.Clean.Get() || x.Rebuild.Get() {
		if err := x.cleanBuild(targets); err != nil {
			return err
		}
	}

	if !x.Clean.Get() || x.Rebuild.Get() {
		if err := x.doBuild(targets); err != nil {
			return err
		}
	}

	return nil
}
func (x *BuildCommand) doBuild(targets []*TargetActions) error {
	aliases := BuildAliases{}
	for _, t := range targets {
		aliases.Append(t.GetOutputAliases()...)
	}

	pbar := LogProgress(0, 0, "build %d actions", len(aliases))
	defer pbar.Close()

	future := CommandEnv.BuildGraph().BuildMany(aliases,
		OptionBuildForceIf(x.Rebuild.Get()),
		OptionWarningOnMissingOutputIf(!x.Rebuild.Get()),
		OptionBuildOnLaunched(func(BuildNode) error {
			pbar.Grow(1)
			return nil
		}),
		OptionBuildOnBuilt(func(BuildNode) error {
			pbar.Inc()
			return nil
		}))

	return future.Join().Failure()
}
func (x *BuildCommand) cleanBuild(targets []*TargetActions) error {
	aliases := BuildAliases{}
	for _, t := range targets {
		for _, payload := range t.Payloads {
			aliases.Append(payload...)
		}
	}

	actions, err := GetBuildActions(aliases...)
	if err != nil {
		return err
	}

	selecteds := ActionSet{}
	actions.ExpandDependencies(&selecteds)

	pbar := LogProgress(0, 0, "clean %d actions", len(selecteds))
	defer pbar.Close()

	for _, it := range selecteds {
		for _, file := range it.GetAction().Outputs {
			distCleanFile(file)
		}
		for _, file := range it.GetAction().Extras {
			distCleanFile(file)
		}

		pbar.Inc()
	}

	return nil
}
