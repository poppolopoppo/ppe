# frozen_string_literal: true


require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

require_once 'FASTBuild.rb'

require 'fileutils'
require 'find'

module Build

    make_command(:"dist-clean", 'Delete generated files') do |&namespace|
        if Build::Args.empty?
            Log.info 'Clean: wiping *all* generated files ...'
            FileUtils.rm_rf($OutputPath, verbose: Log.verbose?)
        else
            re_delete = Regexp.new(Build::Args.collect{|x| Regexp.escape(x) }.join('|'))
            Log.info 'Clean: wiping generated files matching /%s/ ...', re_delete
            Log.info 'Clean: scanning for generated files inside "%s"', $OutputPath
            Find.find($OutputPath) do |entry|
                path = Pathname.new(entry).expand_path
                if entry =~ re_delete
                    FileUtils.rm_rf(path.to_s, verbose: Log.verbose?)
                end
            end
        end
    end

end #~ Build
