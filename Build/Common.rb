# frozen_string_literal: true

require_once 'Utils/deep_dup.rb'
require_once 'Utils/Log.rb'
require_once 'Utils/Options.rb'

module Build
    extend self

    VERSION = '1.0.2'

    persistent_switch(:Cache, 'Use compilation cache')
    persistent_switch(:Diagnose, 'Use compilation diagnostics')
    persistent_switch(:Dist, 'Use distributed compilation')
    persistent_switch(:Incremental, 'Use incremental linker')
    persistent_switch(:LTO, 'Use link time optimization')
    persistent_switch(:Minify, 'Use minified format for exported files', init: true)
    persistent_switch(:PCH, 'Use precompiled headers', init: true)
    persistent_switch(:RuntimeChecks, 'Use runtime checks')
    persistent_switch(:StopOnError, 'Stop on first compilation error', init: false)
    persistent_switch(:Strict, 'Toggle warning as error and non permissive warnings', init: true)
    persistent_switch(:Symbols, 'Toggle generation of debug symbols', init: true)
    persistent_switch(:Unity, 'Use unity builds', init: true)

    opt_switch(:Rebuild, 'Clean build all specified targets', init: false)
    opt_switch(:Timings, 'Measure compilation timings')

    persistent_value(:CppStd, 'Select C++ ISO standard', init: 'c++17', values: %w{ c++14 c++17 c++latest })
    persistent_value(:StackSize, 'Define default thread stack size', init: 2000000)

    def self.set_workspace_path(path)
        path = File.absolute_path(path)
        Log.log("set workspace root = '%s'", path)

        $WorkspacePath = path

        $DataPath = File.join($WorkspacePath, 'Data')
        $BuildPath = File.join($WorkspacePath, 'Build')
        $SourcePath = File.join($WorkspacePath, 'Source')
        $ExtrasPath = File.join($WorkspacePath, 'Extras')
        $OutputPath = File.join($WorkspacePath, 'Output')
        $BinariesPath = File.join($OutputPath, 'Binaries')
        $CachePath = File.join($OutputPath, 'Cache')
        $GeneratedPath = File.join($OutputPath, 'Generated')
        $ProjectsPath = File.join($OutputPath, 'Projects')
        $UnitiesPath = File.join($OutputPath, 'Unity')
        $IntermediatePath = File.join($OutputPath, 'Intermediate')
        $SavedPath = File.join($OutputPath, 'Saved')
        $TemporaryPath = File.join($OutputPath, 'Temp')

        Log.debug("Data path = '%s'", $DataPath)
        Log.debug("Build path = '%s'", $BuildPath)
        Log.debug("Source path = '%s'", $SourcePath)
        Log.debug("Extras path = '%s'", $ExtrasPath)
        Log.debug("Output path = '%s'", $OutputPath)
        Log.debug("Binaries path = '%s'", $BinariesPath)
        Log.debug("Cache path = '%s'", $CachePath)
        Log.debug("Generated path = '%s'", $GeneratedPath)
        Log.debug("Projects path = '%s'", $ProjectsPath)
        Log.debug("Unities path = '%s'", $UnitiesPath)
        Log.debug("Intermediate path = '%s'", $IntermediatePath)
        Log.debug("Saved path = '%s'", $SavedPath)
        Log.debug("Temporary path = '%s'", $TemporaryPath)
    end

    Build.set_workspace_path($ApplicationPath.to_s) # create default global variables

    def self.const_memoize(host, name, &memoized)
        cls = host.class
        ivar = "@#{name}".gsub('?', '_QUESTION').to_sym
        cls.instance_variable_set(ivar, nil)
        cls.define_method(name) do
            value = cls.instance_variable_get(ivar)
            if value.nil?
                Log.debug 'memoize method <%s.%s>', host, name
                value = memoized.call
                value.freeze
                cls.instance_variable_set(ivar, value)
            end
            return value
        end
    end

    RUBY_INTERPRETER_PATH = Pathname.new(File.join(
        RbConfig::CONFIG["bindir"],
        RbConfig::CONFIG["RUBY_INSTALL_NAME"] +
        RbConfig::CONFIG["EXEEXT"] )).realpath.to_s
    RUBY_INTERPRETER_OPTS = $DEBUG ? %w{ --debug } : [] #%w{ --jit-min-calls=4 }

end #~ Build
