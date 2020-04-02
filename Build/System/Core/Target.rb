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

            @all_public_dependencies = nil
            @all_runtime_dependencies = nil

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
                env.source_path(self.public_path) <<
                env.source_path(self.private_path)

            self.force_includes.each do |header|
                facet.includes << env.source_path(header)
            end

            expand_dep = lambda do |dep|
                facet.includes << dep.includes
                facet.includePaths << env.source_path(dep.public_path)

                dep.force_includes.each do |header|
                    facet.includes << env.source_path(header)
                end
            end

            self.all_public_dependencies.each(&expand_dep)
            self.private_dependencies.each(&expand_dep)

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

        def all_public_dependencies()
            if @all_public_dependencies.nil?
                @all_public_dependencies = []
                queue = [ self ]
                while m = queue.pop
                    m.public_dependencies.each do |dep|
                        unless @all_public_dependencies.include?(dep)
                            @all_public_dependencies << dep
                            queue << dep
                        end
                    end
                end
            end
            @all_public_dependencies.each{ |dep| yield(dep) } if block_given?
            return @all_public_dependencies
        end
        def all_private_dependencies()
            @private_dependencies.each{ |dep| yield(dep) } if block_given?
            return @private_dependencies
        end
        def all_runtime_dependencies()
            if @all_runtime_dependencies.nil?
                @all_runtime_dependencies = []
                queue = [ self ]
                while m = queue.pop
                    m.runtime_dependencies.each do |dep|
                        unless @all_runtime_dependencies.include?(dep)
                            @all_runtime_dependencies << dep
                            queue << dep
                        end
                    end
                end
            end
            @all_runtime_dependencies.each{ |dep| yield(dep) } if block_given?
            return @all_runtime_dependencies
        end

    end #~ Target

end #~  Build
