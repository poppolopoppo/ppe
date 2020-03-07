
require_once '../Common.rb'
require_once '../Core/Facet.rb'

module Build

    class Namespace < Policy
        ALL = {}

        attr_reader :parent, :path, :children
        def initialize(name, parent: nil)
            super(name)
            @all_targets = []
            @self_targets = []
            @parent = parent
            @children = {}
            if @parent.nil?
                @path = @name.to_s
                @root = self
            else
                inherits!(@parent)
                @path = @parent.root? ? @name.to_s : File.join(@parent.path, @name.to_s)
                @root = @parent
                loop do
                    break if @root.root?
                    @root = @root.parent
                end
            end
        end

        def root?() @parent.nil? end

        def all() @all_targets end
        def targets() @self_targets end

        def to_s() @path end

        def external!(name, &cfg) make_target!(name, :external, :static, &cfg) end
        def library!(name, link: nil, &cfg) make_target!(name, :library, link, &cfg) end
        def executable!(name, &cfg) make_target!(name, :executable, :static, &cfg) end

        def namespace(name, &block)
            child = Namespace.new(name, parent: self)
            ancestor = $Build
            $Build = child
            define_singleton_method(name) { child }
            @children[name] = child
            child.instance_exec(&block)
            $Build = ancestor
            return self
        end

        def [](*names, &block)
            result = @root
            names.each{|x| result = result / x }
            return block.nil? ? result : result.instance_exec(result, &block)
        end
        def /(name) return @children[name] end

        def append!(target)
            @all_targets << target
            parent.append!(target) unless @parent.nil?
            return self
        end
        def make_target!(name, type, link, &config)
            target = Target.new(name, self, type, link, &config)
            @self_targets << target
            append!(target)
            name = name.to_s
            name.gsub!('-', '_')
            define_singleton_method(name) { target }
            target.configure!
            return self
        end

        def include!(*relpaths)
            rootpath = File.dirname(caller_locations(1, 1)[0].path)
            rootpath = File.realpath(rootpath)

            relpaths.each do |relpath|
                abspath = File.join(rootpath, relpath)
                Log.verbose 'include "%s"', abspath
                require abspath
            end
        end

    end #~ Namespace

    def namespace(name, &block)
        Build.const_memoize(self, name) do
            result = Build::Namespace.new(name)
            $Build = result
            result.instance_exec(&block)
            $Build = nil
            result
        end
    end

end #~ Build
