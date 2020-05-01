# frozen_string_literal: true

require_once 'ClangCl.rb'
require_once 'ResourceCompiler.rb'
require_once 'VisualStudio.rb'
require_once 'WindowsSDK.rb'

require_once '../../Core/Environment.rb'

module Build

    persistent_value(:Compiler, 'Select Windows C++ compiler', init: '2019', values: %w{ 2019 Insider LLVM })
    persistent_switch(:PDB, 'Generate a Program Debug Database', init: true)

    make_facet(:Windows_Base) do
        defines <<
            'PLATFORM_PC' << 'PLATFORM_WINDOWS' <<
            'WIN32' << '__WINDOWS__'
        includes << File.join($SourcePath, 'winnt_version.h')
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

    const_memoize(self, :WindowsConfig_Debug) { SharedConfigs::Debug.deep_dup.inherits!(Build.VisualStudio_Debug) }
    const_memoize(self, :WindowsConfig_FastDebug) { SharedConfigs::FastDebug.deep_dup.inherits!(Build.VisualStudio_FastDebug) }
    const_memoize(self, :WindowsConfig_Release) { SharedConfigs::Release.deep_dup.inherits!(Build.VisualStudio_Release) }
    const_memoize(self, :WindowsConfig_Profiling) { SharedConfigs::Profiling.deep_dup.inherits!(Build.VisualStudio_Profiling) }
    const_memoize(self, :WindowsConfig_Final) { SharedConfigs::Final.deep_dup.inherits!(Build.VisualStudio_Final) }

    WindowsConfigs = [
        :WindowsConfig_Debug,
        :WindowsConfig_FastDebug,
        :WindowsConfig_Release,
        :WindowsConfig_Profiling,
        :WindowsConfig_Final,
    ]

    const_memoize(self, :WindowsCompiler_X86) do
        case Build.Compiler
        when '2019'
            compiler = Build.VisualStudio_2019_Hostx86
        when 'Insider'
            compiler = Build.VisualStudio_Insider_Hostx86
        when 'LLVM'
            compiler = Build.LLVM_Windows_Hostx86
        else
            Assert.unexpected(Build.Compiler)
        end

        Assert.expect(compiler, Compiler)
        Log.log('Windows: using compiler <%s> for X86 (%s)', compiler.name, Build.Compiler)

        compiler.deep_dup.inherits!(Build.WindowsSDK_X86)
    end
    const_memoize(self, :WindowsCompiler_X64) do
        case Build.Compiler
        when '2019'
            compiler = Build.VisualStudio_2019_Hostx64
        when 'Insider'
            compiler = Build.VisualStudio_Insider_Hostx64
        when 'LLVM'
            compiler = Build.LLVM_Windows_Hostx64
        else
            Assert.unexpected(Build.Compiler)
        end

        Assert.expect(compiler, Compiler)
        Log.log('Windows: using compiler <%s> for X64 (%s)', compiler.name, Build.Compiler)

        compiler.deep_dup.inherits!(Build.WindowsSDK_X64)
    end

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
