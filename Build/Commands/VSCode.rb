# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/JSONFile.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

require_once 'BFF.rb'
require_once 'FASTBuild.rb'

require 'fileutils'

module Build

    make_command(:vscode, 'Generate VisualStudio Code bindings') do |&namespace|
        unless File.exist?(Build.bff_output.filename)
            Log.warning 'VSCode: could not find BFF source "%s"', Build.bff_output.filename
            Log.fatal 'VSCode: BFF source not present, you should probably run --bff before'
        end

        vscode = Build.vscode_path

        environments = Build.fetch_environments
        targets = namespace[].all

        compiler = environments.first.compiler

        JSONFile.serialize(File.join(vscode, VSCode::C_CPP_PROPERTIES), {
            version: VSCode::C_CPP_PROPERTIES_VERSION,
            configurations: environments.collect{|x| VSCode.c_cpp_properties(x, targets) }
        })

        JSONFile.serialize(File.join(vscode, VSCode::TASKS), {
            version: VSCode::TASKS_VERSION,
            tasks: VSCode.tasks(targets)
        })

        JSONFile.serialize(File.join(vscode, VSCode::LAUNCH), {
            version: VSCode::LAUNCH_VERSION,
            configurations: VSCode.launch_configs(compiler, targets)
        })
    end

    def self.vscode_path() return File.join($WorkspacePath, VSCode::PATH) end

    module VSCode
        PATH='.vscode'
        C_STANDARD='c11'
        C_CPP_PROPERTIES_VERSION=4
        C_CPP_PROPERTIES='c_cpp_properties.json'
        TASKS='tasks.json'
        TASKS_VERSION='2.0.0'
        LAUNCH='launch.json'
        LAUNCH_VERSION='0.2.0'
        VCDB='vscode-vc.db'
        NATVIS=File.join($ExtrasPath, 'Debug', 'PPE.natvis')

        def self.c_cpp_properties(env, targets)
            compdb = env.intermediate_path('compile_commands.json')
            VSCode.compdb(compdb, env)

            globalIncludePaths = [ env.source_path('.') ]
            globalIncludePaths.concat(env.facet.any_includePaths)

            targets.each do |m|
                globalIncludePaths << env.source_path(m.source_path)

                public_path = env.source_path(m.public_path)
                globalIncludePaths << public_path if Dir.exist?(public_path)

                private_path = env.source_path(m.private_path)
                globalIncludePaths << private_path if Dir.exist?(private_path)
            end

            globalIncludePaths.uniq!

            case env.platform.os
            when :Windows
                intelliSenseMode = 'msvc-x64'
                macFrameworkPath = []
            when :Linux
                intelliSenseMode = 'clang-x64'
                macFrameworkPath = []
            else
                Log.fatal 'unsupported os <%s>', env.platform.os
            end

            return {
                name: env.family,
                includePath: globalIncludePaths,
                intelliSenseMode: intelliSenseMode,
                macFrameworkPath: macFrameworkPath,
                defines: env.facet.defines.data,
                cStandard: C_STANDARD,
                cppStandard: Build.CppStd,
                compilerPath: env.compiler.executable,
                compileCommands: compdb,
                browse: {
                    path: globalIncludePaths,
                    limitSymbolsToIncludedHeaders: true,
                    databaseFilename: env.intermediate_path(VCDB),
                }
            }
        end

        def self.tasks(targets)
            targets.collect do |m|
                args = Build.make_commandline('--fbuild', "#{m.abs_path}-${command:cpptools.activeConfigName}")
                {
                    label: m.abs_path,
                    command: args.shift,
                    args: args,
                    options: { cwd: $WorkspacePath },
                    group: { kind: 'build', isDefault: true },
                    presentation: {
                        clear: true,
                        echo: true,
                        reveal: 'always',
                        focus: false,
                        panel: 'dedicated'
                    },
                    problemMatcher: [ "$msCompile" ]
                }
            end
        end

        def self.launch_configs(compiler, targets)
            extname = compiler.ext_for(:executable)
            executables = targets.select{|x| x.executable? }
            executables.collect! do |m|
                {
                    name: m.abs_path,
                    type: 'cppvsdbg',
                    request: 'launch',
                    program: Environment.executable_path(m.abs_path, "${command:cpptools.activeConfigName}", extname),
                    args: [],
                    stopAtEntry: false,
                    cwd: $BinariesPath,
                    environment: [],
                    #visualizeFile: NATVIS,
                    externalConsole: false
                }
            end
        end

        def self.compdb(filename, env)
            compdb = File.join($WorkspacePath, 'compile_commands.json')
            Log.debug 'VSCode: generate <%s> compdb in "%s"', env.family, compdb
            FBuild.run('-compdb', '-nounity', env.family, quiet: !Log.debug?, wait: false)
            if File.exist?(compdb)
                Log.log 'VSCode: move compdb to "%s"', filename
                dirname = File.dirname(filename)
                FileUtils.mkdir_p(dirname) unless Dir.exist?(dirname)
                FileUtils.mv(compdb, filename)
            else
                Log.fatal 'VSCode: compdb "%s" not found, generation failed ?', compdb
            end
        end

    end #~ VSCode

end #~ Build
