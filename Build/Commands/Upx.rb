# frozen_string_literal: true

require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'

require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    make_command(:upx, 'Compressed specified target executable') do |&namespace|
        Upx.targets(*Build::Args)
    end

    module Upx

        def self.binary_path()
            return File.join(Build.hal_path, 'upx' + Build.environment_compiler().ext_binary)
        end

        def self.compress(path)
            args = [ Upx.binary_path, '-9', '-k', path ]
            args << '-v' if Log.verbose?
            pid = spawn(*args, chdir: $BinariesPath)
            Process.waitpid(pid)
        end

        def self.targets(*targets, use_debugger: false)
            compiler = Build.environment_compiler()
            targets.each do |target|
                bin = Environment.executable_path(target, nil, compiler.ext_binary)
                if File.exist?(bin)
                    Log.log("Compressing <#{target}> executable '#{bin}'")
                    Upx.compress(bin)
                else
                    Log.error("Can't compress <#{target}> since '#{bin}' executable does not exist")
                end
            end
        end

    end #~ Upx

end #~ Build
