# frozen_string_literal: true

require_once '../Core/Facet.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    persistent_switch(:glslang_debug, 'Use Debug variant of libglslang', init: false)

    GLSLANG_PATH = File.join($SourcePath, 'External', 'glslang')

    GLSLANG_INCLUDE = %w{
        include/glslang/Include/*
        include/glslang/Public/*
        include/glslang/MachineIndependent/*
        include/glslang/SPIRV/*
    }
    GLSLANG_LIBRARIES = %w{
        lib/GenericCodeGen*
        lib/glslang*
        lib/OSDependent*
        lib/SPIRV*
    }

    const_memoize(self, :glslang_config) do
        case Build.os_name
        when :Windows
            platform = 'windows-x64'
        when :Linux
            platform = 'linux'
        else
            Log.fatal('unsupported platform: <%s>', Build.os_name)
        end
        config = Build.glslang_debug ? 'Debug' : 'Release'
        "#{platform}-#{config}"
    end
    const_memoize(self, :glslang_path) do
        File.join(GLSLANG_PATH, 'glslang.zip', Build.glslang_config)
    end

    def import_glslang_lib(config)
        Build.import_remote_archive(
            'glslang-master-' + config,
            Build.glslang_path,
            'http://github.com/KhronosGroup/glslang/releases/download/master-tot/glslang-master-%s.zip' % config,
            *(GLSLANG_INCLUDE + GLSLANG_LIBRARIES) )
    end

    import_glslang_lib('linux-Debug')
    import_glslang_lib('linux-Release')

    import_glslang_lib('windows-x64-Debug')
    import_glslang_lib('windows-x64-Release')

    const_memoize(self, :glslang_files) do
        config = Build.glslang_config
        listing = Build.send('glslang-master-' + config)
        listing.map{|x| Pathname.new(x).relative_path_from(GLSLANG_PATH).to_s }
    end
    const_memoize(self, :glslang_headers) do
        Build.glslang_files.select{|x| GLSLANG_INCLUDE.any?{|y| File.fnmatch('*/'+y, x) }}
    end
    const_memoize(self, :glslang_libraries) do
        Build.glslang_files.select{|x| GLSLANG_LIBRARIES.any?{|y| File.fnmatch('*/'+y, x) }}
    end

end #~ Build
