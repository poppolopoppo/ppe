
require_once '../Common.rb'

module Build

    class Policy
        attr_reader :name, :facet
        def initialize(name, facet: Facet.new)
            @name = name.to_sym
            @facet = facet
            @customizations = []
        end

        def tag!(*tags) @facet.tag!(*tags) end
        def tag?(*tags) @facet.tag?(*tags) end

        def to_s() @name.to_s end

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
            custom!() do |facet, env, target|
                if facet.tag?(tag)
                    case options
                    when Facet
                        facet << options
                    when Hash
                        facet.set!(options)
                    else
                        raise ArgumentError.new("unexpected options: #{options.inspect}")
                    end
                end
            end
        end
        def customize(facet, env, target)
            @customizations.each do |custom|
                instance_exec(facet, env, target, &custom)
            end
        end

        Facet::SETS.each do |facet|
            ivar = ('@'<<facet.to_s<<'s').to_sym
            define_method(facet.to_s<<'s') do
                @facet.instance_variable_get(ivar)
            end
            define_method(facet.to_s<<'!') do |*values|
                f = @facet.instance_variable_get(ivar)
                values.each {|v| f.append(v) }
                self
            end
            define_method('no_'<<facet.to_s<<'!') do |*values|
                f = @facet.instance_variable_get(ivar)
                values.each {|v| f.remove(v) }
                self
            end
        end
    end #~ Policy

end #~ Build
