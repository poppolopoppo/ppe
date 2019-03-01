#!/bin/env ruby

require 'fileutils'
require 'json'

CORE_PATH=File.dirname(File.absolute_path(__FILE__))

VSCODE_PATH=File.join(CORE_PATH, '.vscode')
VSCODE_C_CPP_PROPERTIES=File.join(VSCODE_PATH, 'c_cpp_properties.json')
VSCODE_C_CPP_PROPERTIES_VERSION=4
VSCODE_TASKS=File.join(VSCODE_PATH, 'tasks.json')
VSCODE_TASKS_VERSION='2.0.0'
VSCODE_LAUNCH=File.join(VSCODE_PATH, 'launch.json')
VSCODE_LAUNCH_VERSION='0.2.0'

VCDB_PATH=File.join(VSCODE_PATH, 'vc.db')

FBUILD_RB=File.join(CORE_PATH, 'fbuild.rb')
FBUILD_COMPDB=File.join(CORE_PATH, 'compile_commands.json')
FBUILD_SOLUTION_PATH=File.join(CORE_PATH, 'Build', '_solution_path.bff')

system('ruby', FBUILD_RB, '-version') # make sure _solution_path.bff is generated
raise "invalid solution path" unless File.exist?(FBUILD_SOLUTION_PATH)

def fetch_fbuilb_string(filename, key)
    m = File.read(filename).match(/\.#{key}\s*=\s*['"]([^'"]*?)['"]/)
    return m.nil? ? nil : m[1]
end

def fetch_all_modules()
    modules = []
    rootpath = File.join(CORE_PATH, 'Source')
    Dir.glob(File.join(rootpath, '**', '*.bff')).each do |bff|
        relpath = File.dirname(File.dirname(bff[(rootpath.length+1)..bff.length]))
        name = fetch_fbuilb_string(bff, 'ModuleName')
        modules << "#{relpath}/#{name}" unless name.nil?
    end
    return modules.sort
end

def fetch_default_target()
    target = fetch_fbuilb_string(File.join(CORE_PATH, 'fbuild.bff'), 'SolutionBuildProject')
    target.gsub!(/-VCXProject$/, '')
    return target
end

def fetch_fastbuild_options()
    opts = fetch_fbuilb_string(File.join(CORE_PATH, 'Build', 'VisualStudio.bff'), 'FastBuildOptions')
    opts = opts.strip.split(/\s+/)
    return opts
end

ALL_MODULES=fetch_all_modules()
DEFAULT_TARGET=fetch_default_target()
FASTBUILD_OPTIONS=fetch_fastbuild_options()

puts "[Default target]      = <#{DEFAULT_TARGET}>"
puts "[Fastbuild options]   = #{FASTBUILD_OPTIONS}"
puts "[All modules]         ="
puts " - #{ALL_MODULES.join(",\n - ")}"

FileUtils.mkdir_p(VCDB_PATH, :verbose => true)

def make_compile_commands(platform, config)
    dirname = File.join(CORE_PATH, 'Output', 'Intermediate', platform, config)
    FileUtils.mkdir_p(dirname, :verbose => true)
    filename = File.join(dirname, "compile_commands.json")
    unless system('ruby', FBUILD_RB, "-compdb", "#{DEFAULT_TARGET}-#{platform}-#{config}", :out => File::NULL)
        raise "compdb generation failed :'("
    end
    FileUtils.mv(FBUILD_COMPDB, filename)
    puts " + #{filename}"
    return filename
end

def make_file(filename, content)
    puts " < #{filename}"
    File.write(filename, content)
end

class Platform
    attr_reader     :name,
                    :intelliSenseMode

    def initialize(name, intelliSenseMode)
        @name = name
        @intelliSenseMode = intelliSenseMode
    end

    def defines() return [] end
    def includePath() return [ "${workspaceRoot}/Source/**" ] end
    def compilerPath() return '' end
    def macFrameworkPath() return [] end
    def binaryExtension() return '' end

end #~Platform

class WindowsPlatform < Platform
    VS141COMNTOOLS          = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'VS141COMNTOOLS')
    VS141VERSION            = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'VS141VERSION')
    WindowsSDKBasePath10    = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'WindowsSDKBasePath10')
    WindowsSDKVersion10     = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'WindowsSDKVersion10')

    VSBasePath              = "#{VS141COMNTOOLS}..\\.."
    VSToolsVersion          = VS141VERSION
    VSBinaryPath            = "#{VSBasePath}\\VC\\Tools\\MSVC\\#{VS141VERSION}\\bin"
    WindowsSDKBasePath      = WindowsSDKBasePath10
    WindowsSDKVersion       = WindowsSDKVersion10

    def defines()
        return [
            'PLATFORM_PC',
            'PLATFORM_WINDOWS',
            'WIN32',
            '__WINDOWS__',
            'TARGET_PLATFORM=Windows'
        ]
    end

    def includePath()
        return super + [
            "#{VSBasePath}\\VC\\Tools\\MSVC\\#{VSToolsVersion}\\include\\*",
            "#{VSBasePath}\\VC\\Auxiliary\\VS\\include\\*",
            "#{WindowsSDKBasePath}\\Include\\#{WindowsSDKVersion}\\ucrt",
            "#{WindowsSDKBasePath}\\Include\\#{WindowsSDKVersion}\\um",
            "#{WindowsSDKBasePath}\\Include\\#{WindowsSDKVersion}\\shared",
        ]
    end

    def compilerPath() return "#{VSBinaryPath}\\HostX64\\x64\\cl.exe" end

    def binaryExtension() return '.exe' end

