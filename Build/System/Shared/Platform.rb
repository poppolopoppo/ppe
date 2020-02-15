
require './Common.rb'
require './Core/Policy.rb'

module Build

    class Platform < Policy
        attr_reader :target, :os
        def initialize(name, target, os)
            super(name)
            @target = target
            @os = os
        end
        def intrinsics_supported() Log.error("%s: intrinsics_supported() is not implemented", @name); [] end
    end #~ Platform

    module SharedPlatforms
        X86 = Platform.new('x86', 'x86', :dummy)
        X64 = Platform.new('x64', 'x64', :dummy)
        ARM = Platform.new('arm', 'arm', :dummy)
        PPC = Platform.new('ppc', 'ppc', :dummy)
        ALL = [ X86, X64, ARM, PPC ]
    end #~ SharedPlatforms

end #~ Build
