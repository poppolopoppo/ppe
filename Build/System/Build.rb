
BUILD_STARTED_AT = Time.now

require './Core/Facet.rb'
require './Core/Policy.rb'
require './Core/Target.rb'
require './Core/Environment.rb'
require './Core/Namespace.rb'

require './Shared/Compiler.rb'
require './Shared/Configuration.rb'
require './Shared/Platform.rb'

require './Utils/Log.rb'
require './Utils/Options.rb'
require './Utils/Prerequisite.rb'
require './Utils/SourceControl.rb'

require 'set'
require 'pp'

Build::SharedConfigurations::Debug.
    define!("_DEBUG").
    compilerOption!('/Od', '/GS').
    include!('Header/System/Toto.h').
    linkerOption!('/OPT:ICF', '/INCREMENTAL').
    tag!(:debug)

Build::SharedConfigurations::Release.
    on_tag!(:x64, compilerOption: '/ARCH:X64')

FastPDBLinking = Build::Facet.new linkerOption: '/LINK:FASTPDB'

module PPE
    extend Build::Namespace

    make_environment(
        Build::SharedCompilers::Dummy,
        Build::SharedPlatforms,
        Build::SharedConfigurations )

    import_envvar(:APPDATA)
    import_envvar(:USERPROFILE)

    persistent_array(:Targets, 'targets to build')

    module Runtime
        extend Build::Namespace

        library(:Core)
        library(:VFS) do
            depends!(Runtime.Core, :public)
            filter!(config: 'Deb*', options: {
                compilerOption: '/Od',
                includePath: 'Runtime/Core/Public'
            })
            filter!(platform: 'x64', options: {
                define: ['ARCH_X64', 'ARCH=X64'],
                tag: :x64
            })
            facet!(config: 'Release', facet: FastPDBLinking)
        end

    end #~ Runtime

    module Offline
        extend Build::Namespace

        library(:ContentPipeline) do
            depends!(Runtime.VFS, :public)
        end

    end #~ Offline

end #~ PPE

Build::load_options()
Build::parse_options()
END {
    Build::save_options()
    Build::Log.info('total duration: %fs', Time.now - BUILD_STARTED_AT)
}

Build.init_source_control(provider: :git)

Build::Log.info PPE::Environments.collect{|x| x.name }.join(', ')
Build::Log.info PPE.all.join(', ')
Build::Log.clear_pin

Build::Log.info PPE::Dummy_x64_Release.expand(PPE::Runtime.VFS)
Build::Log.info PPE::Dummy_x64_Debug.expand(PPE::Offline.ContentPipeline)
Build::Log.info PPE.USERPROFILE

