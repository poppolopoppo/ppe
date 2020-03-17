# frozen_string_literal: true

BUILD_STARTED_AT = Time.now

require 'pathname'

$ApplicationPath = Pathname.new(Dir.pwd)

def require_once(relpath)
    path = File.join(File.dirname(caller_locations(1, 1)[0].absolute_path), relpath)
    path = Pathname.new(path).relative_path_from($ApplicationPath).to_s
    path = "./#{path}" unless path =~ /^\.\.\//
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
require_once 'Commands/DistClean.rb'
require_once 'Commands/FASTBuild.rb'
require_once 'Commands/PCH.rb'
require_once 'Commands/VCXProj.rb'
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
    def self.ppe_common(target, &cfg)
        target.define!("EXPORT_PPE_#{target.var_path.upcase}")
        target.includePath!($SourcePath)
        target.instance_exec(&cfg) if cfg
        return target
    end
    def ppe_external!(name, &cfg)
        self.external!(name) do
            glob!(path: nil)
            tag!(:nounity)
            force_includes!(File.join(abs_path, 'Public', name.to_s+'-external.h'))
            includePath!(File.join($SourcePath, 'Runtime', 'Core', 'Public'))
            Build::Namespace.ppe_common(self, &cfg)
        end
    end
    def ppe_module!(name, &cfg)
        self.library!(name) do
            isolated_files!('ModuleExport.cpp')
            force_includes!(File.join(abs_path, 'ModuleExport.h'))
            pch!('stdafx.h', 'stdafx.cpp')
            glob!(path: 'Private')
            Build::Namespace.ppe_common(self, &cfg)
        end
    end
end

Build.namespace(:PPE) do
    namespace(:External) do
        ppe_external!('double-conversion') do
            extra_files!(*%w{
                Public/double-conversion-external.h
                double-conversion.git/double-conversion/bignum.h
                double-conversion.git/double-conversion/bignum-dtoa.h
                double-conversion.git/double-conversion/cached-powers.h
                double-conversion.git/double-conversion/diy-fp.h
                double-conversion.git/double-conversion/double-conversion.h
                double-conversion.git/double-conversion/double-to-string.h
                double-conversion.git/double-conversion/fast-dtoa.h
                double-conversion.git/double-conversion/fixed-dtoa.h
                double-conversion.git/double-conversion/ieee.h
                double-conversion.git/double-conversion/string-to-double.h
                double-conversion.git/double-conversion/strtod.h
                double-conversion.git/double-conversion/utils.h })
            isolated_files!(*%w{
                double-conversion.git/double-conversion/bignum.cc
                double-conversion.git/double-conversion/bignum-dtoa.cc
                double-conversion.git/double-conversion/cached-powers.cc
                double-conversion.git/double-conversion/double-to-string.cc
                double-conversion.git/double-conversion/fast-dtoa.cc
                double-conversion.git/double-conversion/fixed-dtoa.cc
                double-conversion.git/double-conversion/string-to-double.cc
                double-conversion.git/double-conversion/strtod.cc })
        end
        ppe_external!(:farmhash) do
            extra_files!(*%w{
                Public/farmhash-external.h
                farmhash.git/src/farmhash.h })
            isolated_files!('farmhash.git/src/farmhash.cc')
        end
        ppe_external!(:lz4) do
            extra_files!(*%w{
                Public/lz4-external.h
                lz4.git/lib/lz4.h
                lz4.git/lib/lz4hc.h
                lz4.git/lib/lz4opt.h })
            isolated_files!(*%w{
                lz4.git/lib/lz4.c
                lz4.git/lib/lz4hc.c })
        end
        ppe_external!(:xxHash) do
            extra_files!(*%w{
                Public/xxHash-external.h
                xxHash.git/xxhash.h })
            isolated_files!('Private/xxHash-dummy.cpp')
        end
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
            excluded_files!('Private/Markup/MarkupSerializer.cpp') # TODO : still need some work, but is it really needed ?
        end
    end
=begin
    namespace(:Offline) do
        ppe_module!(:ContentPipeline) do
            define!('PPE_CONTENTPIPELINE_VERSION=1.0.0')
            public_deps!(namespace[:Runtime].Core)
            public_deps!(namespace[:Runtime].VFS)
            public_deps!(namespace[:Runtime, :Graphics].Test)
        end
        include!('Test.rb')
    end
=end
end

Build.set_workspace_path('C:/Code/PPE')

Build.main() do
    Build.PPE
end
