# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    make_command(:'bash.d', 'Generate bash completion script') do |&namespace|
        environments = Build.fetch_environments
        targets = namespace[].select(*Build::Args)

        aliases = []
        targets.each do |target|
            aliases << target.abs_path
        end
        environments.each do |env|
            aliases << env.family
            targets.each do |target|
                aliases << "#{target.abs_path}-#{env.family}"
            end
        end

        interceptor = Bash::OptionInterceptor.new
        Build.make_pgm_options(interceptor)

        ext = File.extname(Build::Script)
        script = File.basename(Build::Script, ext)

        interceptor.opts.each do |flag, opt|
            Log.log('Bash: found option "%s>" -> %s', flag, opt.desc)
        end
        aliases.each do |target|
            Log.log('Bash: found alias <%s>', target)
        end

        completion = File.join($ExtrasPath,  script + '-completion.bash')
        Log.info('Bash: generating bash completion script with %d aliases and %s options', aliases.length, interceptor.opts.length)

        File.open(completion, 'w') do |fd|
            fd.puts '#/usr/bin/env bash'
            fd.puts "# Generated bash completion script by #{Build::Script}"
            fd.puts "_#{script}_completions() {"
                fd.puts 'local cur prev opts targets'
                fd.puts 'COMPREPLY=()'
                fd.puts 'cur="${COMP_WORDS[COMP_CWORD]}"'
                fd.puts 'prev="${COMP_WORDS[COMP_CWORD-1]}"'
                fd.puts "opts=\"#{interceptor.opts.keys.join(' ')}\""
                fd.puts "targets=\"#{aliases.join(' ')}\""
                fd.puts 'if [[ ${cur} == -* ]] ; then'
                fd.puts '   COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )'
                fd.puts '   return 0'
                fd.puts 'else'
                fd.puts '   COMPREPLY=( $(compgen -W "${targets}" -- ${cur}) )'
                fd.puts '   return 0'
                fd.puts 'fi'
            fd.puts '}'
            fd.puts "complete -F _#{script}_completions #{File.basename(Build::Script)}"
        end

    end #~genbash

    module Bash

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
                    make_opt(flag, long: desc, desc: desc)
                else
                    flag, desc, long = *args[0..2]
                    make_opt(flag, long: desc, desc: desc, params: args[3..-1])
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
                    flag = flag.split(/\s+/).first
                    long = long.split(/\s+/).first unless long.nil?
                    @opts[flag] = Option.new(flag, long: long, params: params, desc: desc)
                end
            end
        end #~OptionInterceptor

    end #~ Bash

end #~ Build
