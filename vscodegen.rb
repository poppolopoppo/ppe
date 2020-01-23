#!/usr/bin/env ruby

require 'fileutils'
require 'json'

ROOT_PATH=File.dirname(File.absolute_path(__FILE__))

require "#{ROOT_PATH}/Build/BFF.rb"
require "#{ROOT_PATH}/Build/OS.rb"

USE_CPPTOOLS_ACTIVECONFIGNAME = true
USE_MULTIPLE_WORKSPACE=ARGV.collect{|v|v.upcase}.include?('-MULTI')

FBUILD_RB=File.join(ROOT_PATH, 'fbuild.rb')
FBUILD_COMPDB=File.join(ROOT_PATH, 'compile_commands.json')
FBUILD_SOLUTION_PATH=File.join(ROOT_PATH, 'Build', '_solution_path.bff')

system('ruby', FBUILD_RB, '-version') # make sure _solution_path.bff is generated
raise "invalid solution path" unless File.exist?(FBUILD_SOLUTION_PATH)

def fetch_fbuilb_strings(filename, *keys)
    bff = File.read(filename)
    return keys.collect do |key|
        m = bff.match(/\.#{key}\s*=\s*['"]([^'"]*?)['"]/)
        m.nil? ? nil : m[1]
    end
end

def fetch_fbuilb_string(filename, key)
    return fetch_fbuilb_strings(filename, key)[0]
end

def fetch_all_modules()
    modules = []
    rootpath = File.join(ROOT_PATH, 'Source')
    Dir.glob(File.join(rootpath, '**', '*.bff')).each do |bff|
        relpath = File.dirname(File.dirname(bff[(rootpath.length+1)..bff.length]))
        name, type  = *fetch_fbuilb_strings(bff, 'ModuleName', 'ModuleType')
        modules << Module.new(name, type, relpath, bff) unless name.nil?
    end
    return modules.sort do |a, b|
        a.target <=> b.target
    end
end

def fetch_default_target()
    target = fetch_fbuilb_string(File.join(ROOT_PATH, 'fbuild.bff'), 'SolutionBuildProject')
    target.gsub!(/-VCXProject$/, '')
    return target
end

def fetch_fastbuild_options()
    opts = fetch_fbuilb_string(File.join(ROOT_PATH, 'Build', 'VisualStudio.bff'), 'FastBuildOptions')
    opts = opts.strip.split(/\s+/)
    return opts
end

def make_compile_commands(relpath, platform, config, modules)
    dirname = File.join(ROOT_PATH, 'Output', 'Intermediate', platform, config, relpath)
    FileUtils.mkdir_p(dirname)
    filename = File.join(dirname, "compile_commands.json")
    targets = modules.collect{|name| "#{name}-#{platform}-#{config}" }
    unless system('ruby', FBUILD_RB, "-compdb", *targets, :out => File::NULL)
        raise "#{FBUILD_RB} -compdb #{targets.join(' ')}: compdb generation failed :'("
    end
    FileUtils.mv(FBUILD_COMPDB, filename)
    puts " + #{filename}"
    return filename
end

def make_json_file(filename, content)
    puts " < #{filename}"
    File.write(filename, content)
end

class Platform
    attr_reader     :name,
                    :intelliSenseMode
                    :arch

    def initialize(name, intelliSenseMode, arch)
        @name = name
        @arch = arch
        @intelliSenseMode = intelliSenseMode
    end

    def defines()
        case @arch
        when :x86 then [ 'ARCH_X86', 'ARCH_32BIT' ]
        when :x64 then [ 'ARCH_X64', 'ARCH_64BIT' ]
        else raise 'invalid arch'
        end
    end
    def includePath() return [ "#{ROOT_PATH}/Source/**" ] end
    def compilerPath() return '' end
    def macFrameworkPath() return [] end
    def binaryExtension() return '' end

    def to_s() @name; end

end #~Platform

class LinuxPlatform < Platform

    def defines()
        return super + [
            'PLATFORM_PC',
            'PLATFORM_LINUX',
            'PLATFORM_POSIX',
            'TARGET_PLATFORM=Linux',
            '__LINUX__'
        ]
    end

    def includePath()
        return super
    end

    def compilerPath() return "/usr/bin/clang++ #{@name == 'Linux32' ? '-m32' : '-m64'}" end

    def binaryExtension() return '.bin' end

end #~ LinuxPlatform

class WindowsPlatform < Platform
    VS141COMNTOOLS          = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'VS141COMNTOOLS')
    VS141VERSION            = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'VS141VERSION')

    VS142COMNTOOLS          = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'VS142COMNTOOLS')
    VS142VERSION            = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'VS142VERSION')

    WindowsSDKBasePath10    = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'WindowsSDKBasePath10')
    WindowsSDKVersion10     = fetch_fbuilb_string(FBUILD_SOLUTION_PATH, 'WindowsSDKVersion10')

    USE_VS2019              = true
    VSCOMNTOOLS             = USE_VS2019 ? VS142COMNTOOLS : VS141COMNTOOLS
    VSVERSION               = USE_VS2019 ? VS142VERSION   : VS141VERSION

    VSBasePath              = "#{VSCOMNTOOLS}../.."
    VSToolsVersion          = VSVERSION
    VSBinaryPath            = "#{VSBasePath}/VC/Tools/MSVC/#{VSVERSION}/bin"

    puts "Using VS found in '#{VSBinaryPath}'"

    WindowsSDKBasePath      = WindowsSDKBasePath10
    WindowsSDKVersion       = WindowsSDKVersion10

    def defines()
        return super + [
            'PLATFORM_PC',
            'PLATFORM_WINDOWS',
            'TARGET_PLATFORM=Windows',
            '__WINDOWS__',
            'WIN32',
        ]
    end

    def includePath()
        return super + [
            "#{VSBasePath}/VC/Tools/MSVC/#{VSToolsVersion}/include/**",
            "#{VSBasePath}/VC/Auxiliary/VS/include/**",
            "#{WindowsSDKBasePath}/Include/#{WindowsSDKVersion}/ucrt/**",
            "#{WindowsSDKBasePath}/Include/#{WindowsSDKVersion}/um/**",
            "#{WindowsSDKBasePath}/Include/#{WindowsSDKVersion}/shared/**",
        ]
    end

    def compilerPath() return "\"#{VSBinaryPath}/HostX64/x64/cl.exe\" #{@name == 'Win32' ? '-m32' : '-m64'}" end

    def binaryExtension() return '.exe' end

