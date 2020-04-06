#!/usr/bin/env ruby
# frozen_string_literal: true

require_relative 'Build/System/Build.rb'

class Build::Namespace
    include Build
    def self.ppe_common(target, &cfg)
        target.define!("EXPORT_PPE_#{target.var_path.upcase}")
        target.includePath!($SourcePath)
        target.instance_exec(&cfg) if cfg
        return target
    end
    def ppe_headers!(name, &cfg)
        self.headers!(name) do
            glob!(path: nil)
            includePath!($SourcePath, File.join($SourcePath, 'Runtime', 'Core', 'Public'))
            instance_exec(&cfg) if cfg
        end
    end
    def ppe_external!(name, &cfg)
        self.external!(name) do
            tag!(:nopdb, :nounity)
            glob!(path: nil)
            includePath!(File.join($SourcePath, 'Runtime', 'Core', 'Public'))
            Namespace.ppe_common(self, &cfg)
        end
    end
    def ppe_module!(name, &cfg)
        self.library!(name) do
            glob!(path: 'Private')
            pch!('stdafx.h', 'stdafx.cpp')
            source_files!('ModuleExport.cpp')
            force_includes!(File.join(abs_path, 'ModuleExport.h'))
            Namespace.ppe_common(self, &cfg)
        end
    end
    def ppe_executable!(name, &cfg)
        self.executable!(name) do
            tag!(:nounity)
            glob!(path: 'Private')
            pch!('stdafx.h', 'stdafx.cpp')
            if Build.os_windows?
                ## natvis integration
                linkerOptions << "/NATVIS:\"#{File.join($ExtrasPath, 'Debug', 'PPE.natvis')}\""
                ## resource compiler
                rel_resource_rc = 'resource.rc'
                abs_resource_rc = File.join($SourcePath, expand_path(rel_resource_rc))
                unit!('Resource') do
                    Log.verbose('add resource rc unit to <%s> for "%s"', abs_path, abs_resource_rc)
                    compiler_override!(Build.WindowsResourceCompiler)
                    source_files!(rel_resource_rc)
                end if File.exist?(abs_resource_rc)
            end
            Namespace.ppe_common(self, &cfg)
        end
    end
end

require_relative 'Source/Source.rb'

# Build.namespace(:PPE) do
#     namespace(:Runtime) do
#         ppe_module!(:Core) do
#             private_deps!(*namespace[:External]{[
#                 double_conversion,
#                 farmhash,
#                 lz4,
#                 xxHash ]})
#         end
#         ppe_module!(:RTTI) do
#             public_deps!(namespace.Core)
#         end
#         ppe_module!(:VFS) do
#             public_deps!(namespace.Core)
#         end
#         ppe_module!(:Serialize) do
#             public_deps!(namespace.Core)
#             public_deps!(namespace.RTTI)
#             public_deps!(namespace.VFS)
#             excluded_files!('Private/Markup/MarkupSerializer.cpp') # TODO : still need some work, but is it really needed ?
#         end
#     end
# end

Build.main() do
    Build.PPE
end
