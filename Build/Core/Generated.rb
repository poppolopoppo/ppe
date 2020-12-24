# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Utils/Checksum.rb'
require_once '../Utils/MemFile.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    class Generated
        attr_reader :path, :generator
        def initialize(path, scope, &generator)
            @path = path
            @scope = scope
            @generator = generator
        end

        ## generate a file in the output file for a target
        def generate(facet, env, target)
            case @scope
            when :generated
                key = 'Generated.'+env.generated_key(target.abs_path, @path)
                path = env.generated_path(target.expand_path(@path))
            when :public
                key = 'Public.'+File.join(target.abs_path, @path)
                path = env.source_path(target.expand_path(File.join('Public', @path)))
            when :private
                key = 'Private.'+File.join(target.abs_path, @path)
                path = env.source_path(target.expand_path(File.join('Private', @path)))
            else
                Log.fatal 'unsupported generated scope <%s>', @scope
            end

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
