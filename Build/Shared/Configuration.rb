# frozen_string_literal: true


require_once '../Core/Policy.rb'

module Build

    class Configuration < Policy
        attr_reader :link
        def initialize(name, link)
            super(name)
            @link = link
        end
        def debug?() defines.data.include?('_DEBUG') end
        def static?() @link == :static end
        def dynamic?() @link == :dynamic end
        def customize(facet, env, target)
            artifact_type = env.target_artifact_type(target)
            case artifact_type
            when :executable, :library, :headers
                facet.defines << 'BUILD_LINK_STATIC'
            when :shared
                facet.defines << 'BUILD_LINK_DYNAMIC'
            else
                Log.fatal 'unsupported artifact type <%s>', artifact_type
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
