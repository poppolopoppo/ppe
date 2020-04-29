
require_once '../../Core/Environment.rb'

module Build

    persistent_switch(:LLVM, 'Use LLVM instead of GCC', init: true)

    make_facet(:Linux_Base) do
        defines <<
            'PLATFORM_PC' << 'PLATFORM_WINDOWS' <<
            'WIN32' << '__WINDOWS__'
        includes << File.join($SourcePath, 'winnt_version.h')
    end

    const_memoize(self, :LinuxPlatform_X86) do
        Platform.new('Win32', :x86, :Windows).
            inherits!(SharedPlatforms::X86).
            inherits!(Build.Linux_Base).
            define!('_WIN32')
    end
    const_memoize(self, :LinuxPlatform_X64) do
        Platform.new('Win64', :x64, :Windows).
            inherits!(SharedPlatforms::X64).
            inherits!(Build.Linux_Base).
            define!('_WIN64')
    end

    const_memoize(self, :LinuxConfig_Debug) { SharedConfigs::Debug.deep_dup.inherits!(Build.PosixCompiler_Debug) }
    const_memoize(self, :LinuxConfig_FastDebug) { SharedConfigs::FastDebug.deep_dup.inherits!(Build.PosixCompiler_FastDebug) }
    const_memoize(self, :LinuxConfig_Release) { SharedConfigs::Release.deep_dup.inherits!(Build.PosixCompiler_Release) }
    const_memoize(self, :LinuxConfig_Profiling) { SharedConfigs::Profiling.deep_dup.inherits!(Build.PosixCompiler_Profiling) }
    const_memoize(self, :LinuxConfig_Final) { SharedConfigs::Final.deep_dup.inherits!(Build.PosixCompiler_Final) }

    LinuxConfigs = [
        :LinuxConfig_Debug,
        :LinuxConfig_FastDebug,
        :LinuxConfig_Release,
        :LinuxConfig_Profiling,
        :LinuxConfig_Final,
    ]

    const_memoize(self, :LinuxCompiler_X86) do
        compiler = Build.LLVM ?
            Build.LLVM_Windows_Hostx86 : Build.VisualStudio_Hostx86
        compiler.deep_dup.inherits!(Build.WindowsSDK_X86)
    end
    const_memoize(self, :LinuxCompiler_X64) do
        compiler = Build.LLVM ?
            Build.LLVM_Windows_Hostx64 : Build.VisualStudio_Hostx64
        compiler.deep_dup.inherits!(Build.WindowsSDK_X64)
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
