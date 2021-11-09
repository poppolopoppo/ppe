# frozen_string_literal: true

require_once '../Core/Facet.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    SPIRV_TOOLS_PATH = File.join($SourcePath, 'External', 'spirv-tools')

    SPIRV_TOOLS_INCLUDE = %w{
        install/include/spirv-tools/*
    }
    SPIRV_TOOLS_LIBRARIES = %w{
        !install/lib/SPIRV-Tools*
        !install/lib/SPIRV-Tools-shared*
    }

    const_memoize(self, :spirv_tools_platform) do
        case Build.os_name
        when :Windows
            platform = 'windows_vs2017'
        when :Linux
            platform = 'linux_clang'
        else
            Log.fatal('unsupported platform: <%s>', Build.os_name)
        end
    end
    const_memoize(self, :spirv_tools_path) do
        File.join(SPIRV_TOOLS_PATH, 'SPIRV-Tools.zip')
    end

    def import_spirv_tools_lib(config)
        archive = "spirv-tools-master-#{config}"
        Build.import_remote_archive(
            archive,
            File.join(Build.spirv_tools_path, config),
            'https://storage.googleapis.com/spirv-tools/badges/build_link_%s.html' % config,
            *(SPIRV_TOOLS_INCLUDE + SPIRV_TOOLS_LIBRARIES),
            redirect: true )
    end

    import_spirv_tools_lib('linux_clang_debug')
    import_spirv_tools_lib('linux_clang_release')

    import_spirv_tools_lib('windows_vs2017_debug')
    import_spirv_tools_lib('windows_vs2017_release')

    const_memoize(self, :spirv_tools_files_debug) do
        listing = Build.send("spirv-tools-master-#{Build.spirv_tools_platform}_debug")
        listing.map{|x| Pathname.new(x).relative_path_from(SPIRV_TOOLS_PATH).to_s }
    end
    const_memoize(self, :spirv_tools_files_release) do
        listing = Build.send("spirv-tools-master-#{Build.spirv_tools_platform}_release")
        listing.map{|x| Pathname.new(x).relative_path_from(SPIRV_TOOLS_PATH).to_s }
    end

    def spirv_tools_include_path(debug)
        File.join(Build.spirv_tools_path, "#{Build.spirv_tools_platform}_#{debug ? 'debug' : 'release'}", 'install', 'include')
    end
    def spirv_tools_headers(debug)
        listing = debug ? Build.spirv_tools_files_debug : Build.spirv_tools_files_release
        listing.select{|x| SPIRV_TOOLS_INCLUDE.any?{|y| File.fnmatch('*/'+y, x) }}
    end
    def spirv_tools_libraries(debug)
        listing = debug ? Build.spirv_tools_files_debug : Build.spirv_tools_files_release
        listing.select{|x| SPIRV_TOOLS_LIBRARIES.any?{|y| File.fnmatch('*/'+y, x) }}
    end

end #~ Build
