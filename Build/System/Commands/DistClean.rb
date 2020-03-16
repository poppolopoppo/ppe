# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

require_once 'FASTBuild.rb'

require 'fileutils'

module Build

    make_command(:"dist-clean", 'Delete generated files') do |&namespace|
        if Build::Args.empty?
            Log.info 'Clean: wiping *all* generated data'
            FileUtils.rm_rf($OutputPath, verbose: Log.verbose?)
        else

        end
    end

end #~ Build