end #~WindowsPlatform

class Config
    attr_reader     :name,
                    :defines

    def initialize(name, *defines)
        @name = name
        @defines = defines
    end

    def to_s() @name; end

end #~Config

class Module
    attr_reader     :name,
                    :type,
                    :relpath,
                    :bff,
                    :data

    def initialize(name, type, relpath, bff)
        @name = name
        @type = type ? type.to_sym : :Library
        @relpath = relpath
        @bff = bff
        @data = BFF.parse(@bff)
        raise "invalid module parsing" if @data.length != 1
        @data = @data[0].inner[0]
    end

    def [](name) @data[name] end

    def rel_root() File.join(@relpath, @name) end
    def root() File.join(ROOT_PATH, rel_root) end

    def source_path() File.dirname(@bff) end
    def public_path() "#{source_path}/Public" end
    def private_path() "#{source_path}/Private" end

    def local_defines()
        result = @data['ModuleDefines']
        result ? result : []
    end

    def forced_includes()
        result = @data['ModuleForcedIncludes']
        result ? result : []
    end

    def dependencies(&block)
        @data.ModulePublicDependencies.each(&block) if @data['ModulePublicDependencies']
        @data.ModulePrivateDependencies.each(&block) if @data['ModulePrivateDependencies']
        return self
    end

    def include_paths()
        result = []
        [public_path, private_path].each do |p|
            result << p if Dir.exist?(p)
        end
        return result
    end

    def target() "#{@relpath}/#{@name}" end
    def to_s() self.target end

    def desc()
        "[#{@type}] #{self.target} '#{@relpath}'"
    end

end #~ Module

class Target
    attr_reader :name
    attr_reader :platforms, :configs, :modules
    attr_reader :cpp_configs

    def initialize(name)
        @name = name
        @platforms = []
        @configs = []
        @modules = []
    end

    def make_cpp_configs!
        result = []
        @platforms.each do |p|
            @configs.each{|c| result << C_CPP_Configuration.new(p, c) }
        end
        return (@cpp_configs = result)
    end

end #~ Target

