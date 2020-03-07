
require_once '../Common.rb'
require_once '../Core/Policy.rb'

module Build

    class Configuration < Policy
        attr_reader :link
        def initialize(name, link)
            super(name)
            @link = link
        end
        def shipping?()
            case @name
            when :Debug
            when :FastDebug
            when :Release
            when :Profiling
                return false
            when :Final
                return true
            else
                Log.fatal 'unsupported config: <%s>', @name
            end
        end
        def customize(facet, env, target)
            artefact_type = env.target_artefact_type(target)
            case artefact_type
            when :executable, :library
                facet.defines << 'STATIC_LINK'
            when :shared
                facet.defines << 'STATIC_LINK'
            else
                Log.fatal 'unsupported artefact type <%s>', artefact_type
            end
            super(facet, env, target)
        end
    end #~ Configuration

    module SharedConfigs
        Debug = Configuration.new(:Debug, :static).
            define!('DEBUG', '_DEBUG')
        FastDebug = Configuration.new(:FastDebug, :dynamic).
            define!('FASTDEBUG', 'DEBUG', '_DEBUG')
        Release = Configuration.new(:Release, :static).
            define!('RELEASE', 'NDEBUG')
        Profiling = Configuration.new(:Profiling, :static).
            define!('RELEASE', 'NDEBUG', 'PROFILING_ENABLED')
        Final = Configuration.new(:Final, :static).
            define!('RELEASE', 'NDEBUG', 'FINAL_RELEASE')
        ALL = [ Debug, FastDebug, Release, Profiling, Final ]
    end #~ SharedConfigs

end #~ Build
