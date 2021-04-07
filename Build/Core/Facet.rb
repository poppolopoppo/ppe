# frozen_string_literal: true

require_once '../Common.rb'

module Build

    class ValueSet
        attr_reader :data
        def initialize(data=[]) @data = data end
        def first() @data.first end
        def last() @data.last end
        def empty?() @data.empty? end
        def &(value) @data.include?(value) end
        def <<(other) push_back(other); self end
        def >>(other) remove(other); self end
        def ==(other) @data.sort == other.data.sort end
        def push_back(*values)
            values.each do |value|
                case value
                when Integer,Float,String,Symbol
                    if $DEBUG && value.is_a?(String) && !(value[0] =~ /[\\\/-]/) && @data.include?(value)
                        Log.fatal "already append \"%s\"\n\tcontent: %s", value, @data.collect{|x|"'#{x}'"}.join(' ')
                    end
                    @data << value
                when Array
                    if $DEBUG
                        push_back(*value)
                    else
                        @data.concat(value)
                    end
                when ValueSet
                    if $DEBUG
                        push_back(*value.data)
                    else
                        @data.concat(value.data)
                    end
                else
                    Log.fatal 'unexpected value: %s', value
                end
            end
            return self
        end
        alias :append :push_back
        def push_front(*values)
            values.each do |value|
                case value
                when Integer,Float,String,Symbol
                    if $DEBUG && value.is_a?(String) && !(value[0] =~ /[\\\/-]/) && @data.include?(value)
                        Log.fatal "already append \"%s\"\n\tcontent: %s", value, @data.collect{|x|"'#{x}'"}.join(' ')
                    end
                    @data.insert(0, value)
                when Array
                    if $DEBUG
                        push_front(*value)
                    else
                        @data = value + @data
                    end
                when ValueSet
                    if $DEBUG
                        push_front(*value.data)
                    else
                        @data = value.data + @data
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
            return self if vars.empty?
            @data.collect! do |value|
                case value
                when String
                    if value.include?('$')
                        value = value.dup if value.frozen?
                        vars.each do |key, subst|
                            value.gsub!(key, subst)
                        end
                    end
                    value
                end
                value
            end
            return self
        end
        def each(&block) @data.each(&block) end
        def join(*args) @data.join(*args) end
        def to_a() @data end
        def to_s() @data.join(' ') end
        def clear() @data.clear end
        def freeze()
            @data.freeze
            super()
        end
        def deep_dup()
            ValueSet.new(@data.dup)
        end
    end #~ ValueSet

    class Facet
        SETS = [
            :define,
            :include,
            :includePath,
            :library,
            :libraryPath,   # project include paths (user)
            :externPath,    # extern include paths (sdk)
            :systemPath,    # system include paths (stl)
            :analysisOption,
            :preprocessorOption,
            :compilerOption,
            :pchOption,
            :librarianOption,
            :linkerOption,
            :tag ]

        def self.pluralize(name)
            case name
            when :library
                return :libraries
            else
                return "#{name}s".to_sym
            end
        end

        ATTRS = SETS.collect{|x| "@#{Facet.pluralize(x)}".to_sym }
        attr_reader(*SETS.collect{|x| Facet.pluralize(x).to_sym })

        SETS.each do |facet|
            define_method("#{facet}=") do |value|
                Log.debug "set facet <%s::%s> = '%s'", self, facet, value
                instance_variable_get("@#{Facet.pluralize(facet)}").push_back(value)
            end
        end

        attr_reader :compiler, :preprocessor
        attr_reader :vars

        def initialize(data={})
            if data.is_a?(Facet)
                @compiler = data.compiler
                @preprocessor = data.preprocessor
                @vars = other.vars.dup
            else
                @compiler = nil
                @preprocessor = nil
                @vars = {}
            end

            ATTRS.each do |facet|
                #Log.debug "initialize facet <%s>", facet
                instance_variable_set(facet, ValueSet.new)
            end

            set!(data)
        end

        def any_includePaths()
            any = []
            any.concat(@systemPaths.to_a)
            any.concat(@externPaths.to_a)
            any.concat(@includePaths.to_a)
            return any
        end

        def [](facet) instance_variable_get(facet) end
        def []=(facet, value) instance_variable_set(facet, value) end
        def each(&block) ATTRS.each{|facet| yield(facet, self[facet]) } end
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
        def compiler!(compiler)
            Assert.expect(compiler, Compiler)
            @compiler = compiler
            return self
        end
        def compilationFlag!(*flags)
            @analysisOptions.push_back(*flags)
            @preprocessorOptions.push_back(*flags)
            @compilerOptions.push_back(*flags)
            @pchOptions.push_back(*flags)
            return self
        end
        def no_compilationFlag!(*flags)
            @analysisOptions.remove(*flags)
            @preprocessorOptions.remove(*flags)
            @compilerOptions.remove(*flags)
            @pchOptions.remove(*flags)
            return self
        end
        def preprocessor?() return !@preprocessor.nil? end
        def preprocessor!(preprocessor)
            Assert.expect(preprocessor, Compiler)
            @preprocessor = preprocessor
            return self
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
        def expand?(key)
            return @vars[key]
        end
        def append(other, push_front: false)
            @compiler = other.compiler unless other.compiler.nil?
            @preprocessor = other.preprocessor unless other.preprocessor.nil?
            @vars.merge!(other.vars)
            ATTRS.each do |facet|
                dst = instance_variable_get(facet)
                src = other.instance_variable_get(facet)
                if push_front
                    dst.push_front(src)
                else
                    dst.push_back(src)
                end
            end
            return self
        end
        def push_front(other) append(other, push_front: true) end
        def push_back(other) append(other, push_front: false) end
        def <<(other)
            return push_back(other)
        end
        def >>(other)
            other.vars.each{|k,v| @vars.delete(k) }
            ATTRS.each do |facet|
                dst = instance_variable_get(facet)
                src = other.instance_variable_get(facet)
                dst >> src
            end
            return self
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
        # def to_s()
        #     attrs = ATTRS.clone
        #         .delete_if{|x| instance_variable_get(x).empty? }
        #         .collect{|x| "\t#{x}: #{instance_variable_get(x)}" }
        #     return attrs.empty? ? '{}' : "{\n#{attrs.join(",\n")}\n}"
        # end
        def clear()
            @compiler = nil
            @preprocessor = nil
            @vars.clear
            ATTRS.each do |facet|
                instance_variable_get(facet).clear
            end
            return
        end
        def copy()
            newFacet = Facet.new(self)
            Assert.unreached if newFacet.frozen?
            return facet
        end
        def freeze()
            @compiler.freeze
            @preprocessor.freeze
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
            Log.debug "apply decorator <%s> to environment <%s>", self.class, environment.family
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
        def facet!(platform: nil, config: nil, compiler: nil, facet: Facet.new, &block)
            facet.instance_exec(&block) unless block.nil?
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
