# frozen_string_literal: true

require_once '../../Common.rb'

require_once './WindowsSDK.rb'

module Build

    class WindowsCompilerRC < Compiler
        def initialize(prefix, rc_exe)
            super(prefix, rc_exe, nil, nil)
            @facet.compilerOptions << '/nologo'
        end
        def family() :custom end
        def ext_obj() '.res' end
        def add_linkType(facet, link)
            # nothing to do
        end
        def add_forceInclude(facet, filename)
            # nothing to do
        end
        def add_define(facet, key, value=nil)
            token = value.nil? ? key : "#{key}=#{value}"
            add_compilationFlag(facet, "/d#{token}")
        end
        def add_includePath(facet, dirpath)
            add_compilationFlag(facet, "/i\"#{dirpath}\"")
        end
        def customize(facet, env, target)
            super(facet, env, target)

            # generate minimal resources for DLLs
            if target.library? && target.dynamic?
                facet.compilerOptions << '/q'
            end

            # .rc file must be the last parameter
            facet.compilerOptions << '/fo"%2"' << '%1'
        end
        alias add_libraryPath add_includePath
        alias add_externPath add_includePath
        alias add_systemPath add_externPath
    end #~ WindowsCompilerRC

    const_memoize(self, :WindowsResourceCompiler) do
        rc = Build.WindowsSDK_RC_exe
        Log.log('Windows: using resource compiler found in \'%s\'', rc)
        WindowsCompilerRC.new('WindowsResourceCompiler', rc)
    end

end #~ Build
