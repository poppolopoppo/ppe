
require 'set'

module Build

    module Log
        LEVELS = [
            :debug,
            :verbose,
            :info,
            :warning,
            :error,
            :fatal
        ]
        VERBOSITY = [ :info, :warning, :error, :fatal ]

        $show_caller = false
        $show_timestamp = true

        def self.elapsed?
            $started_at = Time.now if $started_at.nil?
            return (Time.now.to_f - $started_at.to_f) * 1000.0
        end

        def self.verbosity(min = :info)
            VERBOSITY.clear
            LEVELS.reverse.each do |level|
                VERBOSITY << level
                return if level == min
            end
            raise ArgumentError.new("unknown verbosity level: #{min}")
        end

        def self.puts(message: '', verbosity: :info)
            log = ($show_timestamp ? ("[%010.5f]" % elapsed?()) : '') <<
                "[#{verbosity.to_s.rjust(7)}]  " <<
                message.to_s

            case verbosity
            when :debug, :verbose, :info
                outp = $stdout
            when :warning, :error
                outp = $stderr
            when :fatal
                raise RuntimeError.new('fatal: '<<message)
            else
                raise ArgumentError.new("unsupported log verbosity: #{verbosity}")
            end

            outp.puts(log)
            if $show_caller
                outp.puts("\t at: "<<caller[1].to_s)
            end
            return
        end
        LEVELS.each do |level|
            define_singleton_method(level) do |message|
                Log.puts(message: message, verbosity: level)
            end
        end
    end #~ Log

    class ValueSet
        attr_reader :data
        def initialize() @data = [] end
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
            :preprocessor,
            :compiler,
            :linker,
            :tag ]
        ATTRS = SETS.collect{|x| ('@'<<x.to_s<<'s').to_sym }
        attr_reader *SETS.collect{|x| (x.to_s<<'s').to_sym }
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
    end #~ Facet

    class BuildFacet
        attr_reader :name, :facet
        def initialize(name, facet: Facet.new)
            @name = name
            @facet = facet
        end
        def +(other) @facet + other.facet; return self end
        def -(other) @facet - other.facet; return self end
        def ==(other) @facet == other.facet; return self end
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
        def handle_tag(facet, tag)
            return false
        end
        Facet::SETS.each do |facet|
            aname = ('@'<<facet.to_s<<'s').to_sym
            define_method(facet) do |*values|
                f = @facet.instance_variable_get(aname)
                values.each {|v| f.append(v) }
                return self
            end
        end
    end #~ BuildFacet

    class Platform < BuildFacet
        attr_reader :target, :os
        def initialize(name, target, os)
            super(name)
            @target = target
            @os = os
        end
    end #~ Platform

    class Configuration < BuildFacet
        attr_reader :link
        def initialize(name, link)
            super(name)
            @link = link
        end
    end #~ Configuration

    class Compiler < BuildFacet
        attr_reader :compiler, :linker, :extra_files
        def initialize(name, compiler, linker, *extra_files)
            super(name)
            @compiler = compiler
            @linker = linker
            @extra_files = extra_files
        end
        def add_forceInclude(facet, filename) raise RuntimeError("unsupported") end
        def add_includePath(facet, dirpath) raise RuntimeError("unsupported") end
        def add_library(facet, filename) raise RuntimeError("unsupported") end
        def add_libraryPath(facet, dirpath) raise RuntimeError("unsupported") end
    end #~ Compiler

    class Decorator
        attr_reader :facets
        def initialize()
            @facets = []
        end
        def set(facet, &filter)
            @facets << [filter, facet]
        end
        def eval(output, environment)
            @facets.each do |(filter, facet)|
                output + facet if filter.call(environment)
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
        def depends!(other, visibility=:private)
            case visibility
            when :public
                @public_dependencies << other
            when :private
                @private_dependencies << other
            when :runtime
                @runtime_dependencies << other
            else
                raise ArgumentError.new('invalid visibility')
            end
            return self
        end
    end #~ Decorator

    class Target < Decorator
        attr_reader :name, :namespace, :type, :link

        attr_reader :source_path, :source_glob, :excluded_files
        attr_reader :source_files, :isolated_files, :extra_files

        attr_reader :public_dependencies
        attr_reader :private_dependencies
        attr_reader :runtime_dependencies

        def initialize(name, namespace, type, link)
            super()

            @name = name
            @namespace = namespace
            @type = type
            @link = link

            @source_path = nil
            @source_glob = '*.c,*.cpp'
            @excluded_files = Set.new

            @source_files = Set.new
            @isolated_files = Set.new
            @extra_files = Set.new

            @public_dependencies = Set.new
            @private_dependencies = Set.new
            @runtime_dependencies = Set.new
        end

        def abs_path() "#{@namespace}/#{@name}" end
        def public_path() abs_path << '/Public' end
        def private_path() abs_path << '/Private' end
    end #~ Target

    class Environment < Decorator
        attr_reader :name, :platform, :config, :compiler
        def initialize(name, platform, config, compiler)
            super()

            @name = name
            @platform = platform
            @config = config
            @compiler = compiler

            @buildfacet = Facet.new
            @buildfacet + @platform.facet + @config.facet + @compiler.facet
        end
        def expand(target)
            final = Facet.new + @buildfacet
            target.eval(final, self)
            self.eval(final, self)
            return final
        end
        def handle_tag(facet, tag)
            return (@platform.handle_tag(facet, tag) or
                    @config.handle_tag(facet, tag) or
                    @compiler.handle_tag(facet, tag))
        end
    end #~ Environment

    module Namespace
        def construct_path(name)
            puts self
            return instance_eval() {
                puts self;
                 Module.nesting }.
                collect do |x|
                    puts "Nested: #{x}";
                    x.const_defined?(name) ?
                        x.const_get(name) : nil
                end.
                delete_if{|x| puts x; x.nil? }.
                join('/')
        end
        def namespace_full()
            unless self.const_defined?(:NamespaceFull)
                n = self.construct_path(:Namespace)
                self.const_set(:NamespaceFull, n)
            end
            return self.const_get(:NamespaceFull)
        end
        def external(name)
            return Target.new(name, self.namespace_full(), :external, :static)
        end
        def library(name, link: nil)
            return Target.new(name, self.namespace_full(), :library, link)
        end
        def executable(name, facets: nil)
            return Target.new(name, self.namespace_full(), :executable, :static)
        end
    end #~ Namespace

    module SharedPlatforms
        X86 = Platform.new('x86', 'x86', :dummy)
        X64 = Platform.new('x64', 'x64', :dummy)
        ARM = Platform.new('arm', 'arm', :dummy)
        ALL = [ X86, X64 ]
    end #~ SharedPlatforms

    module SharedConfigs
        Debug = Configuration.new('Debug', :static)
        FastDebug = Configuration.new('FastDebug', :dynamic)
        Release = Configuration.new('Release', :static)
        Profiling = Configuration.new('Profiling', :static)
        Final = Configuration.new('Final', :static)
        ALL = [ Debug, FastDebug, Release, Profiling, Final ]
    end #~ SharedConfigs

    module SharedCompilers
        Dummy = Compiler.new('Dummy', nil, nil)
    end #~ SharedCompilers

    module SharedEnvironments
        def self.bootstrap(targetModule, compiler, platformsModule, configsModule)
            all = []
            platformsModule.const_get(:ALL).each do |platform|
                configsModule.const_get(:ALL).each do |config|
                    name = "#{compiler.name}_#{platform.name}_#{config.name}"
                    env = Environment.new(name, platform, config, compiler)
                    targetModule.const_set(name.to_sym, env)
                    all << env
                end
            end
            targetModule.const_set(:ALL, all)
        end
        bootstrap(self, SharedCompilers::Dummy, SharedPlatforms, SharedConfigs)
    end #~ SharedEnvironments

    def self.expand_target(environment, target)
        expanded = environment.expand(target)
        expanded.tags.each do |tag|
            Log.warning("unhandled tag @#{tag} inside <#{environment.name}> for target '#{target.abs_path}'") unless
                environment.handle_tag(expanded, tag)
        end
        return expanded
    end

end #~ Build

require 'pp'

Build::SharedEnvironments::ALL.each do |env|
    Build::Log.info env.name
end

Build::SharedConfigs::Debug.define("_DEBUG").compiler('/Od').tag(:debug)
#pp Build::SharedConfigs::Debug.clone

FastPDBLinking = Build::Facet.new linker: '/LINK:FASTPDB'

module PPE
    Namespace = 'PPE'
    extend Build::Namespace

    module Runtime
        Namespace = 'Runtime'
        extend Build::Namespace

        Core = library('Core')
        VFS = library('VFS').
            depends!(Core, :public).
            filter!(config: 'Deb*', options: {
                compiler: '/Od',
                includePath: 'Runtime/Core/Public'
            }).
            filter!(platform: 'x64', options: {
                define: ['ARCH_X64', 'ARCH=X64'],
                tag: :x64
            }).
            facet!(config: 'Release', facet: FastPDBLinking)

    end #~ Runtime

end #~ PPE

pp Build::expand_target(Build::SharedEnvironments::Dummy_x64_Debug, PPE::Runtime::VFS)
pp Build::expand_target(Build::SharedEnvironments::Dummy_x64_Release, PPE::Runtime::VFS)

