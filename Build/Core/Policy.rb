# frozen_string_literal: true


require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    class Policy
        attr_reader :name, :facet
        def initialize(name, facet: Facet.new)
            @name = name.to_sym
            @facet = facet
            @customizations = []
        end

        def export!(key, value) @facet.export!(key, value); return self end
        def expand?(key) @facet.expand?(key) end
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
            when Decorator
                custom!() do |env, target|
                    Log.debug 'target <%s-%s> inherits from <%s>', target.abs_path, env.family, other.class
                    other.apply_decorator(self, env)
                end
            else
                Assert.unexpected(other)
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
                Assert.unexpected(other)
            end
        end
        def ==(other)
            case other
            when Policy
                return @facet == other.facet
            when Facet
                return @facet == other
            else
                Assert.unexpected(other)
            end
        end

        def self.match_expr?(name, expr)
            case expr
            when Symbol
                return name == expr
            when Regexp
                return name =~ expr
            when String # assume glob string
                return File.fnmatch?(expr, name.to_s)
            when Array
                return other.any?{|x| Policy.match_expr?(name, x) }
            else
                Assert.unexpected(expr)
            end
        end
        def match?(expr)
            return Policy.match_expr?(@name, expr)
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
                        Assert.unexpected(options)
                    end
                end
            end
        end

        ## cache method result inside configuration
        def cached_attribute(name, &memoizer)
            key = "Cached.#{self.name}.#{name}"
            prereq = Build.fetch_persistent_opt(key)
            unless prereq
                prereq = Prerequisite.new(key, nil, memoizer)
                prereq = Build.make_persistent_opt(prereq)
            end
            return prereq.available?
        end

        ## customize final environment + target
        def customize(facet, env, target)
            Log.debug 'customize target <%s> with environement <%s> and %s <%s>', target.abs_path, env.family, self.class, @name
            @customizations.each do |custom|
                facet.instance_exec(env, target, &custom)
            end
        end

        ## decorate shared environment facet
        def decorate(facet, env)
            Log.debug 'decorate facet with environement <%s> and %s <%s>', env.family, self.class, @name
            facet.push_front(@facet)
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
