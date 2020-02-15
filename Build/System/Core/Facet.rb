
require './Common.rb'

module Build

    class ValueSet
        attr_reader :data
        def initialize() @data = [] end
        def empty?() @data.empty? end
        def &(value) @data.include?(value) end
        def <<(other) @data.concat(other.data); @data.uniq!; self end
        def >>(other) @data.delete_if{|x| other.include?(x) }; self end
        def ==(other) @data.sort == other.data.sort end
        def append(value)
            case value
            when Integer,Float,String,Symbol
                @data << value
            when Array
                @data.concat(value)
            else
                raise ArgumentError.new('unexpected value')
            end
        end
        def remove(value)
            case value
            when Integer,Float,String,Symbol
                @data.delete(value)
            when Array
                @data.delete_if{ |x| value.include?(x) }
            else
                raise ArgumentError.new('unexpected value')
            end
        end
        def each(&block) @data.each(&block) end
        def to_s() @data.join(' ') end
    end #~ ValueSet

    class Facet
        SETS = [
            :define,
            :include,
            :includePath,
            :library,
            :libraryPath,
            :analysisOption,
            :preprocessorOption,
            :compilerOption,
            :linkerOption,
            :tag ]

        ATTRS = SETS.collect{|x| ('@'<<x.to_s<<'s').to_sym }
        attr_reader(*SETS.collect{|x| (x.to_s<<'s').to_sym })

        SETS.each do |facet|
            define_method("#{facet}=") do |value|
                Log.debug "set facet <%s::%s> = '%s'", self, facet, value
                instance_variable_get("@#{facet}s").append(value)
            end
        end

        def initialize(options={})
            ATTRS.each do |facet|
                #Log.debug "initialize facet <%s>", facet
                instance_variable_set(facet, ValueSet.new)
            end
            set(options)
        end
        def set(options={})
            options.each do |facet, value|
                send "#{facet}=", value
            end
            return self
        end
        def tag?(*tags)
            tags.each do |tag|
                return false unless @tags & tag
            end
            return true
        end
        def <<(other)
            ATTRS.each do |facet|
                dst = instance_variable_get(facet)
                src = other.instance_variable_get(facet)
                dst << src
            end
            self
        end
        def >>(other)
            ATTRS.each do |facet|
                dst = instance_variable_get(facet)
                src = other.instance_variable_get(facet)
                dst >> src
            end
            self
        end
        def ==(other)
            ATTRS.each do |facet|
                lhs = instance_variable_get(facet)
                rhs = other.instance_variable_get(facet)
                return false unless lhs == rhs
            end
            return true
        end
        def to_s()
            attrs = ATTRS.clone
                .delete_if{|x| instance_variable_get(x).empty? }
                .collect{|x| "\t#{x}: "<<instance_variable_get(x).to_s }
            attrs.empty? ? '{}' : "{\n" << attrs.join(",\n") << "\n}"
        end
    end #~ Facet

    class Decorator
        attr_reader :facets
        def initialize()
            @facets = []
        end
        def set(facet, &filter)
            @facets << [filter, facet]
        end
        def apply_decorator(output, environment)
            @facets.each do |(filter, facet)|
                output << facet if filter.call(environment)
            end
            return self
        end
    public
        def decorate!(facet, &filter)
            set(facet, &filter)
            return self
        end
        def facet!(platform: nil, config: nil, compiler: nil, facet: Facet.new)
            if platform or config or compiler
                decorate!(facet) do |env|
                    (platform.nil? or env.platform.match?(platform)) and
                    (config.nil? or env.config.match?(config)) and
                    (compiler.nil? or env.compiler.match?(compiler))
                end
            else
                decorate!(facet) { |env| true }
            end
        end
        def filter!(platform: nil, config: nil, compiler: nil, options: {})
            facet = Facet.new
            facet.set(options)
            facet!(platform: platform, config: config, compiler: compiler, facet: facet)
        end
    end #~ Decorator

end #~ Build
