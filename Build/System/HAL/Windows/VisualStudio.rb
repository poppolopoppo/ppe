# frozen_string_literal: true

require_once '../../Common.rb'

require_once '../../Shared/Compiler.rb'
require_once '../../Utils/Prerequisite.rb'

module Build

    persistent_switch(:PerfSDK, 'Use VisualStudio performance tools', init: true)

    module Visual
        MSC_VER_2019 = 1920
        MSC_VER_2017 = 1910
        MSC_VER_2015 = 1900
        MSC_VER_2013 = 1800
    end #~ Visual

    class VisualStudioCompiler < Compiler
        attr_reader :version, :minor_version
        attr_reader :host, :target
        attr_reader :rc
        attr_reader :visualStudioPath, :platformToolset
        def initialize(
            prefix,
            version, minor_version,
            host, target,
            visualStudioPath, platformToolset,
            compiler, librarian, linker, rc, *extra_files)
            super("#{prefix}_#{version}_#{host}", compiler, librarian, linker, *extra_files)
            @version = version
            @minor_version = minor_version
            @host = host
            @target = target
            @rc = rc
            @visualStudioPath = visualStudioPath
            @platformToolset = platformToolset
            @platformToolset.gsub!(/[^\d]+/, '')

            self.inherits!(Build.VisualStudio_Base)
            self.inherits!(Build.send "VisualStudio_Base_#{target}")

            self.export!('VisualStudioPath', @visualStudioPath)
            self.export!('VisualStudioVersion', @minor_version)

            Log.fatal 'invalid VisualStudio path "%s"', @visualStudioPath unless Dir.exist?(@visualStudioPath)
            Log.verbose 'Windows: new VisualStudio %s %s (toolset: %s)', @version, @minor_version, @platformToolset

            self.facet.defines << 'CPP_VISUALSTUDIO'
            self.facet.includePaths <<
                File.join(@visualStudioPath, 'VC', 'Tools', 'MSVC', @minor_version, 'include') <<
                File.join(@visualStudioPath, 'VC', 'Auxiliary', 'VS', 'include')
            self.facet.libraryPaths <<
                File.join(@visualStudioPath, 'VC', 'Tools', 'MSVC', @minor_version, 'lib', '$PlatformArch$') <<
                File.join(@visualStudioPath, 'VC', 'Auxiliary', 'VS', 'lib', '$PlatformArch$')
        end

        def self.infos_from(cl_exe)
            minor_version = cl_exe.match(/\/(14\..*?)\//)[1]

            _, host, target = *cl_exe.match(/\/Host(x86|x64)\/(x86|x64)\//i)
            host = "#{target}_#{host[1..-1]}" if host.downcase != target.downcase
            host = "Host#{host}"

            platformToolset = minor_version[0..3]
            platformToolset.gsub!(/[^\d]+/, '')

            _, visualStudioPath = *cl_exe.match(/^(.*)\/VC\/Tools\/MSVC\//)

            return [
                minor_version,
                host, target,
                visualStudioPath,
                platformToolset
            ]
        end

        def family() :msvc end

        def ext_binary() '.exe' end
        def ext_debug() '.pdb' end
        def ext_library() '.lib' end
        def ext_obj() '.obj' end
        def ext_pch() '.pch' end
        def ext_shared() '.dll' end

        def add_linkType(facet, link)
            case link
            when :static
            when :dynamic
                # https://msdn.microsoft.com/en-us/library/527z7zfs.aspx
                facet.linkerOptions << '/DLL'
            else
                Log.fatal 'unknown link type <%s>', link
            end
        end
        def add_define(facet, key, value=nil)
            token = value.nil? ? key : "#{key}=#{value}"
            add_compilerOption(facet, "/D#{token}")
        end
        def add_forceInclude(facet, filename)
            add_compilerOption(facet, "/FI\"#{filename}\"")
        end
        def add_includePath(facet, dirpath)
            add_compilerOption(facet, "/I\"#{dirpath}\"")
        end
        def add_library(facet, filename)
            facet.linkerOptions << "\"#{filename}\""
        end
        def add_libraryPath(facet, dirpath)
            facet.linkerOptions << "/LIBPATH:\"#{dirpath}\""
        end

        def customize(facet, env, target)
            super(facet, env, target)

            nopdb = facet.tag?(:nopdb)
            if nopdb || Build.Cache
                facet.compilerOptions << '/Z7' # debug symbols inside .obj
            else
                facet.compilerOptions << '/Zi' # debug symbols inside .pdb
                facet.compilerOptions << '/FS' # synchronous filesystem (necessary for concurrent cl.exe instances)
            end

            unless nopdb
                artefact = env.target_artefact_path(target)
                pdb_path = env.target_debug_path(artefact)
                facet.linkerOptions << "/PDB:\"#{pdb_path}\""

                if facet.compilerOptions & '/Zi'
                    artefact = env.output_path(target.abs_path, :library)
                    pdb_path = env.target_debug_path(artefact)
                    facet.compilerOptions << "/Fd\"#{pdb_path}\""
                end
            end

            if Build.PCH and target.pch?
                pch_object_path = env.output_path(target.pch_source, :pch)

                facet << Build.Compiler_PCHEnabled
                facet.pchOptions << facet.compilerOptions << '/Fp"%2"' << '/Fo"%3"'
                facet.pchOptions << "/Yc\"#{target.rel_pch_header}\""
                facet.compilerOptions << "/Yu\"#{target.rel_pch_header}\"" << "/Fp\"#{pch_object_path}\"" << '/Fo"%2"'
            else
                facet << Build.Compiler_PCHDisabled
                facet.compilerOptions << '/Fo"%2"'
            end
        end

        def decorate(facet, env)
            super(facet, env)

            if Build.PerfSDK and !env.config.tag?(:shipping)
                case env.platform.arch
                when :x86
                    facet << Build.VisualStudio_PerfSDK_X86
                when :x64
                    facet << Build.VisualStudio_PerfSDK_X64
                else
                    Log.fatal 'unsupported arch: <%s>', env.platform.arch
                end
            end

            if facet.tag?(:debug)
                facet.compilerOptions << '/MDd'
            else
                facet.compilerOptions << '/MD'
            end

            case env.config.link
            when :static
            when :dynamic
                if facet.tag?(:debug)
                    facet.compilerOptions << '/LDd'
                else
                    facet.compilerOptions << '/LD'
                end
            else
                Assert.not_implemented
            end
        end

    end #~ VisualStudioCompiler

    make_facet(:VisualStudio_ShowTimings) do
        Log.verbose 'Windows: using MSVC /d2cgsummary /d2:-cgsummary /Bt+ to analyze compilation times'
        compilerOptions << '/d2cgsummary' << '/Bt+'
        linkerOptions << '/d2:-cgsummary'
    end

    make_facet(:VisualStudio_Win10SlowDown_Workaround) do
        Log.verbose 'Windows: using MSVC /DELAYLOAD:Shell32.dll for Win10 workaround'
        linkerOptions <<
            'delayimp.lib' <<
            '/DELAYLOAD:Shell32.dll' <<
            '/IGNORE:4199' # warning LNK4199: /DELAYLOAD:XXX.dll ignored; no imports found from XXX.dll, caused by our added .libs #TODO : handles this more granularly ?
    end

    make_facet(:VisualStudio_Base) do
        Log.verbose 'Windows: using %s ISO standard', Build.CppStd
        Log.verbose 'Windows: default thread stack size is %d', Build.StackSize

        compilerOptions.append(
            '/nologo',              # no copyright when compiling
            "/std:#{Build.CppStd}", # C++2017
            '/WX',                  # warning as errors
            '/Gm-',                 # minimal rebuild is handled by FASTBuild
            '/GF',                  # string pooling
            '/GT',                  # fiber safe optimizations (https://msdn.microsoft.com/fr-fr/library/6e298fy4.aspx)
            '/EHsc',                # structure exception support (#TODO: optional ?)
            '/fp:fast',             # non-deterministic, allow vendor specific float intrinsics (https://msdn.microsoft.com/fr-fr/library/tzkfha43.aspx)
            '/bigobj',              # more sections inside obj files, support larger translation units, needed for unity builds
            '/d2FH4',               # https://devblogs.microsoft.com/cppblog/msvc-backend-updates-in-visual-studio-2019-preview-2/
            '/vmb',                 # class is always defined before pointer to member (https://docs.microsoft.com/en-us/cpp/build/reference/vmb-vmg-representation-method?view=vs-2019)
            '/openmp-' )            # disable OpenMP automatic parallelization
        options = [
        #'/Za',                     # disable non-ANSI features
        '/Zc:inline',               # https://msdn.microsoft.com/fr-fr/library/dn642448.aspx
        '/Zc:implicitNoexcept',     # https://msdn.microsoft.com/fr-fr/library/dn818588.aspx
        '/Zc:rvalueCast',           # https://msdn.microsoft.com/fr-fr/library/dn449507.aspx
        '/Zc:strictStrings',        # https://msdn.microsoft.com/fr-fr/library/dn449508.aspx
        '/Zc:wchar_t',              # promote wchar_t as a native type
        '/Zc:forScope',             # prevent from spilling iterators outside loops
        '/utf-8',                   # https://docs.microsoft.com/fr-fr/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
        '/W4',                      # warning level 4 (verbose)
        '/TP',                      # compile as C++
        '/F', Build.StackSize       # set default thread stack size
        ]
        analysisOptions.append(*options)
        compilerOptions.append(*options)

        warnings = [
        ### ERRORS ###
        '/we4062',                  # enumerator 'identifier' in a switch of enum 'enumeration' is not handled
        '/we4263',                  # 'function' : member function does not override any base class virtual member function
        '/we4265',                  # 'class': class has virtual functions, but destructor is not virtual // not handler by boost and stl
        '/we4296',                  # 'operator': expression is always false
        '/we4555',                  # expression has no effect; expected expression with side-effect
        '/we4619',                  # #pragma warning : there is no warning number 'number'
        '/we4640',                  # 'instance' : construction of local static object is not thread-safe
        '/we4826',                  # Conversion from 'type1 ' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
        '/we4836',                  # nonstandard extension used : 'type' : local types or unnamed types cannot be used as template arguments
        '/we4905',                  # wide string literal cast to 'LPSTR'
        '/we4906',                  # string literal cast to 'LPWSTR'
        ### IGNORED ###
        '/wd4201',                  # nonstandard extension used: nameless struct/union'
        '/wd4251',                  # 'XXX' needs to have dll-interface to be used by clients of class 'YYY'
        ]
        analysisOptions.append(*warnings, '/analyze', '/analyze:stacksize', Build.StackSize)
        compilerOptions.append(*warnings)

        compilerOptions.append('/c', '%1') # input file injection

        librarianOptions.append('/nologo', '/WX', '/SUBSYSTEM:WINDOWS', '/IGNORE:4221', '/OUT:"%2"', '%1')
        linkerOptions.append(
            '/nologo',              # no copyright when compiling
            '/WX',                  # warning as errors
            '/TLBID:1',             # https://msdn.microsoft.com/fr-fr/library/b1kw34cb.aspx
            '/DEBUG',               # generate debug infos
            '/IGNORE:4001',         # https://msdn.microsoft.com/en-us/library/aa234697(v=vs.60).aspx
            '/NXCOMPAT:NO',         # disable Data Execution Prevention (DEP)
            '/LARGEADDRESSAWARE',   # inddicate support for VM > 2Gb (if 3Gb flag is toggled)
            '/VERBOSE:INCR',        # incremental linker diagnosis
            #'/VERBOSE',
            #'/VERBOSE:REF',
            #'/VERBOSE:UNUSEDLIBS',
            '/SUBSYSTEM:WINDOWS',   # ~Windows~ application type (vs Console)
            "/STACK:#{Build.StackSize}",
            'kernel32.lib',
            'User32.lib',           # TODO : slim that list down
            'Shell32.lib',
            'Gdi32.lib',
            'Shlwapi.lib',
            'userenv.lib',
            'advapi32.lib',
            'version.lib',
            '/OUT:"%2"', '%1' )

        # https://devblogs.microsoft.com/cppblog/stl-features-and-fixes-in-vs-2017-15-8/
        defines.append('_ENABLE_EXTENDED_ALIGNED_STORAGE')

        if Build.Incremental
            Log.verbose 'Windows: using incremental linker with fastlink'
            linkerOptions.append('/INCREMENTAL') # enable incremental linker
        else
            Log.verbose 'Windows: using non-incremental linker'
            linkerOptions.append('/INCREMENTAL:NO') # disable incremental linker
        end

        self << Build.VisualStudio_Win10SlowDown_Workaround
        self << Build.VisualStudio_ShowTimings if Build.ShowTimings
    end

    make_facet(:VisualStudio_Base_x86) do
        compilerOptions << '/favor:blend' << '/arch:AVX2'
        librarianOptions << '/MACHINE:X86'
        linkerOptions << '/MACHINE:X86' << '/SAFESEH'
    end
    make_facet(:VisualStudio_Base_x64) do
        compilerOptions << '/favor:AMD64' << '/arch:AVX2'
        librarianOptions << '/MACHINE:X64'
        linkerOptions << '/MACHINE:X64'
    end

    make_facet(:VisualStudio_PerfSDK_X86) do
        Log.verbose 'Windows: using VisualStudio PerfSDK for x86'
        perfSDK = '$VisualStudioPath$/Team Tools/Performance Tools/PerfSDK'
        defines << 'WITH_VISUALSTUDIO_PERFSDK'
        includePaths << perfSDK
        libraryPaths << perfSDK
    end
    make_facet(:VisualStudio_PerfSDK_X64) do
        Log.verbose 'Windows: using VisualStudio PerfSDK for x64'
        perfSDK = '$VisualStudioPath$/Team Tools/Performance Tools/x64/PerfSDK'
        defines << 'WITH_VISUALSTUDIO_PERFSDK'
        includePaths << perfSDK
        libraryPaths << perfSDK
    end

    make_facet(:VisualStudio_LTO) do
        if Build.LTO
            if Build.Incremental
                Log.verbose 'Windows: using incremental link-time code generation'
                linkerOptions << '/LTCG:INCREMENTAL'
            else
                Log.verbose 'Windows: using link-time code generation'
                linkerOptions << '/LTCG'
            end
        else
            Log.verbose 'Windows: using compile-time code generation'
            linkerOptions << '/LTCG:OFF'
        end
    end

    make_facet(:VisualStudio_STL_DisableIteratorDebug) do
        defines.append(
            "_SECURE_SCL=0",                 # https://msdn.microsoft.com/fr-fr/library/aa985896.aspx
            "_ITERATOR_DEBUG_LEVEL=0",       # https://msdn.microsoft.com/fr-fr/library/hh697468.aspx
            "_HAS_ITERATOR_DEBUGGING=0")     # https://msdn.microsoft.com/fr-fr/library/aa985939.aspx
    end
    make_facet(:VisualStudio_STL_EnableIteratorDebug) do
        if Build.RuntimeChecks
            defines.append(
                "_SECURE_SCL=1",                 # https://msdn.microsoft.com/fr-fr/library/aa985896.aspx
                "_ITERATOR_DEBUG_LEVEL=2",       # https://msdn.microsoft.com/fr-fr/library/hh697468.aspx
                "_HAS_ITERATOR_DEBUGGING=1")     # https://msdn.microsoft.com/fr-fr/library/aa985939.aspx
        else
            self << Build.VisualStudio_STL_DisableIteratorDebug
        end
    end

    make_facet(:VisualStudio_Debug) do
        compilerOptions.append('/Od', '/Oy-', '/Gw-', '/GR')
        if Build.RuntimeChecks
            # https://msdn.microsoft.com/fr-fr/library/jj161081(v=vs.140).aspx
            # https://msdn.microsoft.com/fr-fr/library/8wtf2dfz.aspx
            compilerOptions.append('/GS', '/sdl', '/RTC1')
        end
        linkerOptions.append('/DYNAMICBASE:NO')
        self << Build.VisualStudio_STL_EnableIteratorDebug
    end
    make_facet(:VisualStudio_FastDebug) do
        compilerOptions.append('/Ob1', '/Oy-', '/Gw-', '/GR', '/Zo')
        if Build.RuntimeChecks
            # https://msdn.microsoft.com/fr-fr/library/jj161081(v=vs.140).aspx
            compilerOptions.append('/GS', '/sdl')
        end
        linkerOptions.append('/DYNAMICBASE:NO')
        self << Build.VisualStudio_STL_EnableIteratorDebug
    end
    make_facet(:VisualStudio_Release) do
        defines << '_NO_DEBUG_HEAP=1'
        compilerOptions.append('/O2', '/Oy-', '/GS-', '/GA', '/GR-', '/Zo')
        linkerOptions.append('/DYNAMICBASE:NO')
        self << Build.VisualStudio_LTO << Build.VisualStudio_STL_DisableIteratorDebug
    end
    make_facet(:VisualStudio_Profiling) do
        defines << '_NO_DEBUG_HEAP=1'
        compilerOptions.append('/O2', '/Ob3', '/GS-', '/Gw', '/Gy', '/GL', '/GA', '/GR-', '/Zo')
        linkerOptions.append('/DYNAMICBASE', '/PROFILE', '/OPT:REF')
        self << Build.VisualStudio_LTO << Build.VisualStudio_STL_DisableIteratorDebug
    end
    make_facet(:VisualStudio_Final) do
        defines << '_NO_DEBUG_HEAP=1'
        compilerOptions.append('/O2', '/Ob3', '/GS-', '/Gw', '/Gy', '/GL', '/GA', '/GR-', '/Zo')
        linkerOptions.append('/DYNAMICBASE', '/OPT:REF', '/OPT:ICF=3')
        self << Build.VisualStudio_LTO << Build.VisualStudio_STL_DisableIteratorDebug
    end

    make_facet(:VisualStudio_Base_2019) do
        # https://blogs.msdn.microsoft.com/vcblog/2019/12/13/broken-warnings-theory/
        defines.append('USE_PPE_MSVC_PRAGMA_SYSTEMHEADER')
        compilerOptions.append('/experimental:external', '/external:anglebrackets', '/external:W0')

        # https://docs.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance
        compilerOptions.append('/permissive-')

        # https://docs.microsoft.com/en-us/cpp/preprocessor/preprocessor-experimental-overview?view=vs-2019
        # TODO: disabled since there is no support for __VA_OPT__(x), and it's necessary to handle some funky macros
        # compilerOptions.append('/experimental:preprocessor')

        if Build.Incremental
            Log.verbose 'Windows: using /DEBUG:FASTLINK to speed-up the linker (non shippable!)'
            linkerOptions.append('/DEBUG:FASTLINK') # enable incremental linker
        end
    end

    VSWhere_exe = File.join($BuildPath, 'System', 'HAL', 'Windows', 'vswhere.exe')
    import_cmdline(:VsWhere_Hostx86_x86, VSWhere_exe, '-find', '**/Hostx86/x86/cl.exe').validate_FileExist!
    import_cmdline(:VsWhere_Hostx86_x64, VSWhere_exe, '-find', '**/Hostx86/x64/cl.exe').validate_FileExist!
    import_cmdline(:VsWhere_Hostx64_x86, VSWhere_exe, '-find', '**/Hostx64/x86/cl.exe').validate_FileExist!
    import_cmdline(:VsWhere_Hostx64_x64, VSWhere_exe, '-find', '**/Hostx64/x64/cl.exe').validate_FileExist!

    const_memoize(self, :VsWhere_Hostx86) do
        where = os_x64? ? Build.VsWhere_Hostx64_x86 : Build.VsWhere_Hostx86_x86
        where.collect!{|x| File.expand_path(x) }
        where.sort!{|a,b| b <=> a } # descending
        where
    end
    const_memoize(self, :VsWhere_Hostx64) do
        where = os_x64? ? Build.VsWhere_Hostx64_x64 : Build.VsWhere_Hostx86_x64
        where.collect!{|x| File.expand_path(x) }
        where.sort!{|a,b| b <=> a } # descending
        where
    end

    def self.make_fileset_vs2019(prereq, cl_exe)
        return false unless cl_exe
        root_host = File.dirname(cl_exe)
        clui_dll = prereq.need_glob!('**/clui.dll', basepath: root_host)
        return false unless clui_dll
        clui_dll = clui_dll.first
        root_base = root_host.end_with?('/Hostx64/x86') ?
            root_host[0..-3]<<'64' : root_host
        prereq.need_fileset!(
            cl_exe,
            clui_dll,
            File.join(root_host, 'c1.dll'),
            File.join(root_host, 'c1xx.dll'),
            File.join(root_host, 'c2.dll'),
            File.join(root_host, 'vcruntime140_1.dll'),
            File.join(root_base, 'msobj140.dll'),
            File.join(root_base, 'mspdb140.dll'),
            File.join(root_base, 'mspdbcore.dll'),
            File.join(root_base, 'mspft140.dll'),
            File.join(root_base, 'msvcp140_1.dll'),
            File.join(root_base, 'msvcp140_2.dll'),
            File.join(root_base, 'vcruntime140.dll') )
    end

    make_prerequisite(:VS2019_Hostx86_FileSet) do
        Build.make_fileset_vs2019(self, Build.VsWhere_Hostx86.detect{|x| x.include?('/2019/') })
    end
    make_prerequisite(:VS2019_Hostx64_FileSet) do
        Build.make_fileset_vs2019(self, Build.VsWhere_Hostx64.detect{|x| x.include?('/2019/') })
    end

    def self.make_visualstudio_compiler(version, fileset)
        return unless fileset

        cl_exe = fileset.first
        fileset = fileset[1..-1]

        Log.debug 'Windows: found VisualStudio compiler in "%s"', cl_exe

        binpath = File.dirname(cl_exe)
        lib_exe = File.join(binpath, 'lib.exe')
        link_exe = File.join(binpath, 'link.exe')
        rc_exe = Build.WindowsSDK_RC_exe

        return VisualStudioCompiler.new(
            'VisualStudio', version,
            *VisualStudioCompiler.infos_from(cl_exe),
            cl_exe, lib_exe, link_exe, rc_exe, *fileset).
            inherits!(Build.VisualStudio_Base_2019)
    end

    const_memoize(self, :VisualStudio2019_Hostx86) do
        Build.make_visualstudio_compiler('2019', Build.VS2019_Hostx86_FileSet)
    end
    const_memoize(self, :VisualStudio2019_Hostx64) do
        Build.make_visualstudio_compiler('2019', Build.VS2019_Hostx64_FileSet)
    end

    def VisualStudio_Hostx86() Build.VisualStudio2019_Hostx86 end
    def VisualStudio_Hostx64() Build.VisualStudio2019_Hostx64 end

end #~ Build