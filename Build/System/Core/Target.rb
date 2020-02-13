
require './Common.rb'

require 'set'

module Build

    class Target < Decorator
        attr_reader :name, :namespace, :type, :link

        attr_reader :public_dependencies
        attr_reader :private_dependencies
        attr_reader :runtime_dependencies

        attr_accessor :source_path, :source_glob

        attr_reader :source_files, :isolated_files, :excluded_files
        attr_reader :extra_files

        def initialize(name, namespace, type, link)
            super()

            @name = name
            @namespace = namespace
            @type = type
            @link = link

            @source_path = self.abs_path()
            @source_glob = '*.c,*.cpp'
            @excluded_files = Set.new

            @source_files = Set.new
            @isolated_files = Set.new
            @extra_files = Set.new

            @public_dependencies = []
            @private_dependencies = []
            @runtime_dependencies = []

            @all_public_dependencies = nil
            @all_runtime_dependencies = nil
        end

        def abs_path() "#{@namespace}/#{@name}" end
        def public_path() abs_path << '/Public' end
        def private_path() abs_path << '/Private' end

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

        def glob!(path: @source_path, glob: @source_glob)
            @source_path = path
            @source_glob = glob
            return self
        end

        def source_files!(*filenames) @source_files.append(filenames); return self end
        def isolated_files!(*filenames) @isolated_files.append(filenames); return self end
        def excluded_files!(*filenames) @excluded_files.append(filenames); return self end
        def extra_files!(*filenames) @extra_files.append(filenames); return self end

        def depends!(other, visibility=:private)
            case visibility
            when :public
                @public_dependencies << other unless @public_dependencies.include?(other)
            when :private
                @private_dependencies << other unless @private_dependencies.include?(other)
            when :runtime
                @runtime_dependencies << other unless @runtime_dependencies.include?(other)
            else
                raise ArgumentError.new('invalid visibility')
            end
            return self
        end

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
