
require './Common.rb'

module Build

    class SourceControl
        def initialize(path)
            @path = path
        end
        def modified_files?()
            Log.error("%s: modified_files?() is not implemented", self.class) end
        end
    end #~ SourceControl


end #~ Build
