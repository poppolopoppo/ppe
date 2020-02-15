
require './Common.rb'

module Build

    class Policy
        attr_reader :name, :facet
        def initialize(name, facet: Facet.new)
            @name = name
            @facet = facet
            @customizations = []
        end
        def <<(other) @facet << other.facet; return self end
        def >>(other) @facet >> other.facet; return self end
        def ==(other) return @facet == other.facet end
        def match?(expr)
            case expr
            when Regexp
                return @name =~ expr
            when String # assume glob string
                return File.fnmatch?(expr, @name)
            else
                raise ArgumentError.new('unexpected match expression')
            end
        end
        def custom!(&block)
            @customizations << block
            return self
        end
        def on_tag!(tag, options)
            custom!() do |facet, env|
                if facet.tag?(tag)
                    case options
                    when Facet
                        facet << options
                    when Hash
                        facet.set(options)
                    else
                        raise ArgumentError.new("unexpected options: #{options.inspect}")
                    end
                end
            end
        end
        def customize(facet, env)
            @customizations.each do |custom|
                custom.call(facet, env)
            end
        end

        Facet::SETS.each do |facet|
            ivar = ('@'<<facet.to_s<<'s').to_sym
            define_method(facet.to_s<<'!') do |*values|
                f = @facet.instance_variable_get(ivar)
                values.each {|v| f.append(v) }
                return self
            end
            define_method('no_'<<facet.to_s<<'!') do |*values|
                f = @facet.instance_variable_get(ivar)
                values.each {|v| f.remove(v) }
                return self
            end
        end
    end #~ Policy

end #~ Build
