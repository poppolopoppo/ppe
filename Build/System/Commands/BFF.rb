# frozen_string_literal: true

require_once '../Common.rb'

require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'

require_once '../Utils/BFFFile.rb'
require_once '../Utils/Checksum.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

require 'set'

module Build

    persistent_switch(:DeoptimizeWithToken, 'Deoptimize TU when FASTBUILD_DEOPTIMIZE_OBJECT found')
    persistent_switch(:LightCache, 'Experimental cache support using command line analysis')

    make_command(:bff, 'BFF generator') do |&namespace|
        bff_output = Build.bff_output

        source = BFF::Source.new(bff_output.filename, minify: Build.Minify)

        source.comment!("BFF generated by %s v%s", File.basename(Build::Script), VERSION)
        source.func!('Settings') do
            set!('AllowDBMigration_Experimental', true)
            set!('RootPath', $WorkspacePath)
            set!('CachePath', $CachePath) if Build.Cache
        end

        modified_fileslist_path = BFF.modified_fileslist_path

        source.comment!('Global invariants')
        source.set!('CompilerOutputKeepBaseExtension', false)
        source.set!('DeoptimizeWritableFilesWithToken', Build.DeoptimizeWithToken)
        source.set!('LinkerVerboseOutput', true)
        source.set!('UnityInputIsolateListFile', modified_fileslist_path)

        namespace = namespace[] # <=> .call() => evaluate lambda to defer namespace instanciation

        BFF.make_targets(source, Build.get_environments(), namespace)

        source.export_ifn?(bff_output)
    end

    make_persistent_file(:bff_output) do
        ext = File.extname(Build::Script)
        File.join($OutputPath, File.basename(Build::Script, ext) << '.bff')
    end
    make_persistent_file(:modified_fileslist_output) do
        BFF.modified_fileslist_path
    end

    module BFF

        def self.make_targets(bff, environments, namespace)
            platforms = Set.new
            configs = Set.new

            targets = namespace.all.dup
            targets.delete_if{|target| target.headers? }

            environments.each do |envname|
                env = Build.send(envname)
                bff.comment!("Environment <%s>", envname)

                platforms << env.platform
                configs << env.config

                bff.source.scope!() do
                    aliases = targets.collect do |target|
                        target_alias = BFF.make_target_alias(env, target)
                        bff.comment!("Target <%s> '%s'", target.type, target_alias)

                        expanded = env.expand(target)

                        case env.target_artefact_type(target)
                        when :headers
                            Assert.not_implemented
                        when :executable
                            BFF.make_executable(bff, env, target, target_alias, expanded)
                        when :shared
                            BFF.make_shared(bff, env, target, target_alias, expanded)
                        when :library
                            BFF.make_library(bff, env, target, target_alias, expanded)
                        else
                            Log.fatal 'unsupported target type <%s>', target.type
                        end

                        target_alias
                    end

                    aliases.delete_if{|x| x.nil? } # some targets don't produce output

                    bff.comment!("Alias for all targets using <%s>", envname)
                    bff.func!('Alias', env.family) do
                        set!('Targets', aliases)
                    end
                end
            end

            BFF.make_aliases(bff, platforms, configs, targets)
        end

        def self.make_aliases(bff, platforms, configs, targets)
            bff.comment!('Generate aliases for all target-platform-config')

            bff.set!('AllPlatforms', platforms.collect{|x| x.name.to_s })
            bff.set!('AllConfigs', configs.collect{|x| x.name.to_s })
            bff.set!('AllTargets', targets.collect{|x| x.abs_path })

            bff.func!('ForEach', :".target", :in, :".AllTargets") do
                bff.func!('ForEach', :".platform", :in, :".AllPlatforms") do
                    bff.func!('Alias', "$target$-$platform$") do
                        bff.set!('Targets', [], force: true)
                        bff.func!('ForEach', :".config", :in, :".AllConfigs") do
                            bff.append!('Targets', "$target$-$platform$-$config$", parent:true)
                        end
                    end
                end
                bff.func!('ForEach', :".config", :in, :".AllConfigs") do
                    bff.func!('Alias', "$target$-$config$") do
                        bff.set!('Targets', [], force: true)
                        bff.func!('ForEach', :".platform", :in, :".AllPlatforms") do
                            bff.append!('Targets', "$target$-$platform$-$config$", parent:true)
                        end
                    end
                end
                bff.func!('Alias', '$target$') do
                    bff.set!('Targets', [], force: true)
                    bff.func!('ForEach', :".platform", :in, :".AllPlatforms") do
                        bff.func!('ForEach', :".config", :in, :".AllConfigs") do
                            bff.append!('Targets', "$target$-$platform$-$config$", parent:true)
                        end
                    end
                end
            end

            bff.comment!('Default alias')
            bff.func!('Alias', 'All') do
                bff.set!('Targets', '.AllTargets'.to_sym)
            end
        end

        def self.make_target_alias(env, target)
            return "#{target.abs_path}-#{env.family}"
        end

        def self.make_library(bff, env, target, target_alias, expanded)
            artefact = BFF.target_base(bff, env, target, target_alias, expanded)
            bff.func!('Alias', target_alias) do
                set!('Targets', artefact)
            end
        end

        def self.make_executable(bff, env, target, target_alias, expanded)
            BFF.make_deliverable(bff, env, target, target_alias, expanded, 'Executable')
        end
        def self.make_shared(bff, env, target, target_alias, expanded)
            BFF.make_deliverable(bff, env, target, target_alias, expanded, 'DLL')
        end

        def self.make_deliverable(bff, env, target, target_alias, expanded, func)
            artefact = BFF.target_base(bff, env, target, target_alias, expanded)
            bff.func!(func, target_alias) do
                using!(env.compiler.name.to_s+'_Details')
                facet!(expanded, :@linkerOptions)
                set!('LinkerOutput', env.target_artefact_path(target))

                libraries = [ artefact ]
                target.all_private_dependencies do |dep|
                    libraries << BFF.make_target_alias(env, dep)
                end
                target.all_public_dependencies do |dep|
                    libraries << BFF.make_target_alias(env, dep)
                end
                set!('Libraries', libraries)
            end
        end

        def self.target_base(bff, env, target, target_alias, expanded)
            compiler_details = expanded.compiler.name.to_s+'_Details'
            bff.once?(expanded.compiler.name) do
                func!('Compiler', expanded.compiler.name.to_s) do
                    set!('Executable', expanded.compiler.executable)
                    set!('ExtraFiles', expanded.compiler.extra_files)
                    set!('CompilerFamily', expanded.compiler.family.to_s) if expanded.compiler.family
                    set!('UseLightCache_Experimental', Build.LightCache)
                end
                struct!(compiler_details) do
                    set!('Compiler', expanded.compiler.name.to_s)
                    set!('Librarian', expanded.compiler.librarian)
                    set!('Linker', expanded.compiler.linker)
                    set!('CompilerOutputExtension', expanded.compiler.ext_for(:obj))
                end
            end

            target_source = target.var_path+'_Source'

            if Build.PCH and target.pch?
                target_pch_header = target.pch_header
                target_pch_source = target.pch_source
            else
                target_pch_header, target_pch_source = nil
            end

            bff.once?(target_source) do
                bff.comment!('Target source details for <%s>', target.abs_path)

                target_unity = nil
                if Build.Unity and not target.tag?(:nounity) and target.rel_glob_path and target.glob_patterns
                    target_unity = target.var_path+'_Unity'
                    bff.func!('Unity', target_unity) do
                        set!('UnityNumFiles', target.unity_num_files)
                        if target_pch_header
                            set!('UnityPCH', target.rel_pch_header)
                        end
                        glob_path = env.source_path(target.glob_path)
                        set!('UnityInputPath', glob_path)
                        set!('UnityInputPattern', target.glob_patterns)
                        set!('UnityInputExcludedFiles', env.relative_path(glob_path, target.unity_excluded_files))
                        set!('UnityOutputPath', File.join($UnitiesPath, File.dirname(target.abs_path)))
                        set!('UnityOutputPattern', "#{target.name}_*_of_#{target.unity_num_files}.cpp")
                        set!('Hidden', true)
                    end
                end

                bff.struct!(target_source) do
                    set!('CompilerInputFilesRoot', env.source_path(target.source_path))
                    set!('CompilerInputFiles', env.source_path(target.source_files))
                    if target_unity
                        append!('CompilerInputFiles', env.source_path(target.isolated_files))
                        set!('CompilerInputUnity', target_unity)
                    elsif target.rel_glob_path and target.glob_patterns
                        glob_path = env.source_path(target.glob_path)
                        set!('CompilerInputPath', glob_path)
                        set!('CompilerInputPattern', target.glob_patterns)
                        set!('CompilerInputExcludedFiles', env.relative_path(glob_path, target.excluded_files))
                    end
                    if target_pch_source
                        set!('PCHInputFile', env.source_path(target_pch_source))
                    end
                end
            end

            case env.config.link
            when :static
                link_library_objects = target.executable?
            when :dynamic
                link_library_objects = true
            else
                Assert.not_implemented
            end

            if link_library_objects
                artefactFunc = 'ObjectList'
                artefactName = target_alias+'-Obj'
            else
                artefactFunc = 'Library'
                artefactName = target_alias+'-Lib'
            end

            bff.func!(artefactFunc, artefactName) do
                using!(compiler_details)
                using!(target_source)

                set!('CompilerOutputPath', env.intermediate_path(target.abs_path))
                facet!(expanded, :@compilerOptions, :@preprocessorOptions)

                unless link_library_objects
                    set!('LibrarianOutput', env.output_path(target.abs_path, :library))
                    facet!(expanded, :@librarianOptions)
                end

                if target_pch_source
                    set!('PCHOutputFile', env.output_path(target_pch_source, :pch))
                    facet!(expanded, PCHOptions: :@pchOptions)
                end

                set!('Hidden', true)
            end

            return artefactName
        end

    end #~ BFF

end #~ Build