end #~WindowsPlatform

class Config
    attr_reader     :name,
                    :defines

    def initialize(name, *defines)
        @name = name
        @defines = defines
    end

end #~Platform

PLATFORMS=[
    WindowsPlatform.new('Win32', 'msvc-x64'), # msvc-x86 doesn't exist
    WindowsPlatform.new('Win64', 'msvc-x64'),
    # TODO : add all platforms
]

CONFIGS=[
    Config.new('Debug',     "DEBUG", "_DEBUG"),
    Config.new('FastDebug', "FASTDEBUG", "_DEBUG", "DEBUG"),
    Config.new('Release',   "RELEASE", "NDEBUG"),
    Config.new('Profiling', "RELEASE", "NDEBUG", "PROFILING_ENABLED"),
    Config.new('Final',     "FINAL_RELEASE", "RELEASE", "NDEBUG"),
]

# https://github.com/Microsoft/vscode-cpptools/blob/master/Documentation/LanguageServer/c_cpp_properties.json.md
class C_CPP_Configuration
    C_Standard      = "c11"
    CPP_Standard    = "c++17"

    attr_reader     :platform,
                    :config,
                    :name,
                    :data


    # optional
    attr_accessor   :forcedInclude,
                    :compilerPath,
                    :compileCommands

    def initialize(platform, config)
        @platform = platform
        @config = config
        @name = "#{platform.name}-#{config.name}"
    end

    def export()
        includePath = @platform.includePath
        compileCommands = make_compile_commands(@platform.name, @config.name)
        return {
            'name' => @name,
            'intelliSenseMode' => @platform.intelliSenseMode,
            'includePath' => includePath,
            'macFrameworkPath' => @platform.macFrameworkPath,
            'defines' => @platform.defines + @config.defines,
            'cStandard' => C_Standard,
            'cppStandard' => CPP_Standard,
            'compilerPath' => @platform.compilerPath,
            'compileCommands' => compileCommands,
            'browse' => {
                'path' => includePath,
                'limitSymbolsToIncludedHeaders' => true,
                'databaseFilename' => "${workspaceRoot}/Output/Intermediate/#{@platform.name}/#{@config.name}/vscode.vc.db"
            }
        }
    end

end #~ C_CPP_Configuration

#-----------------------------------------------------------------------------
#   Parse all configurations
#-----------------------------------------------------------------------------

ALL_CONFIGURATIONS = []
PLATFORMS.each do |p|
    CONFIGS.each do |c|
        ALL_CONFIGURATIONS << C_CPP_Configuration.new(p, c)
    end
end

configurations = []
ALL_CONFIGURATIONS.each do |configuration|
    configurations << configuration.export
end

c_cpp_properties = JSON.pretty_generate({
    'version' => VSCODE_C_CPP_PROPERTIES_VERSION,
    'configurations' => configurations,
})
c_cpp_properties.gsub!(CORE_PATH, '${workspaceRoot}')
make_file(VSCODE_C_CPP_PROPERTIES, c_cpp_properties)

#-----------------------------------------------------------------------------
#  Create compilation tasks
#-----------------------------------------------------------------------------

compiletasks = []
ALL_MODULES.each do |target|
    compiletasks << {
        "label": target,
        "type": "shell",
        "isBackground": true,
        "command": "ruby",
        "args": [ "fbuild.rb" ] + FASTBUILD_OPTIONS + [ "#{target}-${input:ppe_config}" ],
        "options": {
            "cwd": '${workspaceRoot}'
        },
        "group": {
            "kind": "build",
            "isDefault": true
        },
        "presentation": {
            "clear": true,
            "echo": true,
            "reveal": "always",
            "focus": false,
            "panel": "dedicated"
        },
        "problemMatcher": [
            "$msCompile"
        ]
    }
end

inputs = [
    {
        "id": "ppe_config",
        "description": "PPE build configuration ?",
        "default": "Win64-Release",
        "type": "pickString",
        "options": ALL_CONFIGURATIONS.collect do |build|
            "#{build.platform.name}-#{build.config.name}"
        end
    }
]

tasks = JSON.pretty_generate({
    'version' => VSCODE_TASKS_VERSION,
    'tasks' => compiletasks,
    'inputs' => inputs
})
make_file(VSCODE_TASKS, tasks)

#-----------------------------------------------------------------------------
#  Create debug launch configurations
#-----------------------------------------------------------------------------

launchconfigurations = [
    {
        "name": DEFAULT_TARGET,
        "type": "cppvsdbg",
        "request": "launch",
        "program": "${workspaceRoot}\\Output\\Binary\\#{DEFAULT_TARGET}.${input:ppe_config}",
        "args": [],
        "stopAtEntry": false,
        "cwd": "${workspaceRoot}\\Output\\Binary",
        "environment": [],
        "visualizerFile": "${workspaceRoot}\\Extras\\Debug\\PPE.natvis",
        "externalConsole": true
    }
]

inputs = [
    {
        "id": "ppe_config",
        "description": "PPE build configuration ?",
        "default": "Release.Win64",
        "type": "pickString",
        "options": ALL_CONFIGURATIONS.collect do |build|
            "#{build.config.name}.#{build.platform.name}.#{build.platform.binaryExtension}"
        end
    }
]

launch = JSON.pretty_generate({
    'version' => VSCODE_LAUNCH_VERSION,
    'configurations' => launchconfigurations,
    'inputs' => inputs
})
make_file(VSCODE_LAUNCH, launch)
