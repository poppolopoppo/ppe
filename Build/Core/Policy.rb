# frozen_string_literal: true

require_once '../Common.rb'

module Build

    class Policy
        attr_reader :name, :facet
        def initialize(name, facet: Facet.new)
            @name = name.to_sym
            @facet = facet
            @customizations = []
        end

        def export!(key, value) @facet.export!(key, value); return self end
        def tag?(*tags) @facet.tag?(*tags) end
        def compilationFlag!(*flags) @facet.compilationFlag!(*tags); return self end

        def to_s() @name.to_s end
        def freeze()
            @name.freeze
            @facet.freeze
            @customizations.freeze
            super()
        end

        def <<(other)
            case other
            when Policy
                @facet << other.facet
            when Facet
                @facet << other
            else
                Log.fatal 'unsupported type <%s>', other.class
            end
            return self
        end
        def inherits!(other)
            return self.<<(other)
        end
        def >>(other)
            case other
            when Policy
                @facet >> other.facet
            when Facet
                @facet >> other
            else
                Log.fatal 'unsupported type <%s>', other.class
            end
        end
        def ==(other)
            case other
            when Policy
                return @facet == other.facet
            when Facet
                return @facet == other
            else
                Log.fatal 'unsupported type <%s>', other.class
            end
        end

        def match?(expr)
            case expr
            when Regexp
                return @name =~ expr
            when String # assume glob string
                return File.fnmatch?(expr, @name.to_s)
            else
                raise ArgumentError.new('unexpected match expression')
            end
        end
        def custom!(&block)
            @customizations << block
            return self
        end
        def on_tag!(tag, options)
            custom!() do |env, target|
                if tag?(tag)
                    case options
                    when Facet
                        self << options
                    when Hash
                        set!(options)
                    else
                        raise ArgumentError.new("unexpected options: #{options.inspect}")
                    end
                end
            end
        end

        ## customize final environment + target
        def customize(facet, env, target)
            Log.debug 'customize target <%s> with environement <%s> and policy <%s>', target.abs_path, env.family, @name
            @customizations.each do |custom|
                facet.instance_exec(env, target, &custom)
            end
        end

        ## decorate shared environment facet
        def decorate(facet, env)
            Log.debug 'decorate facet with environement <%s> and policy <%s>', env.family, @name
            facet << @facet
        end

        Facet::SETS.each do |facet|
            ivar = "@#{Facet.pluralize(facet)}".to_sym
            define_method(Facet.pluralize(facet)) do
                @facet.instance_variable_get(ivar)
            end
            define_method(facet.to_s<<'!') do |*values|
                f = @facet.instance_variable_get(ivar)
                values.each {|v| f.append(v) }
                self
            end
            define_method("no_#{facet}!") do |*values|
                f = @facet.instance_variable_get(ivar)
                values.each {|v| f.remove(v) }
                self
            end
        end

    end #~ Policy

end #~ Build
