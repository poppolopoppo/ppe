# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Facet.rb'
require_once '../Core/Policy.rb'

module Build

    class Compiler < Policy
        attr_reader :executable, :librarian, :linker, :extra_files
        def initialize(name, executable, librarian, linker, *extra_files)
            super(name)
            @executable = executable
            @librarian = librarian
            @linker = linker
            @extra_files = extra_files
        end

        def family() nil end

        def ext_for(output)
            case output
            when :executable
                return self.ext_binary
            when :debug
                return self.ext_debug
            when :library
                return self.ext_library
            when :obj
                return self.ext_obj
            when :pch
                return self.ext_pch
            when :shared
                return self.ext_shared
            else
                Log.fatal 'unknown output type "%s"', output
            end
        end

        def ext_binary() Assert.not_implemented end
        def ext_debug() Assert.not_implemented end
        def ext_library() Assert.not_implemented end
        def ext_obj() Assert.not_implemented end
        def ext_pch() Assert.not_implemented end
        def ext_shared() Assert.not_implemented end

        def add_linkType(facet, link) Log.error("%s: add_linkType('%s') is not implemented", @name, link) end
        def add_define(facet, key, value=nil) Log.error("%s: add_define('%s', '%s') is not implemented", @name, key, value) end
        def add_forceInclude(facet, filename) Log.error("%s: add_forceInclude('%s') is not implemented", @name, filename) end
        def add_includePath(facet, dirpath) Log.error("%s: add_includePath('%s') is not implemented", @name, dirpath) end
        def add_externPath(facet, dirpath) Log.error("%s: add_externPath('%s') is not implemented", @name, dirpath) end
        def add_systemPath(facet, dirpath) Log.error("%s: add_systemPath('%s') is not implemented", @name, dirpath) end
        def add_library(facet, filename) Log.error("%s: add_library('%s') is not implemented", @name, filename) end
        def add_libraryPath(facet, dirpath) Log.error("%s: add_libraryPath('%s') is not implemented", @name, dirpath) end

        def add_compilerOption(facet, *tokens)
            tokens.each do |token|
                #facet.analysisOptions << token
                facet.compilerOptions << token
                facet.pchOptions << token
                facet.preprocessorOptions << token
            end
        end

        def decorate(facet, env)
            super(facet, env)

            facet.compiler!(self)
        end

        def customize(facet, env, target)
            super(facet, env, target)

            add_linkType(facet, target.link.nil? ? env.config.link : target.link)

            facet.defines.each{|x| add_define(facet, x) }
            facet.systemPaths.each{|x| add_systemPath(facet, x) }
            facet.externPaths.each{|x| add_externPath(facet, x) }
            facet.includePaths.each{|x| add_includePath(facet, x) }
            facet.includes.each{|x| add_forceInclude(facet, x) }
            facet.libraryPaths.each{|x| add_libraryPath(facet, x) }
            facet.librarys.each{|x| add_library(facet, x) }
        end

        def freeze()
            @executable.freeze
            @librarian.freeze
            @linker.freeze
            @extra_files.freeze
            super()
        end

    end #~ Compiler

    module SharedCompilers
        Dummy = Compiler.new('Dummy', nil, nil, nil)
    end #~ SharedCompilers

    make_facet(:Compiler_PCHEnabled) do
        defines << 'BUILD_PCH=1'
    end
    make_facet(:Compiler_PCHDisabled) do
        defines << 'BUILD_PCH=0'
    end

end #~ Build
