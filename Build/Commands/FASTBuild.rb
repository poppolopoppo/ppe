# frozen_string_literal: true


require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

require_once '../Commands/BFF.rb'

require 'fileutils'

module Build

    opt_array('x-fbuild', 'Pass option directly to FASTBuild')

    make_prerequisite(:FBuild_binary) do
        case Build.os_name
        when :Windows
            need_fileset!(File.join($BuildPath, 'HAL', 'Windows', 'FBuild.exe'))
        when :Linux
            need_fileset!(File.join($BuildPath, 'HAL', 'Linux', 'fbuild'))
        else
            Log.fatal 'unsupported os <%s>', os_name
        end
    end

    make_command(:fbuild, 'FASTBuild interop') do |&namespace|
        BFF.write_modified_fileslist()
        FBuild.run(*Build::Args, quiet: (not Log.warning?))
    end

    module FBuild

        def self.run(*args, config: Build.bff_output.filename, quiet: false, wait: true, &block)
            FBuild.prepare_for_build()

            Log.debug 'FBuild: launching "%s"', Build.FBuild_binary

            cmd = []
            cmd << Build.FBuild_binary.to_s
            cmd << '-config' << config
            cmd << '-clean' if Build.Rebuild
            cmd << '-nounity' unless Build.Unity
            cmd << '-nostoponerror' unless Build.StopOnError
            #cmd << '-fastcancel' # EXPERIMENTAL: crashing and PDB errors

            if Build.interactive
                cmd << '-progress'
            else
                cmd << '-noprogress'
            end

            if quiet
                cmd << '-quiet'
            else
                cmd << '-summary' << '-nosummaryonerror'
            end

            if (wait && !cmd.include?('-showtargets')) # TODO: https://github.com/fastbuild/fastbuild/issues/719
                cmd << '-wait' << '-ide'
            end

            if Build.Diagnose
                cmd << '-j1' # limit to one worker thread, disable cache and distribution
                cmd << '-report' # emit a report.html after building
            else
                cmd << '-cache' if Build.Cache  # compilation cache
                cmd << '-dist'  if Build.Dist   # compilation distribution

                cmd << "-j#{Build.WorkerCount}" if Build.WorkerCount.to_i > 0
            end

            cmd.concat(Build.send('x-fbuild').collect{|x| "-#{x}" })
            cmd.concat(args)

            env = {
                'FASTBUILD_CACHE_PATH' => $CachePath,
                'FASTBUILD_TEMP_PATH' => $TemporaryPath,
            }

            result = Process.start(env, *cmd, chdir: $WorkspacePath, quiet: quiet, &block)

            if result.success?
                Log.debug 'FBuild: success'
            else
                Log.fatal 'FBuild: command failed (exit code: %d)', result.to_i
            end

            return result.success?
        end

        @@_build_prepared = false
        def self.prepare_for_build()
            unless @@_build_prepared
                @@_build_prepared = true
                verbose = Log.debug?
                FileUtils.mkdir_p($BinariesPath, verbose: verbose) unless DirCache.exist?($BinariesPath)
                FileUtils.mkdir_p($GeneratedPath, verbose: verbose) unless DirCache.exist?($GeneratedPath)
                FileUtils.mkdir_p($IntermediatePath, verbose: verbose) unless DirCache.exist?($IntermediatePath)
                FileUtils.mkdir_p($ProjectsPath, verbose: verbose) unless DirCache.exist?($ProjectsPath)
                FileUtils.mkdir_p($UnitiesPath, verbose: verbose) unless DirCache.exist?($UnitiesPath)
                FileUtils.mkdir_p($TemporaryPath, verbose: verbose) unless DirCache.exist?($TemporaryPath)
            end
        end

    end #~ FBuild

end #~ Build