class Workspace
    VSCODE='.vscode'
    C_CPP_PROPERTIES_VERSION=4
    C_CPP_PROPERTIES='c_cpp_properties.json'
    TASKS='tasks.json'
    TASKS_VERSION='2.0.0'
    LAUNCH='launch.json'
    LAUNCH_VERSION='0.2.0'
    VCDB='vc.db'

    attr_reader :root, :relpath, :target
    attr_reader :modules, :default, :vscode
    attr_reader :c_cpp_properties, :launch, :tasks, :vcdb

    def initialize(root, relpath, target, modules, default=nil)
        @root = root
        @relpath = relpath
        @target = target
        @modules = modules

        @default = default
        self.executables do |m|
            @default = m.name
            break
        end if @default.nil?

        @vscode = File.join(@root, VSCODE)
        @tasks = File.join(@vscode, TASKS)
        @launch = File.join(@vscode, LAUNCH)
        @vcdb = File.join(@vscode, VCDB)
        @c_cpp_properties = File.join(@vscode, C_CPP_PROPERTIES)
    end

    def each_modules(mtype=nil, &block)
        @modules.each do |m|
            block.call(m) if mtype.nil? || m.type == mtype
        end
    end

    def externals(&block) self.each_modules(:External, &block) end
    def libraries(&block) self.each_modules(:Library, &block) end
    def executables(&block) self.each_modules(:Executable, &block) end

    def each_files(&block)
        [@c_cpp_properties, @launch, @tasks, @vcdb].each(&block)
    end

    def export(all_configs)
        FileUtils.mkdir_p(File.dirname(@vcdb), :verbose => true)
        write__c_cpp_properties(all_configs)
        write__tasks(all_configs)
        write__launch(all_configs)
    end

    def cleanup()
        each_files do |fname|
            FileUtils.rm(fname, :version => true)
        end
    end

