
require './Common.rb'

module Build

    module Namespace
        def parent()
            return @build_parent
        end
        def namespace()
            return @build_namespace.join('/')
        end
        def all()
            return @build_all
        end
        def targets()
            return @build_targets
        end
        def external(name)
            make_target(name, :external, :static)
        end
        def library(name, link: nil)
            make_target(name, :library, link)
        end
        def executable(name)
            make_target(name, :executable, :static)
        end
        def append_target(target)
            self.all() << target
            self.parent().append_target(target) if self.parent()
            return self
        end
        def make_target(name, type, link)
            target = Target.new(name, self.namespace(), type, link)
            self.targets() << target
            self.append_target(target)
            return target
        end
        def self.extended(base)
            path = []
            parent = nil

            ancestors = base.name.split('::').reverse
            name = ancestors[0]
            ancestors = ancestors[1..-1]

            ancestors.length.times do |i|
                parent = instance_eval(ancestors[0..i].join('::'))
                if parent.instance_variable_defined?(:@build_namespace)
                    path = parent.instance_variable_get(:@build_namespace).clone
                    break unless path.nil?
                end
                path = []
                parent = nil
            end

            path << name
            Log.debug "new namespace: #{path.join('/')} -> #{base}"

            base.instance_variable_set(:@build_all, [])
            base.instance_variable_set(:@build_targets, [])
            base.instance_variable_set(:@build_parent, parent)
            base.instance_variable_set(:@build_namespace, path)
        end
    end #~ Namespace

end #~ Build
