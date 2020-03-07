
BUILD_STARTED_AT = Time.now

require 'pathname'

$ApplicationPath = Pathname.new(File.absolute_path(File.dirname($0)))

def require_once(relpath)
    path = File.join(File.dirname(caller_locations(1, 1)[0].absolute_path), relpath)
    path = Pathname.new(path).relative_path_from($ApplicationPath).to_s
    path = './' << path unless path =~ /^\.\.\//
    require(path)
end

require_once 'Common.rb'

require_once 'Utils/Options.rb'

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

require_once 'Commands/BFF.rb'
require_once 'Commands/FASTBuild.rb'
require_once 'Commands/VSCode.rb'

module Build

    def self.elapsed_time()
        return Time.now - BUILD_STARTED_AT
    end

    def self.main(provider: :git, &namespace)
        Build::load_options()
        Build::parse_options()

        at_exit do
            Build::save_options()
            Build::Log.info('total duration: %fs', Build.elapsed_time)
        end

        Build.init_source_control(provider: provider)
        Build.run_command(&namespace)
    end

end #~ Build

class Build::Namespace
    def ppe_external!(name, &cfg)
        self.external!(name) do
            instance_exec(&cfg) if cfg
            glob!(path: nil)
            tag!(:nounity)
            includePath!($SourcePath)
        end
    end
    def ppe_module!(name, &cfg)
        self.library!(name) do
            instance_exec(&cfg) if cfg
            define!('EXPORT_PPE_'<<var_path.upcase)
            isolated_files!('ModuleExport.cpp')
            force_includes!(File.join(abs_path, 'ModuleExport.h'))
            pch!('stdafx.h', 'stdafx.cpp')
            glob!(path: 'Private')
            includePath!($SourcePath)
        end
    end
end

Build.namespace(:PPE) do
    namespace(:External) do
        ppe_external!('double-conversion')
        ppe_external!(:farmhash)
        ppe_external!(:lz4)
        ppe_external!(:xxHash)
    end
    namespace(:Runtime) do
        ppe_module!(:Core) do
            private_deps!(*namespace[:External]{[
                double_conversion,
                farmhash,
                lz4,
                xxHash ]})
        end
        ppe_module!(:RTTI) do
            public_deps!(namespace.Core)
        end
        ppe_module!(:VFS) do
            public_deps!(namespace.Core)
        end
        ppe_module!(:Serialize) do
            public_deps!(namespace.Core)
            public_deps!(namespace.RTTI)
            public_deps!(namespace.VFS)
        end
        namespace(:Graphics) do
            ppe_module!(:Test) do
                depends!(namespace[:Runtime].Core)
            end
        end
    end
    namespace(:Offline) do
        ppe_module!(:ContentPipeline) do
            define!('PPE_CONTENTPIPELINE_VERSION=1.0.0')
            public_deps!(namespace[:Runtime].Core)
            public_deps!(namespace[:Runtime].VFS)
            public_deps!(namespace[:Runtime, :Graphics].Test)
        end
        include!('Test.rb')
    end
end

Build.set_workspace_path('C:/Code/PPE')

Build.main() do
    Build.PPE
end