private

    def write__c_cpp_properties(all_configs)
        expanded_configs = []
        all_configs.each do |cfg|
            expanded_configs << cfg.export(@relpath, @modules)
        end

        json = JSON.pretty_generate({
            'version' => C_CPP_PROPERTIES_VERSION,
            'configurations' => expanded_configs,
        })

        make_json_file(@c_cpp_properties, json)
    end

    def write__tasks(all_configs)
        compiletasks = []
        @modules.each do |m|
            compiletasks << {
                "label": m.target,
                "type": "shell",
                "isBackground": true,
                "command": "ruby",
                "args": USE_CPPTOOLS_ACTIVECONFIGNAME ?
                    [ FBUILD_RB ] + FASTBUILD_OPTIONS + [ "#{m.target}-${command:cpptools.activeConfigName}" ] :
                    [ FBUILD_RB ] + FASTBUILD_OPTIONS + [ "#{m.target}-${input:ppe_config}" ],
                "options": {
                    "cwd": ROOT_PATH
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

        inputs = USE_CPPTOOLS_ACTIVECONFIGNAME ? [] : [
            {
                "id": "ppe_config",
                "description": "PPE build configuration ?",
                "default": "Win64-Release",
                "type": "pickString",
                "options": all_configs.collect do |build|
                    "#{build.platform.name}-#{build.config.name}"
                end
            }
        ]

        json = JSON.pretty_generate({
            'version' => TASKS_VERSION,
            'tasks' => compiletasks,
            'inputs' => inputs
        })

        make_json_file(@tasks, json)
    end

    def write__launch(all_configs)
        launch_configs = []
        self.executables do |m|
            launch_configs << {
                "name": m.target,
                "type": "cppvsdbg",
                "request": "launch",
                "program": USE_CPPTOOLS_ACTIVECONFIGNAME ?
                    "#{ROOT_PATH}/Output/Binary/#{File.basename(m.name)}-${command:cpptools.activeConfigName}" :
                    "#{ROOT_PATH}/Output/Binary/#{File.basename(m.name)}-${input:ppe_config}",
                "args": [],
                "stopAtEntry": false,
                "cwd": "#{ROOT_PATH}/Output/Binary",
                "environment": [],
                "visualizerFile": "#{ROOT_PATH}/Extras/Debug/PPE.natvis",
                "externalConsole": false
            }
        end

        inputs = USE_CPPTOOLS_ACTIVECONFIGNAME ? [] : [
            {
                "id": "ppe_config",
                "description": "PPE build configuration ?",
                "default": "Release.Win64",
                "type": "pickString",
                "options": all_configs.collect do |build|
                    "#{build.config.name}.#{build.platform.name}#{build.platform.binaryExtension}"
                end
            }
        ]

        json = JSON.pretty_generate({
            'version' => LAUNCH_VERSION,
            'configurations' => launch_configs,
            'inputs' => inputs
        })

        make_json_file(@launch, json)
    end

end #~ Workspace

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

    def export(relpath, modules)
        includePath = @platform.includePath
        compileCommands = make_compile_commands(relpath, @platform.name, @config.name, modules)

        defines = @platform.defines + @config.defines

        # greedy fallback for includes since this is a global state (!= per module)
        # and headers don't benefit from compile_commands.json goodness :(
        modules.each do |m|
            includePath.concat(m.include_paths)
            if modules.length == 1
                defines.concat(m.local_defines)
                local_includes = []
                m.dependencies do |relpath|
                    local_includes << "#{ROOT_PATH}/Source/#{relpath}/Public"
                end
                includePath.concat(local_includes)
            end
        end

        return {
            'name' => @name,
            'intelliSenseMode' => @platform.intelliSenseMode,
            'includePath' => includePath,
            'macFrameworkPath' => @platform.macFrameworkPath,
            'defines' => defines,
            'cStandard' => C_Standard,
            'cppStandard' => CPP_Standard,
            'compilerPath' => @platform.compilerPath,
            'compileCommands' => compileCommands,
            'browse' => {
                'path' => includePath,
                'limitSymbolsToIncludedHeaders' => true,
                'databaseFilename' => "#{ROOT_PATH}/Output/Intermediate/#{@platform.name}/#{@config.name}/#{relpath}/vscode.vc.db"
            }
        }
    end

end #~ C_CPP_Configuration

#-----------------------------------------------------------------------------
#   Fetch all platforms and compilation configs
#-----------------------------------------------------------------------------

# this is hard-coded atm :'(
# ----TODO: use/add a json export feature inside fastbuild---
# TODO: Nope, move logic out of bff and use ruby/python instead!

PPE = Target.new('PPE')

if OS.windows?
    PPE.platforms << WindowsPlatform.new('Win32', 'msvc-x64', :x86) # msvc-x86 doesn't exist
    PPE.platforms << WindowsPlatform.new('Win64', 'msvc-x64', :x64)
elsif OS.linux?
    PPE.platforms << LinuxPlatform.new('Linux32', 'clang-x64', :x86)
    PPE.platforms << LinuxPlatform.new('Linux64', 'clang-x64', :x64)
else
    raise SystemError.new('invalid platform')
end

PPE.configs << Config.new('Debug',     "DEBUG", "_DEBUG")
PPE.configs << Config.new('FastDebug', "FASTDEBUG", "_DEBUG", "DEBUG")
PPE.configs << Config.new('Release',   "RELEASE", "NDEBUG")
PPE.configs << Config.new('Profiling', "RELEASE", "NDEBUG", "PROFILING_ENABLED")
PPE.configs << Config.new('Final',     "FINAL_RELEASE", "RELEASE", "NDEBUG")

#-----------------------------------------------------------------------------
#   Parse all modules from BFF
#-----------------------------------------------------------------------------

PPE.modules.concat(fetch_all_modules())

# also need some additional settings

DEFAULT_TARGET=fetch_default_target()
FASTBUILD_OPTIONS=fetch_fastbuild_options()

puts "[Default target]      = <#{DEFAULT_TARGET}>"
puts "[Fastbuild options]   = #{FASTBUILD_OPTIONS}"
puts "[All platforms]       ="
puts " - #{PPE.platforms.join(",\n - ")}"
puts "[All configs]         ="
puts " - #{PPE.configs.join(",\n - ")}"
puts "[All modules]         ="
puts " - #{PPE.modules.collect{|m| m.desc}.join(",\n - ")}"

#-----------------------------------------------------------------------------
#   Parse all configurations
#-----------------------------------------------------------------------------

PPE.make_cpp_configs!

#-----------------------------------------------------------------------------
#   Finally export the workspace(s)
#-----------------------------------------------------------------------------

workspaces=[]

if USE_MULTIPLE_WORKSPACE
    PPE.modules.each do |m|
        workspaces << Workspace.new(File.join(ROOT_PATH, 'Source', m.rel_root), m.rel_root, PPE, [m])
    end

    make_json_file(File.join(ROOT_PATH, 'Output', 'ppe.code-workspace'),
        JSON.pretty_generate({
            "folders": workspaces.collect do |w|
                {
                    "name": w.relpath,
                    "path": w.root
                }
            end + [
                {
                    "name": 'Build',
                    "path": File.join(ROOT_PATH, 'Build')
                },
                {
                    "name": 'Doc',
                    "path": File.join(ROOT_PATH, 'Doc')
                },
                {
                    "name": 'Extras',
                    "path": File.join(ROOT_PATH, 'Extras')
                }
            ]
        }))
else
    workspaces << Workspace.new(ROOT_PATH, '.', PPE, PPE.modules, DEFAULT_TARGET)
end

workspaces.each do |w|
    puts "Preparing workspace #{w.root}..."
    w.export(PPE.cpp_configs)
end
