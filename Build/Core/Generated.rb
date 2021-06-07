# frozen_string_literal: true


require_once '../Utils/Checksum.rb'
require_once '../Utils/MemFile.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    class Generated
        attr_reader :path, :scope, :generator
        def initialize(path, scope, &generator)
            @path = path
            @scope = scope
            @generator = generator
        end

        def public?() return @scope == :public end

        def scope_path()
            case @scope
            when :public
                return 'Public'
            when :private
                return 'Private'
            else
                Log.fatal 'unsupported generated scope <%s>', @scope
            end
        end

        def generated_path(env, target)
            return env.generated_path(target.expand_path(self.scope_path))
        end

        ## generate a file in the output file for a target
        def generate(facet, env, target)
            key = "Generated."+env.generated_key(target.abs_path, self.scope_path, @path)
            path = File.join(generated_path(env, target), @path)

            Log.debug('generating "%s" for target <%s>', path, target)

            prereq = Build.fetch_persistent_opt(key)
            unless prereq
                prereq = Prerequisite.new(key, nil, lambda{ FileChecksum.from(path) })
                prereq.validate!{|x| x.validate_checksum! }
                prereq = Build.make_persistent_opt(prereq)
            end

            io = MemFile.new(path)
            target.instance_exec(facet, env, io, &@generator)

            return io.export_ifn?(prereq.available?)
        end

    end #~ Generated

end #~ Build
