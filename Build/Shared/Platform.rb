# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Policy.rb'

module Build

    class Platform < Policy
        attr_reader :arch, :os
        def initialize(name, arch, os)
            super(name)
            @arch = arch
            @os = os
            export!('PlatformArch', @arch)
        end
        def match?(expr)
            super(expr) || Policy.match_expr?(@arch, expr) || Policy.match_expr?(@os, expr)
        end
        def intrinsics_supported() Log.error("%s: intrinsics_supported() is not implemented", @name); [] end
        def decorate(facet, env)
            facet.defines << "TARGET_PLATFORM=#{@os}"
            super(facet, env)
        end
    end #~ Platform

    module SharedPlatforms
        X86 = Platform.new('x86', :x86, :Dummy).
            define!('ARCH_X86', 'ARCH_32BIT')
        X64 = Platform.new('x64', :x64, :Dummy).
            define!('ARCH_X64', 'ARCH_64BIT')
        ARM = Platform.new('arm', :arm, :Dummy).
            define!('ARCH_ARM', 'ARCH_64BIT')
        PPC = Platform.new('ppc', :ppc, :Dummy).
            define!('ARCH_PPC', 'ARCH_64BIT')
        ALL = [ X86, X64, ARM, PPC ]
    end #~ SharedPlatforms

end #~ Build
