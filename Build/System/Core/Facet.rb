
require './Common.rb'

module Build

    class ValueSet
        attr_reader :data
        def initialize() @data = [] end
        def empty?() @data.empty? end
        def &(value) @data.include?(value) end
        def +(other) @data.concat(other.data); @data.uniq!; self end
        def -(other) @data.delete_if{|x| other.include?(x) }; self end
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
            :preprocessorOption,
            :compilerOption,
            :linkerOption,
            :tag ]
        ATTRS = SETS.collect{|x| ('@'<<x.to_s<<'s').to_sym }
        attr_reader(*SETS.collect{|x| (x.to_s<<'s').to_sym })
        def initialize(options={})
            ATTRS.each{|facet| self.instance_variable_set(facet, ValueSet.new) }
            self.set(options)
        end
        def set(options={})
            options.each do |facet, value|
                facet = ('@'<<facet.to_s<<'s').to_sym
                valueSet = self.instance_variable_get(facet)
                valueSet.append(value)
            end
            return self
        end
        def tag?(*tags)
            tags.each do |tag|
                return false unless @tags & tag
            end
            return true
        end
        def +(other)
            ATTRS.each do |facet|
                self.instance_variable_get(facet) +
                    other.instance_variable_get(facet)
            end
            self
        end
        def -(other)
            ATTRS.each do |facet|
                self.instance_variable_get(facet) -
                    other.instance_variable_get(facet)
            end
            self
        end
        def ==(other)
            ATTRS.each do |facet|
                return false unless self.instance_variable_get(facet) ==
                    other.instance_variable_get(facet)
            end
            return true
        end
        def to_s()
            "{\n" << ATTRS
                .delete_if{|x| instance_variable_get(x).empty? }
                .collect{|x| "\t#{x}: "<<instance_variable_get(x).to_s }
                .join(",\n") << "\n}"
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
                output += facet if filter.call(environment)
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
