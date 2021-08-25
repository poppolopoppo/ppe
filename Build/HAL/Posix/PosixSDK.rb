# frozen_string_literal: true

require_once '../../Common.rb'
require_once '../../Utils/Prerequisite.rb'

module Build

    make_facet(:PosixSDK_Base) do
        compilationFlag!('-pthread')
        linkerOptions << '-pthread' << '-ldl' << '-lncurses' << '-lglfw'
    end

    make_facet(:PosixSDK_X86) do
        self << Build.PosixSDK_Base
    end
    make_facet(:PosixSDK_X64) do
        self << Build.PosixSDK_Base
    end

    Build.make_command(:prereq, 'Setup Posix prerequisites (assume debian-based)') do
        pkgs = %w{
            build-essential
            gcc-multilib
            g++-multilib
            clang
            libncurses-dev
            libvulkan1
            libunwind-dev
            libglfw3-dev
        }
        Process.start({}, 'sudo', 'apt-get', '-y', 'install', *pkgs)
    end

end #~ Build