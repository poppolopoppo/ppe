# frozen_string_literal: true


require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    make_command(:'ps1', 'Generate powershell completion script') do |&namespace|
        environments = Build.fetch_environments
        targets = namespace[].select(*Build::Args)

        aliases = []
        Build.each_build_aliases(environments, targets) do |alias_name, alias_targets|
            aliases << alias_name
        end

        interceptor = PS1::OptionInterceptor.new
        Build.make_pgm_options(interceptor)

        ext = File.extname(Build::Script)
        script = File.basename(Build::Script, ext)

        interceptor.opts.each do |flag, opt|
            Log.log('PS1: found option "%s" -> %s', flag, opt.desc)
        end
        aliases.each do |target|
            Log.log('PS1: found alias <%s>', target)
        end

        completion = File.join($ExtrasPath, "#{script}-#{Build.os_name}-completion.ps1")
        Log.info('PS1: export completion config to "%s"', completion)
        Log.verbose('PS1: generating PowerShell completion script with %d aliases and %s options', aliases.length, interceptor.opts.length)

        File.open(completion, 'w') do |fd|
            fd.puts "$build_#{script}='.\\#{File.basename(Build::Script)}'"
            fd.puts "$complete_#{script} = {"
                fd.puts "\tparam($wordToComplete, $commandAst, $fakeBoundParameters)"
                fd.puts "\t$opts = @(#{interceptor.opts.keys.collect{|x| "'#{x}'"}.join(',')})"
                fd.puts "\t$targets = @(#{aliases.collect{|x| "'#{x}'"}.join(',')})\r"
                fd.puts "\t$values = If ($wordToComplete -like \"-*\") {$opts} Else {$targets}"
                fd.puts "\t$values | Where-Object { $_ -like \"$wordToComplete*\" }"
            fd.puts "}"
            fd.puts "Register-ArgumentCompleter -Native -CommandName $build_#{script} -ScriptBlock $complete_#{script}"
        end

    end #~genps1

    module PS1

        class Option
            attr_reader :flag, :long, :params, :desc
            def initialize(flag, long:nil, params:nil, desc:nil)
                @flag = flag
                @long = long
                @params = params
                @desc = desc
            end
        end #~Option

        class OptionInterceptor
            attr_reader :opts
            def initialize()
                @opts = {}
            end
            def on(*args, &block)
                case args.length
                when 0,1
                    Assert.not_reached
                when 2
                    flag, desc = *args
                    make_opt(flag, desc: desc)
                when 3
                    flag, desc, long = *args
                    make_opt(flag, long: long, desc: desc)
                else
                    flag, desc, long = *args[0..2]
                    make_opt(flag, long: long, desc: desc, params: args[3..-1])
                end
                return
            end
            def banner=(*args) end
            def separator(*args) end
        private
            def make_opt(flag, long:nil, params:nil, desc:nil)
                if flag.start_with?('--[no-]')
                    make_opt("--#{flag[7..-1]}", long: long, params: params, desc: desc)
                    make_opt("--no-#{flag[7..-1]}", long: long, params: params, desc: desc)
                else
                    flag = flag.to_s.split(/\s+/).first
                    long = long.to_s.split(/\s+/).first unless long.nil?
                    @opts[flag] = Option.new(flag, long: long, params: params, desc: desc)
                end
            end
        end #~OptionInterceptor

    end #~ PS1

end #~ Build
