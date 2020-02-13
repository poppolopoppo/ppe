
require './Common.rb'
require './Core/Policy.rb'

module Build

    module SharedPlatforms
        X86 = Platform.new('x86', 'x86', :dummy)
        X64 = Platform.new('x64', 'x64', :dummy)
        ARM = Platform.new('arm', 'arm', :dummy)
        PPC = Platform.new('ppc', 'ppc', :dummy)
        ALL = [ X86, X64, ARM, PPC ]
    end #~ SharedPlatforms

    module SharedConfigs
        Debug = Configuration.new('Debug', :static)
        FastDebug = Configuration.new('FastDebug', :dynamic)
        Release = Configuration.new('Release', :static)
        Profiling = Configuration.new('Profiling', :static)
        Final = Configuration.new('Final', :static)
        ALL = [ Debug, FastDebug, Release, Profiling, Final ]
    end #~ SharedConfigs

    module SharedCompilers
        Dummy = Compiler.new('Dummy', nil, nil)
    end #~ SharedCompilers

    module SharedEnvironments
        Build.make_environment(self, SharedCompilers::Dummy, SharedPlatforms, SharedConfigs)
    end #~ SharedEnvironments

end #~ Build
