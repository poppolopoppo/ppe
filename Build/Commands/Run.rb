# frozen_string_literal: true

require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'

require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    make_command(:run, 'Run specified target') do |&namespace|
        Run.targets(*Build::Args, use_debugger: false)
    end
    make_command(:rundbg, 'Run and debug specified target') do |&namespace|
        Run.targets(*Build::Args, use_debugger: true)
    end

    module Run

        def self.inject_debugger(args)
            if Build.os_windows?
                args.insert(0, 'vsjitdebugger.exe')
            end
        end

        def self.targets(*targets, use_debugger: false)
            extraArgs = []
            compiler = Build.environment_compiler()
            targets.each do |target|
                bin = Environment.executable_path(target, nil, compiler.ext_binary)
                if File.exist?(bin)
                    Log.log("Running <#{target}> with #{bin}")
                    args = [ bin, *extraArgs ]
                    args = Run.inject_debugger(args) if use_debugger
                    pid = spawn(*args, chdir: $BinariesPath)
                    Process.waitpid(pid)
                else
                    Log.error("Can't run <#{target}> since #{bin} does not exist")
                end
            end
        end

    end #~ Run

end #~ Build
