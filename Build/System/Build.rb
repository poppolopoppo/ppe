
require './Core/Facet.rb'
require './Core/Policy.rb'
require './Core/Target.rb'
require './Core/Environment.rb'
require './Core/Namespace.rb'
require './Core/Shared.rb'

require './Utils/Log.rb'
require './Utils/Options.rb'

require 'set'

Build::load_options()
Build::parse_options()
END { Build::save_options() }

require 'pp'

# Build::SharedEnvironments::ALL.each do |env|
#     Build::Log.info env.name
# end

Build::SharedConfigs::Debug.
    define!("_DEBUG").
    compilerOption!('/Od', '/GS').
    include!('Header/System/Toto.h').
    linkerOption!('/OPT:ICF', '/INCREMENTAL').
    tag!(:debug)

Build::SharedConfigs::Release.
    on_tag!(:x64, compilerOption: '/ARCH:X64')

FastPDBLinking = Build::Facet.new linkerOption: '/LINK:FASTPDB'

module PPE
    extend Build::Namespace

    module Runtime
        extend Build::Namespace

        Core = library('Core')
        VFS = library('VFS').
            depends!(Core, :public).
            filter!(config: 'Deb*', options: {
                compilerOption: '/Od',
                includePath: 'Runtime/Core/Public'
            }).
            filter!(platform: 'x64', options: {
                define: ['ARCH_X64', 'ARCH=X64'],
                tag: :x64
            }).
            facet!(config: 'Release', facet: FastPDBLinking)

    end #~ Runtime

    module Offline
        extend Build::Namespace

        ContentPipeline = library('ContentPipeline').
            depends!(Runtime::VFS, :public)

    end #~ Offline

end #~ PPE

Build::Log.pin('** This is a pinned message **')

Build::Log.info PPE.all.join(', ')
Build::Log.clear_pin

puts Build::SharedEnvironments::Dummy_x64_Release.expand(PPE::Runtime::VFS)
puts Build::SharedEnvironments::Dummy_x64_Debug.expand(PPE::Offline::ContentPipeline)
