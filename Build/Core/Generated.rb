# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Utils/Checksum.rb'
require_once '../Utils/MemFile.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    class Generated
        attr_reader :path, :generator
        def initialize(path, &generator)
            @path = path
            @generator = generator
        end

        ## generate a file in the output file for a target
        def generate(facet, env, target)
            key = 'Generated/'+env.generated_key(target.abs_path, @path)
            path = env.generated_path(target.expand_path(@path))

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
