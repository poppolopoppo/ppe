
require './Common.rb'
require './Core/Policy.rb'

module Build

    class Compiler < Policy
        attr_reader :compiler, :linker, :extra_files
        def initialize(name, compiler, linker, *extra_files)
            super(name)
            @compiler = compiler
            @linker = linker
            @extra_files = extra_files
        end
        def add_forceInclude(facet, filename) Log.error("%s: add_forceInclude('%s') is not implemented", @name, filename) end
        def add_includePath(facet, dirpath) Log.error("%s: add_includePath('%s') is not implemented", @name, dirpath) end
        def add_library(facet, filename) Log.error("%s: add_library('%s') is not implemented", @name, filename) end
        def add_libraryPath(facet, dirpath) Log.error("%s: add_libraryPath('%s') is not implemented", @name, dirpath) end
    end #~ Compiler

    module SharedCompilers
        Dummy = Compiler.new('Dummy', nil, nil)
    end #~ SharedCompilers

end #~ Build
