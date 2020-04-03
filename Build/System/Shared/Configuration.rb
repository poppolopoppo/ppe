# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Policy.rb'

module Build

    class Configuration < Policy
        attr_reader :link
        def initialize(name, link)
            super(name)
            @link = link
        end
        def customize(facet, env, target)
            artefact_type = env.target_artefact_type(target)
            case artefact_type
            when :executable, :library, :headers
                facet.defines << 'BUILD_LINK_STATIC'
            when :shared
                facet.defines << 'BUILD_LINK_DYNAMIC'
            else
                Log.fatal 'unsupported artefact type <%s>', artefact_type
            end
            super(facet, env, target)
        end
    end #~ Configuration

    module SharedConfigs
        Debug = Configuration.new(:Debug, :static).
            define!('DEBUG', '_DEBUG').
            tag!(:debug)
        FastDebug = Configuration.new(:FastDebug, :dynamic).
            define!('FASTDEBUG', 'DEBUG', '_DEBUG').
            tag!(:debug)
        Release = Configuration.new(:Release, :static).
            define!('RELEASE', 'NDEBUG').
            tag!(:ndebug)
        Profiling = Configuration.new(:Profiling, :static).
            define!('RELEASE', 'NDEBUG', 'PROFILING_ENABLED').
            tag!(:ndebug, :profiling)
        Final = Configuration.new(:Final, :static).
            define!('RELEASE', 'NDEBUG', 'FINAL_RELEASE').
            tag!(:ndebug, :shipping)
        ALL = [ Debug, FastDebug, Release, Profiling, Final ]
    end #~ SharedConfigs

end #~ Build
