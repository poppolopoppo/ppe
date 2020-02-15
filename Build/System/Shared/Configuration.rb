
require './Common.rb'
require './Core/Policy.rb'

module Build

    class Configuration < Policy
        attr_reader :link
        def initialize(name, link)
            super(name)
            @link = link
        end
    end #~ Configuration

    module SharedConfigurations
        Debug = Configuration.new('Debug', :static)
        FastDebug = Configuration.new('FastDebug', :dynamic)
        Release = Configuration.new('Release', :static)
        Profiling = Configuration.new('Profiling', :static)
        Final = Configuration.new('Final', :static)
        ALL = [ Debug, FastDebug, Release, Profiling, Final ]
    end #~ SharedConfigs

end #~ Build
