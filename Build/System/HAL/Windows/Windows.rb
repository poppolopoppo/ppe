
require_once 'VisualStudio.rb'
require_once 'WindowsSDK.rb'

require_once '../../Core/Environment.rb'

module Build

    make_facet(:Windows_Base) do
        defines <<
            'PLATFORM_PC' << 'PLATFORM_WINDOWS' <<
            'WIN32' << '__WINDOWS__'
    end

    const_memoize(self, :WindowsPlatform_X86) do
        Platform.new('Win32', :x86, :Windows).
            inherits!(SharedPlatforms::X86).
            inherits!(Build.Windows_Base).
            define!('_WIN32')
    end
    const_memoize(self, :WindowsPlatform_X64) do
        Platform.new('Win64', :x64, :Windows).
            inherits!(SharedPlatforms::X64).
            inherits!(Build.Windows_Base).
            define!('_WIN64')
    end

    const_memoize(self, :WindowsConfig_Debug) { SharedConfigs::Debug.clone.inherits!(Build.VisualStudio_Debug) }
    const_memoize(self, :WindowsConfig_FastDebug) { SharedConfigs::FastDebug.clone.inherits!(Build.VisualStudio_FastDebug) }
    const_memoize(self, :WindowsConfig_Release) { SharedConfigs::Release.clone.inherits!(Build.VisualStudio_Release) }
    const_memoize(self, :WindowsConfig_Profiling) { SharedConfigs::Profiling.clone.inherits!(Build.VisualStudio_Profiling) }
    const_memoize(self, :WindowsConfig_Final) { SharedConfigs::Final.clone.inherits!(Build.VisualStudio_Final) }

    WindowsConfigs = [
        :WindowsConfig_Debug,
        :WindowsConfig_FastDebug,
        :WindowsConfig_Release,
        :WindowsConfig_Profiling,
        :WindowsConfig_Final,
    ]

    const_memoize(self, :WindowsCompiler_X86) { Build.VisualStudio_Hostx86.clone.inherits!(Build.WindowsSDK_X86) }
    const_memoize(self, :WindowsCompiler_X64) { Build.VisualStudio_Hostx64.clone.inherits!(Build.WindowsSDK_X64) }

    WindowsEnvironment =
        make_environment(
            :WindowsCompiler_X86,
            :WindowsPlatform_X86,
            WindowsConfigs ) +
        make_environment(
            :WindowsCompiler_X64,
            :WindowsPlatform_X64,
            WindowsConfigs )

    Build.append_environments(*WindowsEnvironment)

end #~ Build
