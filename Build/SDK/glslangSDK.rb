# frozen_string_literal: true

require_once '../Core/Facet.rb'
require_once '../Utils/Prerequisite.rb'

module Build

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
        lib/MachineIndependent*
        lib/OGLCompiler*
        lib/OSDependent*
        lib/SPIRV*
    }

    const_memoize(self, :glslang_platform) do
        case Build.os_name
        when :Windows
            platform = 'windows-x64'
        when :Linux
            platform = 'linux'
        else
            Log.fatal('unsupported platform: <%s>', Build.os_name)
        end
    end
    const_memoize(self, :glslang_path) do
        File.join(GLSLANG_PATH, 'glslang.zip')
    end

    def import_glslang_lib(config)
        Build.import_remote_archive(
            'glslang-master-' + config,
            File.join(Build.glslang_path, config),
            'http://github.com/KhronosGroup/glslang/releases/download/master-tot/glslang-master-%s.zip' % config,
            *(GLSLANG_INCLUDE + GLSLANG_LIBRARIES) )
    end

    import_glslang_lib('linux-Debug')
    import_glslang_lib('linux-Release')

    import_glslang_lib('windows-x64-Debug')
    import_glslang_lib('windows-x64-Release')

    const_memoize(self, :glslang_files_debug) do
        listing = Build.send("glslang-master-#{Build.glslang_platform}-Debug")
        listing.map{|x| Pathname.new(x).relative_path_from(GLSLANG_PATH).to_s }
    end
    const_memoize(self, :glslang_files_release) do
        listing = Build.send("glslang-master-#{Build.glslang_platform}-Release")
        listing.map{|x| Pathname.new(x).relative_path_from(GLSLANG_PATH).to_s }
    end

    def glslang_include_path(debug)
        File.join(Build.glslang_path, "#{Build.glslang_platform}-#{debug ? 'Debug' : 'Release'}", 'Include')
    end
    def glslang_headers(debug)
        listing = debug ? Build.glslang_files_debug : Build.glslang_files_release
        listing.select{|x| GLSLANG_INCLUDE.any?{|y| File.fnmatch('*/'+y, x) }}
    end
    def glslang_libraries(debug)
        listing = debug ? Build.glslang_files_debug : Build.glslang_files_release
        listing.select{|x| GLSLANG_LIBRARIES.any?{|y| File.fnmatch('*/'+y, x) }}
    end

end #~ Build
