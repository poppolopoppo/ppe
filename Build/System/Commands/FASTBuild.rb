# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

require_once '../Commands/BFF.rb'

require 'fileutils'
require 'open3'

module Build

    opt_array('x-fbuild', 'Pass option directly to FASTBuild')

    make_prerequisite(:FBuild_binary) do
        case Build.os_name
        when :Windows
            need_fileset!(File.join($BuildPath, 'System', 'HAL', 'Windows', 'FBuild.exe'))
        when :Linux
            need_fileset!(File.join($BuildPath, 'System', 'HAL', 'Linux', 'fbuild'))
        else
            Log.fatal 'unsupported os <%s>', os_name
        end
    end

    make_command(:fbuild, 'FASTBuild interop') do |&namespace|
        BFF.write_modified_fileslist()
        FBuild.run(*Build::Args, quiet: (not Log.warning?))
    end

    module FBuild

        def self.run(*args, config: Build.bff_output.filename, quiet: false, wait: true)
            FBuild.prepare_for_build()

            Log.debug 'FBuild: launching "%s"', Build.FBuild_binary

            cmd = []
            cmd << Build.FBuild_binary.to_s
            cmd << '-config' << config
            cmd << '-cache' if Build.Cache
            cmd << '-clean' if Build.Rebuild
            cmd << '-nounity' unless Build.Unity
            cmd << '-nostoponerror' unless Build.StopOnError
            cmd << '-fastcancel' << '-noprogress'

            if quiet
                cmd << '-quiet'
            else
                cmd << '-summary' << '-nosummaryonerror'
            end

            cmd.concat(Build.send('x-fbuild').collect{|x| "-#{x}" })
            cmd.concat(args)

            if (wait && !cmd.include?('-showtargets')) # TODO: https://github.com/fastbuild/fastbuild/issues/719
                cmd << '-wait' << '-ide'
            end

            Log.verbose 'FBuild: %s', cmd.join(' ')

            result = nil
            Open3.popen3(*cmd, chdir: $WorkspacePath) do |io_in, io_out, io_err, wait_thr|
                loop do
                    if line = io_out.gets
                        line = FBuild.trim_crlf(line)
                        if block_given?
                            yield line
                        elsif not quiet and line
                            Log.raw(line, verbosity: :info)
                        end
                    elsif line = io_err.gets
                        line = FBuild.trim_crlf(line)
                        if line
                            Log.raw(line, verbosity: :error)
                        end
                    else
                        break
                    end
                end
                result = wait_thr.value
            end

            if result.success?
                Log.debug 'FBuild: success'
            else
                Log.fatal 'FBuild: command failed (exit code: %d)', result.to_i
            end

            return result.success?
        end

        def self.trim_crlf(str)
            str.chomp!
            str.rstrip!
            str.empty? ? nil : str
        end

        @@_build_prepared = false
        def self.prepare_for_build()
            unless @@_build_prepared
                @@_build_prepared = true
                verbose = Log.verbose?
                FileUtils.mkdir_p($BinariesPath, verbose: verbose) unless Dir.exist?($BinariesPath)
                FileUtils.mkdir_p($IntermediatePath, verbose: verbose) unless Dir.exist?($IntermediatePath)
                FileUtils.mkdir_p($ProjectsPath, verbose: verbose) unless Dir.exist?($ProjectsPath)
                FileUtils.mkdir_p($UnitiesPath, verbose: verbose) unless Dir.exist?($UnitiesPath)
            end
        end

    end #~ FBuild

end #~ Build