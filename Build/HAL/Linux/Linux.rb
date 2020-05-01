
require_once '../../Core/Environment.rb'

require_once '../Posix/Posix.rb'

module Build

    make_facet(:Linux_Base) do
        defines << 'PLATFORM_PC' <<
            'PLATFORM_LINUX' <<
            '__LINUX__'
    end

    const_memoize(self, :LinuxPlatform_X86) do
        Platform.new('Linux32', :x86, :Linux).
            inherits!(SharedPlatforms::X86).
            inherits!(Build.Linux_Base).
            define!('_LINUX32')
    end
    const_memoize(self, :LinuxPlatform_X64) do
        Platform.new('Linux64', :x64, :Linux).
            inherits!(SharedPlatforms::X64).
            inherits!(Build.Linux_Base).
            define!('_LINUX64')
    end

    const_memoize(self, :LinuxConfig_Debug) { SharedConfigs::Debug.deep_dup.inherits!(Build.PosixConfig_Debug) }
    const_memoize(self, :LinuxConfig_FastDebug) { SharedConfigs::FastDebug.deep_dup.inherits!(Build.PosixConfig_FastDebug) }
    const_memoize(self, :LinuxConfig_Release) { SharedConfigs::Release.deep_dup.inherits!(Build.PosixConfig_Release) }
    const_memoize(self, :LinuxConfig_Profiling) { SharedConfigs::Profiling.deep_dup.inherits!(Build.PosixConfig_Profiling) }
    const_memoize(self, :LinuxConfig_Final) { SharedConfigs::Final.deep_dup.inherits!(Build.PosixConfig_Final) }

    LinuxConfigs = [
        :LinuxConfig_Debug,
        :LinuxConfig_FastDebug,
        :LinuxConfig_Release,
        :LinuxConfig_Profiling,
        :LinuxConfig_Final,
    ]

    const_memoize(self, :LinuxCompiler_X86) do
        compiler = Build.PosixCompiler_Hostx86
        compiler.deep_dup.inherits!(Build.PosixSDK_X86)
    end
    const_memoize(self, :LinuxCompiler_X64) do
        compiler = Build.PosixCompiler_Hostx64
        compiler.deep_dup.inherits!(Build.PosixSDK_X64)
    end

    LinuxEnvironment =
        make_environment(
            :LinuxCompiler_X86,
            :LinuxPlatform_X86,
            LinuxConfigs ) +
        make_environment(
            :LinuxCompiler_X64,
            :LinuxPlatform_X64,
            LinuxConfigs )

    Build.append_environments(*LinuxEnvironment)

end #~ Build
