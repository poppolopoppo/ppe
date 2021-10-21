# frozen_string_literal: true

BUILD_STARTED_AT = Time.now

require 'pathname'

# default path is current main script folder
$ApplicationPath = Pathname.new(File.absolute_path(File.join(File.dirname(__FILE__), '..')))
Dir.chdir($ApplicationPath)

def require_once(relpath)
    path = File.join(File.dirname(caller_locations(1, 1)[0].absolute_path), relpath)
    path = Pathname.new(path).relative_path_from($ApplicationPath).to_s
    path = "./#{path}" unless path =~ /^\.\.\//
    require(path)
end

require_once 'Common.rb'

require_once 'Core/Facet.rb'
require_once 'Core/Policy.rb'
require_once 'Core/Target.rb'
require_once 'Core/Environment.rb'
require_once 'Core/Namespace.rb'

require_once 'Shared/Compiler.rb'
require_once 'Shared/Configuration.rb'
require_once 'Shared/Platform.rb'

require_once 'Utils/Log.rb'
require_once 'Utils/Prerequisite.rb'
require_once 'Utils/SourceControl.rb'

require_once 'HAL/hal.rb'

require_once 'Commands/Backup.rb'
require_once 'Commands/Bash.rb'
require_once 'Commands/BFF.rb'
require_once 'Commands/DistClean.rb'
require_once 'Commands/Export.rb'
require_once 'Commands/FASTBuild.rb'
require_once 'Commands/Hint.rb'
require_once 'Commands/Insights.rb'
require_once 'Commands/Lists.rb'
require_once 'Commands/PCH.rb'
require_once 'Commands/PS1.rb'
require_once 'Commands/Run.rb'
require_once 'Commands/Upx.rb'
require_once 'Commands/VCXProj.rb'
require_once 'Commands/VSCode.rb'

#require_once 'SDK/VulkanSDK.rb'

module Build

    def self.elapsed_time()
        return Time.now - BUILD_STARTED_AT
    end

    def self.main(provider: :git, &namespace)
        Build::init_options()
        Build::load_options()
        Build::parse_options()

        if $DEBUG
            require_once 'Utils/BreakOnException.rb'
        end

        at_exit do
            Build::save_options()
            Build::Log.info('total duration: %fs', Build.elapsed_time)
        end

        Build.init_source_control(provider: provider)

        begin
            Build.run_command(&namespace)
        # rescue RuntimeError => err
        #     $stderr.puts("caught a runtime error: %s, aborting build" % [err.to_s])
        #     exit(-42)
        end
    end

end #~ Build
