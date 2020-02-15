
require './Common.rb'

module Build

    class Environment < Decorator
        attr_reader :name, :platform, :config, :compiler
        def initialize(name, platform, config, compiler)
            super()
            @name = name
            @platform = platform
            @config = config
            @compiler = compiler
            @memoized = nil
        end
        def facet()
            if @memoized.nil?
                @memoized = Facet.new
                @memoized << @platform.facet << @config.facet << @compiler.facet

                apply_decorator(@memoized, self)

                @platform.customize(@memoized, self)
                @config.customize(@memoized, self)
                @compiler.customize(@memoized, self)
            end
            return @memoized
        end
        def expand(target)
            expanded = facet().clone
            target.apply_decorator(expanded, self)

            @compiler.add_includePath(expanded, target.public_path)

            target.all_public_dependencies do |dep|
                @compiler.add_includePath(expanded, dep.public_path)
            end
            target.all_private_dependencies do |dep|
                @compiler.add_includePath(expanded, dep.public_path)
            end

            return expanded
        end
    end #~ Environment

    def make_environment(compiler, platformsModule, configsModule)
        all = []
        platformsModule.const_get(:ALL).each do |platform|
            configsModule.const_get(:ALL).each do |config|
                name = "#{compiler.name}_#{platform.name}_#{config.name}"
                env = Environment.new(name, platform, config, compiler)
                self.const_set(name.to_sym, env)
                all << env
            end
        end
        self.const_set(:Environments, all)
    end

end #~ Build
