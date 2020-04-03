# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Facet.rb'

require 'set'

module Build

    class Target < Policy
        attr_reader :namespace, :type, :link

        attr_reader :public_dependencies
        attr_reader :private_dependencies
        attr_reader :runtime_dependencies

        attr_reader :abs_path, :var_path

        attr_accessor :source_path
        attr_accessor :unity_num_files
        attr_accessor :glob_patterns
        attr_reader :force_includes # not relative to source path

        def self.relative_path(*names)
            names.each do |name|
                ivar = "@#{name}".to_sym
                define_method(name) do
                    expand_path(instance_variable_get(ivar))
                end
                define_method("rel_#{name}") do
                    instance_variable_get(ivar)
                end
            end
        end

        relative_path :glob_path
        relative_path :pch_header, :pch_source
        relative_path :source_files, :isolated_files, :excluded_files
        relative_path :extra_files

        def initialize(name, namespace, type, link, &config)
            super(name)

            @namespace = namespace
            @type = type
            @link = link

            @abs_path = File.join(@namespace.to_s, @name.to_s)
            @var_path = @abs_path.tr('/-', '_')

            @source_path = @abs_path

            @glob_path = ''
            @glob_patterns = %w{ *.c *.cpp }
            @unity_num_files = 1

            @pch_header = nil
            @pch_source = nil

            @excluded_files = Set.new
            @source_files = Set.new
            @isolated_files = Set.new
            @extra_files = Set.new
            @force_includes = Set.new

            @public_dependencies = []
            @private_dependencies = []
            @runtime_dependencies = []

            @_all_dependencies = nil
            @_ordinal = nil

            @config = config

            Log.verbose 'new target %s <%s>', @type, abs_path
        end

        def executable?() @type == :executable end
        def library?() @type == :library end
        def shared?() @type == :shared end

        def configure!()
            if @config
                instance_exec(&@config)
                @config = nil
            end
            return self
        end

        def customize(facet, env, target)
            facet << @facet
            super(facet, env, target)

            @namespace.customize(facet, env, target)

            facet.includePaths <<
                env.source_path(self.source_path) <<
                env.source_path(self.public_path)

            private_path = env.source_path(self.private_path)
            facet.includePaths << private_path if Dir.exist?(private_path)

            self.all_dependencies do |(dep, visibility)|
                facet.includePaths << env.source_path(dep.public_path)

                case visibility
                when :public
                    facet.includes << dep.includes
                    dep.force_includes.each do |header|
                        facet.includes << env.source_path(header)
                    end
                end
            end

            self.force_includes.each do |header|
                facet.includes << env.source_path(header)
            end

            return facet
        end

        def expand_path(path)
            case path
            when NilClass
                return nil
            when String
                return path == File.absolute_path(path) ? path : File.join(@source_path, path)
            when Array
                path.collect{|x| expand_path(x) }
            when Set
                a = path.to_a
                a.collect!{|x| expand_path(x) }
                return a
            else
                Log.fatal 'unsupported path type: %s', path.inspect
            end
        end

        def public_path() expand_path('Public') end
        def private_path() expand_path('Private') end

        def to_s() abs_path end

        def link!(type)
            case type
            when :static, :dynamic
                @link = type
            else
                raise ArgumentError.new("unsupported link type: #{type}")
            end
            return self
        end

        def glob!(path: @glob_path, glob: @glob_patterns)
            @glob_path = path
            glob = [ glob ] unless glob.is_a?(Array)
            @glob_patterns = glob
            return self
        end

        def pch?() not (@pch_header.nil? and @pch_source.nil?) end
        def pch!(header, source)
            @pch_header = header
            @pch_source = source
            return self
        end

        def source_files!(*filenames) @source_files.merge(filenames.flatten); return self end
        def isolated_files!(*filenames) @isolated_files.merge(filenames.flatten); return self end
        def excluded_files!(*filenames) @excluded_files.merge(filenames.flatten); return self end
        def extra_files!(*filenames) @extra_files.merge(filenames.flatten); return self end
        def force_includes!(*filenames) @force_includes.merge(filenames.flatten); return self end

        def all_source_files() return (self.source_files + self.isolated_files) end
        def unity_excluded_files() return (self.isolated_files + self.excluded_files) end

        def depends!(*others, visibility: :private)
            case visibility
            when :public
                dst = @public_dependencies
            when :private
                dst = @private_dependencies
            when :runtime
                dst = @runtime_dependencies
            else
                raise ArgumentError.new('invalid visibility')
            end
            others.each do |target|
                Log.fatal 'expected a target, not: %s', target.inspect unless target.is_a?(Target)
                if $DEBUG and (
                    @public_dependencies.include?(target) or
                    @private_dependencies.include?(target) or
                    @runtime_dependencies.include?(target) ) then
                    Log.fatal 'target already referenced: %s', target.abs_path,  dst.include?(target)
                end
                dst << target
            end
            return self
        end
        def public_deps!(*others) depends!(*others, visibility: :public) end
        def private_deps!(*others) depends!(*others, visibility: :private) end
        def runtime_deps!(*others) depends!(*others, visibility: :runtime) end

        def all_public_dependencies(&block) all_dependencies(:public, &block) end
        def all_private_dependencies(&block) all_dependencies(:private, &block) end
        def all_runtime_dependencies(&block) all_dependencies(:runtime, &block) end

        def all_dependencies(scope=nil)
            if @_all_dependencies.nil?
                visiteds = Hash.new
                dependency_visitor_(visiteds, self)
                @_all_dependencies = visiteds.values
                @_all_dependencies.sort!{|a, b| a.first.ordinal <=> b.first.ordinal }
            end
            if block_given?
                if scope.nil?
                    @_all_dependencies.each{|it| yield it }
                else
                    @_all_dependencies.each do |(dep, visibility)|
                        yield dep if visibility == scope
                    end
                end
            else
                Assert.check{ scope.nil? }
            end
            return @_all_dependencies
        end

        @@_ordinal_cnt = 0
        def ordinal()
            if @_ordinal.nil?
                deps = []
                self.private_dependencies.each{|x| deps << x }
                self.public_dependencies.each{|x| deps << x }
                self.runtime_dependencies.each{|x| deps << x }
                deps.sort!{|a, b| a.abs_path <=> b.abs_path }
                deps.each{|x| x.ordinal }
                @_ordinal = (@@_ordinal_cnt += 1)
            end
            return @_ordinal
        end

    private
        def dependency_visitor_(result, target, depth: 0)
            target.private_dependencies.each do |dep|
                result[dep] = [dep, :private]
            end if 0 == depth
            target.public_dependencies.each do |dep|
                unless result.include?(dep)
                    result[dep] = [dep, :public]
                    dep.all_dependencies do |it|
                        next if it.last == :private
                        next if result.include?(it.first)
                        result[it.first] = it
                    end
                end
            end
            target.runtime_dependencies.each do |dep|
                unless result.include?(dep)
                    result[dep] = [dep, :runtime]
                    dep.all_dependencies do |it|
                        next if it.last == :private
                        next if result.include?(it.first)
                        result[it.first] = it
                    end
                end
            end
            return
        end

    end #~ Target

end #~  Build
