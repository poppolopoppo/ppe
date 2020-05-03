
require_once '../../Core/Environment.rb'

require_once '../Posix/Posix.rb'

module Build

    make_facet(:Linux_Base) do
        defines <<
            'PLATFORM_LINUX' <<
            '__LINUX__'
    end

    const_memoize(self, :LinuxPlatform_X86) do
        Platform.new('Linux32', :x86, :Linux).
            inherits!(Build.PosixPlatform_X86).
            inherits!(Build.Linux_Base).
            define!('_LINUX32')
    end
    const_memoize(self, :LinuxPlatform_X64) do
        Platform.new('Linux64', :x64, :Linux).
            inherits!(Build.PosixPlatform_X64).
            inherits!(Build.Linux_Base).
            define!('_LINUX64')
    end

    const_memoize(self, :LinuxConfig_Debug) { Build.PosixConfig_Debug.deep_dup }
    const_memoize(self, :LinuxConfig_FastDebug) { Build.PosixConfig_FastDebug.deep_dup }
    const_memoize(self, :LinuxConfig_Release) { Build.PosixConfig_Release.deep_dup }
    const_memoize(self, :LinuxConfig_Profiling) { Build.PosixConfig_Profiling.deep_dup }
    const_memoize(self, :LinuxConfig_Final) { Build.PosixConfig_Final.deep_dup }

    LinuxConfigs = [
        :LinuxConfig_Debug,
        :LinuxConfig_FastDebug,
        :LinuxConfig_Release,
        :LinuxConfig_Profiling,
        :LinuxConfig_Final,
    ]

    const_memoize(self, :LinuxCompiler_X86) do
        compiler = Build.PosixCompiler_X86
        compiler.deep_dup
    end
    const_memoize(self, :LinuxCompiler_X64) do
        compiler = Build.PosixCompiler_X64
        compiler.deep_dup
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
