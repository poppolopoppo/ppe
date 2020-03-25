# frozen_string_literal: true

require_once '../Common.rb'

module Build

    class ValueSet
        attr_reader :data
        def initialize(data=[]) @data = data end
        def empty?() @data.empty? end
        def &(value) @data.include?(value) end
        def <<(other) append(other); self end
        def >>(other) remove(other); self end
        def ==(other) @data.sort == other.data.sort end
        def append(*values)
            values.each do |value|
                case value
                when Integer,Float,String,Symbol
                    if $DEBUG && value.is_a?(String) && @data.include?(value)
                        Log.fatal 'already append "%s"', value
                    end
                    @data << value
                when Array
                    if $DEBUG
                        append(*value)
                    else
                        @data.concat(value)
                    end
                when ValueSet
                    if $DEBUG
                        append(*value.data)
                    else
                        @data.concat(value.data)
                    end
                else
                    Log.fatal 'unexpected value: %s', value
                end
            end
            return self
        end
        def remove(*values)
            values.each do |value|
                case value
                when Integer,Float,String,Symbol
                    @data.delete(value)
                when Array
                    @data.delete_if{ |x| value.include?(x) }
                when ValueSet
                    @data.delete_if{ |x| value.data.include?(x) }
                else
                    Log.fatal 'unexpected value: %s', value
                end
            end
            return self
        end
        def expand!(vars={})
            Assert.check{ not frozen? }
            @data.collect! do |value|
                case value
                when String
                    value = value.dup if value.frozen?
                    vars.each do |key, subst|
                        value.gsub!(key, subst)
                    end
                end
                value
            end
            return self
        end
        def each(&block) @data.each(&block) end
        def join(*args) @data.join(*args) end
        def to_s() @data.join(' ') end
        def freeze()
            @data.freeze
            super()
        end
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
            :pchOption,
            :librarianOption,
            :linkerOption,
            :tag ]

        ATTRS = SETS.collect{|x| "@#{x}s".to_sym }
        attr_reader(*SETS.collect{|x| "#{x}s".to_sym })

        SETS.each do |facet|
            define_method("#{facet}=") do |value|
                Log.debug "set facet <%s::%s> = '%s'", self, facet, value
                instance_variable_get("@#{facet}s").append(value)
            end
        end

        attr_reader :vars

        def initialize(data={})
            @vars = {}
            ATTRS.each do |facet|
                #Log.debug "initialize facet <%s>", facet
                instance_variable_set(facet, ValueSet.new)
            end
            set!(data)
        end
        def [](facet) instance_variable_get(facet) end
        def []=(facet, value) instance_variable_set(facet, value) end
        def set!(options={})
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
        def export!(key, subst)
            @vars["$#{key}$"] = subst.to_s
            return self
        end
        def expand!()
            ATTRS.each do |facet|
                instance_variable_get(facet).expand!(@vars)
            end unless @vars.empty?
            return self
        end
        def <<(other)
            @vars.merge!(other.vars)
            ATTRS.each do |facet|
                dst = instance_variable_get(facet)
                src = other.instance_variable_get(facet)
                dst << src
            end
            self
        end
        def >>(other)
            other.vars.each{|k,v| @vars.delete(k) }
            ATTRS.each do |facet|
                dst = instance_variable_get(facet)
                src = other.instance_variable_get(facet)
                dst >> src
            end
            self
        end
        def ==(other)
            return false unless @vars == other.vars
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
                .collect{|x| "\t#{x}: #{instance_variable_get(x)}" }
            attrs.empty? ? '{}' : "{\n#{attrs.join(",\n")}\n}"
        end
        def copy() 
            newFacet = Facet.new(self)
            Assert.unreached if newFacet.frozen?
            return facet
        end
        def freeze()
            @vars.freeze
            ATTRS.each do |facet|
                instance_variable_get(facet).freeze
            end
            super()
        end
    end #~ Facet

    def make_facet(name, options={}, &block)
        Build.const_memoize(self, name) do
            facet = Facet.new(options)
            facet.instance_exec(&block) if block
            facet
        end
    end

    class Decorator
        attr_reader :facets
        def initialize()
            @facets = []
        end
        def decorate!(facet, &filter)
            @facets << [filter, facet]
            return self
        end
        def apply_decorator(output, environment)
            @facets.each do |(filter, facet)|
                output << facet if filter.call(environment)
            end
            return self
        end
        def freeze()
            @facets.freeze
            super()
        end
    public
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
            facet.set!(options)
            facet!(platform: platform, config: config, compiler: compiler, facet: facet)
        end
    end #~ Decorator

end #~ Build
