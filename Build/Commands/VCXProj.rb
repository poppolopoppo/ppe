# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/BFFFile.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

require_once '../Commands/BFF.rb'

require 'fileutils'

module Build

    make_command(:vcxproj, 'Generate VisualStudio project files') do |&namespace|
        environments = Build.fetch_environments
        namespace = namespace[]

        bff = BFF::Source.new(File.join($OutputPath, 'VCXProj.bff'))

        environments.each do |env|
            namespace.all.each do |target|
                expanded = env.expand(target)
                VCXProj.make_vcxconfig(bff, env, target, expanded)
            end
        end

        VCXProj.make_sln(bff, environments, namespace)

        bff.write_to_disk

        FBuild.run('sln', config: bff.filename)
    end

    module VCXProj

        def self.solution_platform(env)
            case env.platform.arch
            when :x86
                return 'Win32'
            when :x64
                return 'x64'
            else
                Log.fatal 'unsupported platform architecture <%s>', env.platform.arch
            end
        end

        def self.make_vcxconfig(bff, env, target, expanded)
            target_alias = "#{target.abs_path}-#{env.family}"
            bff.struct!("#{target.var_path}_#{env.varname}_VCXConfig") do
                artifact = env.target_artifact_path(target)
                intermediate = env.intermediate_path(target.abs_path)

                set!('Platform', VCXProj.solution_platform(env))
                set!('Config', env.config.name.to_s)

                set!('PlatformToolset', "v#{env.compiler.platformToolset}")

                set!('IntermediateDirectory', intermediate)
                set!('BuildLogFile', File.join(intermediate, 'Build.log'))

                if artifact
                    set!('Output', artifact)
                    set!('OutputDirectory', File.dirname(artifact))

                    set!('ProjectBuildCommand', Build.make_commandstr('--fbuild', target_alias, '-v'))
                    set!('ProjectRebuildCommand', Build.make_commandstr('--fbuild', target_alias, '--Rebuild', '-v'))
                    set!('ProjectCleanCommand', Build.make_commandstr('--dist-clean', '-v', target_alias, "#{env.platform.name}/#{env.config.name}/#{target.abs_path}"))
                end

                set!('AdditionalOptions', expanded.analysisOptions.join(' '))
                set!('ForcedIncludes', expanded.includes.join(';'))
                set!('IncludeSearchPath', expanded.any_includePaths.join(';'))
                set!('PreprocessorDefinitions', expanded.defines.join(';'))
            end
            return target_alias
        end

        def self.make_sln(bff, environments, namespace)
            solutionOutput = File.join($OutputPath, namespace.name.to_s + '.sln')

            buildProjects = []
            solutionFolders = {}

            vcxprojects = namespace.all.collect do |target|
                targetVcxproject = "#{target.abs_path}-VCXProject"

                bff.func!('VCXProject', targetVcxproject) do
                    set!('ProjectConfigs', environments.collect{|env| :".#{target.var_path}_#{env.varname}_VCXConfig" })
                    set!('ProjectOutput', File.join($ProjectsPath, target.abs_path + '.vcxproj'))
                    set!('ProjectBasePath', File.join($SourcePath, target.abs_path))
                    set!('ProjectInputPaths', File.join($SourcePath, target.abs_path))
                    set!('ProjectAllowedFileExtensions', target.glob_patterns + %w{ *.h *.rb *.rc })
                    set!('ProjectFilesToExclude', target.excluded_files.to_a.collect{|x| File.join($SourcePath, x) })
                    set!('ProjectFiles', target.all_source_files.to_a.collect{|x| File.join($SourcePath, x) }, force: true)
                    target.all_units do |unit|
                        append!('ProjectFiles', unit.source_files.to_a.collect{|x| File.join($SourcePath, x) })
                    end
                end

                solutionFolder = solutionFolders[target.namespace]
                solutionFolder = solutionFolders[target.namespace] = [] if solutionFolder.nil?
                solutionFolder << targetVcxproject

                buildProjects << targetVcxproject unless target.headers? #if target.executable?

                targetVcxproject
            end

            bff.func!('VCXProject', 'Build-VCXProject') do
                platformToolset = nil
                projectConfigs = environments.collect do |env|
                    projectConfig = 'ProjectConfig_'+env.varname
                    platformToolset = env.compiler.platformToolset
                    struct!(projectConfig) do
                        set!('Platform', VCXProj.solution_platform(env))
                        set!('Config', env.config.name.to_s)
                    end
                    ('.'+projectConfig).to_sym
                end

                set!('PlatformToolset', "v#{platformToolset}") if platformToolset
                set!('ProjectOutput', File.join($ProjectsPath, 'Build.vcxproj'))
                set!('ProjectBasePath', $BuildPath)
                set!('ProjectInputPaths', $BuildPath)
                set!('ProjectAllowedFileExtensions', %w{ *.exe *.rb *.yml })
                set!('ProjectConfigs', projectConfigs)
                set!('ProjectFiles', [
                    # Include startup script
                    Build::Script,
                    # VS will include automatically every .natvis referenced in the solution
                    File.join($ExtrasPath, 'Debug', 'PPE.natvis') ])

                # meta commands used to regen the solution or delete all generated files
                set!('ProjectBuildCommand', Build.make_commandstr('--vcxproj', '-v'))
                set!('ProjectRebuildCommand', Build.make_commandstr('--bff', '--vcxproj', '-v', '--clean', '--Rebuild'))
                set!('ProjectCleanCommand', Build.make_commandstr('--dist-clean', '-v', '--clean'))
            end
            vcxprojects << 'Build-VCXProject'

            bff.func!('VSSolution', 'sln') do
                solutionConfigs = environments.collect do |env|
                    solutionConfig = 'SolutionConfig_'+env.varname
                    struct!(solutionConfig) do
                        set!('Platform', VCXProj.solution_platform(env))
                        set!('Config', env.config.name.to_s)
                    end
                    ('.'+solutionConfig).to_sym
                end

                solutionFolders = solutionFolders.keys.collect do |namespace|
                    solutionFolder = 'SolutionFolder_'+namespace.var_path
                    bff.struct!(solutionFolder) do
                        set!('Path', namespace.path)
                        set!('Projects', solutionFolders[namespace])
                    end
                    ('.'+solutionFolder).to_sym
                end

                set!('SolutionOutput', solutionOutput)
                set!('SolutionProjects', vcxprojects)
                set!('SolutionConfigs', solutionConfigs)
                set!('SolutionFolders', solutionFolders)
                set!('SolutionBuildProject', buildProjects)
            end
        end

    end #~ VCXProj

end #~ Build