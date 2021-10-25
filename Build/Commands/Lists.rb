# frozen_string_literal: true


require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    make_command(:'list-executables', 'Print all executables in plain text') do |&namespace|
        environments = Build.fetch_environments
        targets = namespace[].select(*Build::Args)

        aliases = []
        Build.each_build_aliases(environments, targets, only_executables=true) do |alias_name, alias_targets|
            aliases << alias_name
        end

        aliases.each {|x| print "#{x}\n" }
    end

    make_command(:'list-targets', 'Print all targets in plain text') do |&namespace|
        environments = Build.fetch_environments
        targets = namespace[].select(*Build::Args)

        aliases = []
        Build.each_build_aliases(environments, targets) do |alias_name, alias_targets|
            aliases << alias_name
        end

        aliases.each {|x| print "#{x}\n" }
    end

end #~ Build
