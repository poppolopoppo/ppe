# frozen_string_literal: true

require_once '../../Common.rb'

require_once '../../Shared/Compiler.rb'
require_once '../../Utils/Prerequisite.rb'

module Build

    persistent_switch(:ASAN, 'Use VisualStudio Address Sanitizer (ASAN)', init: false)
    persistent_switch(:JMC, 'Use VisualStudio Just-My-Code', init: false)
    persistent_switch(:PerfSDK, 'Use VisualStudio performance tools', init: true)
    persistent_switch(:StaticCRT, 'Use VisualStudio static CRT (/MT vs /MD)', init: false)
    persistent_switch(:TraditionalPP, 'Use VisualStudio traditional preprocessor (omit /Zc:preprocessor)', init: false)

    module Visual
        MSC_VER_2022 = 1930
        MSC_VER_2019 = 1920
        MSC_VER_2017 = 1910
        MSC_VER_2015 = 1900
        MSC_VER_2013 = 1800
    end #~ Visual

    class VisualStudioCompiler < Compiler
        attr_reader :version, :minor_version
        attr_reader :host, :target
        attr_reader :platformToolset
        attr_reader :visualStudioPath, :visualStudioTools
        def initialize(
            prefix,
            version, minor_version,
            host, target,
            visualStudioPath, platformToolset,
            compiler, librarian, linker, *extra_files)
            super("#{prefix}_#{version}_#{target}", compiler, librarian, linker, *extra_files)
            @version = version
            @minor_version = minor_version
            @host = host
            @target = target
            @visualStudioPath = visualStudioPath
            @platformToolset = platformToolset
            @visualStudioTools = File.join(@visualStudioPath, 'VC', 'Tools', 'MSVC', @minor_version, 'bin', "Host#{@target}", @target)

            self.inherits!(Build.VisualStudio_Base)
            self.inherits!(Build.send "VisualStudio_Base_#{target}")

            self.export!('VisualStudioPath', @visualStudioPath)
            self.export!('VisualStudioVersion', @minor_version)
            self.export!('VisualStudioTools', @visualStudioTools)

            Log.fatal 'invalid VisualStudio path "%s"', @visualStudioPath unless DirCache.exist?(@visualStudioPath)
            Log.log 'Windows: new VisualStudio %s %s (toolset: %s)', @version, @minor_version, @platformToolset

            self.facet.defines << 'CPP_VISUALSTUDIO'
            self.facet.systemPaths <<
                File.join(@visualStudioPath, 'VC', 'Tools', 'MSVC', @minor_version, 'include') <<
                File.join(@visualStudioPath, 'VC', 'Tools', 'MSVC', @minor_version, 'crt', 'src') <<
                File.join(@visualStudioPath, 'VC', 'Auxiliary', 'VS', 'include')
            self.facet.libraryPaths <<
                File.join(@visualStudioPath, 'VC', 'Tools', 'MSVC', @minor_version, 'lib', '$PlatformArch$') <<
                File.join(@visualStudioPath, 'VC', 'Auxiliary', 'VS', 'lib', '$PlatformArch$')

            case @version
            when '2022', 'ClangCl_VS2019'
                # not reproducing with 2022 (or with updates/reinstall ?)
                Log.log('Windows: don\'t use workaround for C1090 PDB API call failed error code 3 (%s)', @version)
                @workaround_C1090_PDB_API_call_failed_error_code_3 = false
            else
                # https://developercommunity.visualstudio.com/content/problem/552999/fatal-error-c1090-pdb-api-call-failed-error-code-3.html
                # https://developercommunity.visualstudio.com/content/problem/48897/c1090-pdb-api-call-failed-error-code-23.html
                Log.warning('Windows: use workaround for C1090 PDB API call failed error code 3 (%s)', @version)
                @workaround_C1090_PDB_API_call_failed_error_code_3 = true
            end
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

        def dbg_env()
            env = {}
            env['PATH'] = "#{@visualStudioTools};%PATH%"
            env['ASAN_OPTIONS'] = "windows_hook_rtl_allocators=true" if Build.ASAN
            return env;
        end

        def add_linkType(facet, link)
            case link
            when :static
                return # nothing todo
            when :dynamic
                # https://msdn.microsoft.com/en-us/library/527z7zfs.aspx
                facet.linkerOptions << '/DLL'
            else
                Log.fatal 'unknown link type <%s>', link
            end
        end
        def add_define(facet, key, value=nil)
            token = value.nil? ? key : "#{key}=#{value}"
            add_compilationFlag(facet, "/D#{token}")
        end
        def add_forceInclude(facet, filename)
            add_compilationFlag(facet, "/FI\"#{filename}\"")
        end
        def add_includePath(facet, dirpath)
            add_compilationFlag(facet, '/I', "\"#{dirpath}\"")
        end
        def add_externPath(facet, dirpath)
            facet.analysisOptions << '/I' << "\"#{dirpath}\""
            facet.compilerOptions << '/external:I' << "\"#{dirpath}\""
            facet.pchOptions << '/external:I' << "\"#{dirpath}\""
            facet.preprocessorOptions << '/external:I' << "\"#{dirpath}\""
        end
#         alias add_externPath add_includePath
        alias add_systemPath add_externPath
        def add_library(facet, filename)
            facet.linkerOptions << "\"#{filename}\""
            facet.librarianOptions << "\"#{filename}\""
        end
        def add_libraryPath(facet, dirpath)
            facet.linkerOptions << "/LIBPATH:\"#{dirpath}\""
            facet.librarianOptions << "/LIBPATH:\"#{dirpath}\""
        end

        def customize(facet, env, target)
            nopdb = !Build.PDB || target.headers? || facet.tag?(:nopdb)
            nosymbols = !Build.Symbols || facet.tag?(:nosymbols)

            if nosymbols && !Build.ASAN
                Log.debug('VisualStudio: no debug symbols generated for target <%s-%s>', target.abs_path, env.family)
                facet.linkerOptions << '/DEBUG:NONE'
            else
                if Build.Incremental && !Build.ASAN
                    facet.linkerOptions << '/DEBUG:FASTLINK' # enable incremental linker
                else
                    facet.linkerOptions << '/DEBUG'
                end

                if nopdb || Build.Cache || (@workaround_C1090_PDB_API_call_failed_error_code_3 && !Build.ASAN)
                    # debug symbols inside .obj
                    facet.compilerOptions << '/Z7'
                    facet.pchOptions << '/Z7'
                else
                    # debug symbols inside .pdb / synchronous filesystem (necessary for concurrent cl.exe instances)
                    facet.compilerOptions << '/Zi' << '/FS'
                    facet.pchOptions << '/Zi' << '/FS'
                end
            end

            unless nopdb || nosymbols
                artifact = env.target_artifact_path(target)
                pdb_path = env.target_debug_path(artifact)

                facet.linkerOptions << "/PDB:\"#{pdb_path}\""

                if facet.compilerOptions & '/Zi'
                    artifact = env.output_path(target.abs_path, :library)
                    pdb_path = env.target_debug_path(artifact)

                    facet.compilerOptions << "/Fd\"#{pdb_path}\""
                    facet.pchOptions << "/Fd\"#{pdb_path}\""
                end
            end

            if Build.PCH and target.pch?
                pch_object_path = env.output_path(target.pch_source, :pch)

                facet << Build.Compiler_PCHEnabled
                facet.pchOptions << "/Yc\"#{target.rel_pch_header}\""
                facet.compilerOptions << "/Yu\"#{target.rel_pch_header}\"" << "/Fp\"#{pch_object_path}\""
            else
                facet << Build.Compiler_PCHDisabled
            end

            if Build.ASAN && env.compiler.version < '14.28.29617'
                # https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
                asan_type = 'asan' # ( facet.tag?(:debug) ? 'asan_dbg' : 'asan')
                case env.platform.arch
                when :x86
                    asan_host = 'i386'
                when :x64
                    asan_host = 'x86_64'
                else
                    Assert.not_implemented
                end

                if env.target_need_link?(target)
                    if Build.StaticCRT && facet.tag?(:debug) == false # ASAN is not supported with static debug (/MTd)
                        if target.executable?
                            facet.libraries << "/wholearchive:clang_rt.#{asan_type}_cxx-#{asan_host}.lib"
                        else
                            facet.libraries << "/wholearchive:clang_rt.#{asan_type}_dll_thunk-#{asan_host}.lib"
                        end
                    else
                        facet.libraries <<
                            "/wholearchive:clang_rt.#{asan_type}_dynamic-#{asan_host}.lib" <<
                            "/wholearchive:clang_rt.#{asan_type}_dynamic_runtime_thunk-#{asan_host}.lib"
                        dll = File.join(@visualStudioTools, "clang_rt.#{asan_type}_dynamic-#{asan_host}.dll")
                        env.target_deploy(target, dll)
                    end
                end
            end

            super(facet, env, target)
        end

        def decorate(facet, env)
            super(facet, env)

            if Build.PerfSDK and !facet.tag?(:shipping)
                case env.platform.arch
                when :x86
                    facet << Build.VisualStudio_PerfSDK_X86
                when :x64
                    facet << Build.VisualStudio_PerfSDK_X64
                else
                    Log.fatal 'unsupported arch: <%s>', env.platform.arch
                end
            end

            if Build.StaticCRT
                facet.compilationFlag!(facet.tag?(:debug) ? '/MTd' : '/MT')
            else
                facet.compilationFlag!(facet.tag?(:debug) ? '/MDd' : '/MD')
            end

            case env.config.link
            when :static
                # nothing todo
            when :dynamic
                facet.compilationFlag!(facet.tag?(:debug) ? '/LDd' : '/LD')
            else
                Assert.not_implemented
            end
        end

        def freeze()
            @version.freeze
            @minor_version.freeze
            @host.freeze
            @target.freeze
            @visualStudioPath.freeze
            @platformToolset.freeze
            @visualStudioTools.freeze
            super()
        end

    end #~ VisualStudioCompiler

    make_facet(:VisualStudio_ShowTimings) do
        Log.log 'Windows: using MSVC /d2cgsummary /d2:-cgsummary /Bt+ to analyze compilation times'
        compilerOptions << '/d2cgsummary' << '/Bt+'
        linkerOptions << '/d2:-cgsummary'
    end

    make_facet(:VisualStudio_Win10SlowDown_Workaround) do
        Log.log 'Windows: using MSVC /DELAYLOAD:Shell32.dll for Win10 workaround'
        linkerOptions <<
            'delayimp.lib' <<
            '/DELAYLOAD:Shell32.dll' <<
            '/IGNORE:4199' # warning LNK4199: /DELAYLOAD:XXX.dll ignored; no imports found from XXX.dll, caused by our added .libs #TODO : handles this more granularly ?
    end

    make_facet(:VisualStudio_Base) do
        Log.log 'Windows: using %s ISO standard', Build.CppStd
        Log.log 'Windows: default thread stack size is %d', Build.StackSize

        compilationArgs = [
            '/nologo',                  # no copyright when compiling
            '/c', '%1' ]                # input file injection
        compilerOptions.append(*compilationArgs)
        pchOptions.append(*compilationArgs)
        preprocessorOptions.append(*compilationArgs)

        compilerOptions.append('/Fo"%2"')
        pchOptions.append('/Fp"%2"', '/Fo"%3"')
        preprocessorOptions.append('/Fo"%2"')

        stackSize = Build.StackSize
        stackSize *= 2 if Build.ASAN

        compilationFlag!(
            '/Gm-',                     # minimal rebuild is handled by FASTBuild
            '/GF',                      # string pooling
            '/GT',                      # fiber safe optimizations (https://msdn.microsoft.com/fr-fr/library/6e298fy4.aspx)
            "/std:#{Build.CppStd}",     # specify Cpp standard (c++14,c++17,c++latest)
            '/bigobj',                  # more sections inside obj files, support larger translation units, needed for unity builds
            '/d2FH4',                   # https://devblogs.microsoft.com/cppblog/msvc-backend-updates-in-visual-studio-2019-preview-2/
            '/EHsc',                    # structure exception support (#TODO: optional ?)
            '/fp:fast',                 # non-deterministic, allow vendor specific float intrinsics (https://msdn.microsoft.com/fr-fr/library/tzkfha43.aspx)
            '/vmb',                     # class is always defined before pointer to member (https://docs.microsoft.com/en-us/cpp/build/reference/vmb-vmg-representation-method?view=vs-2019)
            '/openmp-',                 # disable OpenMP automatic parallelization
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
            "/F\"#{stackSize}\"" )      # set default thread stack size

        if Build.Strict
            Log.log 'Windows: using strict warnings and warning as error'

            # toggle warning as error for whole build chain
            compilationFlag!('/WX')
            librarianOptions.append('/WX')
            linkerOptions.append('/WX')
            # promote some warnings as errors
            compilationFlag!(
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
            )
        end

        compilationFlag!(
        ### IGNORED ###
            '/wd4201',                  # nonstandard extension used: nameless struct/union'
            '/wd4251' )                 # 'XXX' needs to have dll-interface to be used by clients of class 'YYY'

        librarianOptions.append('/nologo', '/SUBSYSTEM:WINDOWS', '/IGNORE:4221', '/OUT:"%2"', '%1')
        linkerOptions.append(
            '/nologo',                  # no copyright when compiling
            '/TLBID:1',                 # https://msdn.microsoft.com/fr-fr/library/b1kw34cb.aspx
            '/IGNORE:4001',             # https://msdn.microsoft.com/en-us/library/aa234697(v=vs.60).aspx
            '/NXCOMPAT:NO',             # disable Data Execution Prevention (DEP)
            '/LARGEADDRESSAWARE',       # indicate support for VM > 2Gb (if 3Gb flag is toggled)
            '/VERBOSE:INCR',            # incremental linker diagnosis
            '/SUBSYSTEM:WINDOWS',       # ~Windows~ application type (vs Console)
            "/STACK:#{stackSize}",
            '/fastfail',                # better error reporting
            'kernel32.lib',
            'User32.lib',               # TODO : slim that list down
            'Shell32.lib',
            'Gdi32.lib',
            'Shlwapi.lib',
            'userenv.lib',
            'advapi32.lib',
            'version.lib',
            '/OUT:"%2"', '%1' )


        if Build.Diagnose
            Log.log 'Windows: using static analysis options'

            analysisOptions.append('/analyze', '/analyze:stacksize', stackSize)

            Log.log 'Windows: using verbose output for the linker'

            linkerOptions.append(
                '/VERBOSE',
                #'/VERBOSE:LIB',
                #'/VERBOSE:ICF',
                #'/VERBOSE:REF',
                '/VERBOSE:UNUSEDLIBS' )
        end

        # https://devblogs.microsoft.com/cppblog/stl-features-and-fixes-in-vs-2017-15-8/
        defines.append('_ENABLE_EXTENDED_ALIGNED_STORAGE')

        if Build.Incremental && !Build.ASAN
            Log.log 'Windows: using incremental linker with fastlink'
            linkerOptions.append('/INCREMENTAL') # enable incremental linker
        else
            Log.log 'Windows: using non-incremental linker'
            linkerOptions.append('/INCREMENTAL:NO') # disable incremental linker
        end

        if Build.Strict
            # https://docs.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance
            compilationFlag!('/permissive-')
        else
            compilationFlag!('/permissive')
        end

        if Build.ASAN
            # https://devblogs.microsoft.com/cppblog/addresssanitizer-asan-for-windows-with-msvc/
            defines.append('USE_PPE_SANITIZER=1')
            compilationFlag!('/fsanitize=address')
        end

        # https://docs.microsoft.com/en-us/cpp/preprocessor/preprocessor-experimental-overview?view=vs-2019
        # UPDATE: https://devblogs.microsoft.com/cppblog/announcing-full-support-for-a-c-c-conformant-preprocessor-in-msvc/
        unless Build.TraditionalPP
            compilationFlag!('/Zc:preprocessor')
        end

        self << Build.VisualStudio_Win10SlowDown_Workaround
        self << Build.VisualStudio_ShowTimings if Build.Timings
    end

    make_facet(:VisualStudio_Base_x86) do
        compilationFlag!('/favor:blend', '/arch:AVX2')
        librarianOptions << '/MACHINE:X86'
        linkerOptions << '/MACHINE:X86' << '/SAFESEH'
    end
    make_facet(:VisualStudio_Base_x64) do
        compilationFlag!('/favor:AMD64', '/arch:AVX2')
        librarianOptions << '/MACHINE:X64'
        linkerOptions << '/MACHINE:X64'
    end

    make_facet(:VisualStudio_PerfSDK_X86) do
        Log.log 'Windows: using VisualStudio PerfSDK for x86'
        perfSDK = '$VisualStudioPath$/Team Tools/Performance Tools/PerfSDK'
        defines << 'WITH_VISUALSTUDIO_PERFSDK'
        externPaths << perfSDK
        libraryPaths << perfSDK
    end
    make_facet(:VisualStudio_PerfSDK_X64) do
        Log.log 'Windows: using VisualStudio PerfSDK for x64'
        perfSDK = '$VisualStudioPath$/Team Tools/Performance Tools/x64/PerfSDK'
        defines << 'WITH_VISUALSTUDIO_PERFSDK'
        externPaths << perfSDK
        libraryPaths << perfSDK
    end

    make_facet(:VisualStudio_LTO_Disabled) do
        linkerOptions << '/LTCG:OFF'
    end
    make_facet(:VisualStudio_LTO_Enabled) do
        if Build.LTO
            if Build.Incremental && !Build.ASAN
                Log.log 'Windows: using incremental link-time code generation'
                linkerOptions << '/LTCG:INCREMENTAL'
            else
                Log.log 'Windows: using link-time code generation'
                linkerOptions << '/LTCG'
            end
        else
            Log.log 'Windows: using compile-time code generation'
            self << Build.VisualStudio_LTO_Disabled
        end
    end

    make_facet(:VisualStudio_STL_DisableIteratorDebug) do
        defines.append(
            "_SECURE_SCL=0",                # https://msdn.microsoft.com/fr-fr/library/aa985896.aspx
            "_ITERATOR_DEBUG_LEVEL=0",      # https://msdn.microsoft.com/fr-fr/library/hh697468.aspx
            "_HAS_ITERATOR_DEBUGGING=0")    # https://msdn.microsoft.com/fr-fr/library/aa985939.aspx
    end
    make_facet(:VisualStudio_STL_EnableIteratorDebug) do
        defines.append(
            "_SECURE_SCL=1",                # https://msdn.microsoft.com/fr-fr/library/aa985896.aspx
            "_ITERATOR_DEBUG_LEVEL=2",      # https://msdn.microsoft.com/fr-fr/library/hh697468.aspx
            "_HAS_ITERATOR_DEBUGGING=1")    # https://msdn.microsoft.com/fr-fr/library/aa985939.aspx
    end

    make_facet(:VisualStudio_CXX_DisableRTTI) do
        defines.append('PPE_HAS_CXXRTTI=0')
        compilationFlag!('/GR-')
    end
    make_facet(:VisualStudio_CXX_EnableRTTI) do
        defines.append('PPE_HAS_CXXRTTI=1')
        compilationFlag!('/GR')
    end

    make_facet(:VisualStudio_Debug) do
        compilationFlag!('/Od', '/Oy-', '/Gw-')
        if Build.RuntimeChecks
            # https://msdn.microsoft.com/fr-fr/library/jj161081(v=vs.140).aspx
            # https://msdn.microsoft.com/fr-fr/library/8wtf2dfz.aspx
            compilationFlag!('/GS', '/sdl', '/RTC1')
        end
        linkerOptions.append('/DYNAMICBASE:NO')
        self <<
            Build.VisualStudio_LTO_Disabled <<
            Build.VisualStudio_STL_EnableIteratorDebug <<
            Build.VisualStudio_CXX_EnableRTTI
    end
    make_facet(:VisualStudio_FastDebug) do
        compilationFlag!('/Ob1', '/Oy-', '/Gw-', '/Zo')
        if Build.RuntimeChecks
            # https://msdn.microsoft.com/fr-fr/library/jj161081(v=vs.140).aspx
            compilationFlag!('/GS', '/sdl')
        end
        linkerOptions.append('/DYNAMICBASE:NO')
        self <<
            Build.VisualStudio_LTO_Disabled <<
            Build.VisualStudio_STL_EnableIteratorDebug <<
            Build.VisualStudio_CXX_EnableRTTI
    end
    make_facet(:VisualStudio_Release) do
        defines << '_NO_DEBUG_HEAP=1'
        compilationFlag!('/O2', '/Oy-', '/GS-', '/GA', '/Zo')
        linkerOptions.append('/DYNAMICBASE:NO')
        self <<
            Build.VisualStudio_LTO_Disabled <<
            Build.VisualStudio_STL_DisableIteratorDebug <<
            Build.VisualStudio_CXX_DisableRTTI
    end
    make_facet(:VisualStudio_Profiling) do
        defines << '_NO_DEBUG_HEAP=1'
        compilationFlag!('/O2', '/Ob3', '/GS-', '/Gw', '/Gy', '/GL', '/GA', '/Zo')
        linkerOptions.append('/DYNAMICBASE', '/PROFILE', '/OPT:REF')
        self <<
            Build.VisualStudio_LTO_Enabled <<
            Build.VisualStudio_STL_DisableIteratorDebug <<
            Build.VisualStudio_CXX_DisableRTTI
    end
    make_facet(:VisualStudio_Final) do
        defines << '_NO_DEBUG_HEAP=1'
        compilationFlag!('/O2', '/Ob3', '/GS-', '/Gw', '/Gy', '/GL', '/GA', '/Zo')
        linkerOptions.append('/DYNAMICBASE', '/OPT:REF', '/OPT:ICF=3')
        self <<
            Build.VisualStudio_LTO_Enabled <<
            Build.VisualStudio_STL_DisableIteratorDebug <<
            Build.VisualStudio_CXX_DisableRTTI
    end

    make_facet(:VisualStudio_Base_2019) do
        # https://blogs.msdn.microsoft.com/vcblog/2019/12/13/broken-warnings-theory/
        defines.append('USE_PPE_MSVC_PRAGMA_SYSTEMHEADER')
        compilationFlag_noAnalysis!('/experimental:external', '/external:anglebrackets', '/external:W0')

        # Just-My-Code
        if Build.JMC
            # https://docs.microsoft.com/fr-fr/cpp/build/reference/jmc?view=msvc-160
            compilationFlag_noAnalysis!('/JMC')
        else
            compilationFlag_noAnalysis!('/JMC-')
        end
    end

    make_facet(:VisualStudio_Base_2022) do
        # External headers are now an official feature
        # https://docs.microsoft.com/en-us/cpp/build/reference/external-external-headers-diagnostics?view=msvc-160
        defines.append('USE_PPE_MSVC_PRAGMA_SYSTEMHEADER')
        compilationFlag_noAnalysis!('/external:templates-', '/external:W0')
        #compilationFlag_noAnalysis!('/external:anglebrackets') # TODO: broken with last preview, restore when fixed

        # Just-My-Code
        if Build.JMC
            # https://docs.microsoft.com/en-us/cpp/build/reference/jmc?view=msvc-160
            compilationFlag_noAnalysis!('/JMC')
        else
            compilationFlag_noAnalysis!('/JMC-')
        end
    end


    VSWhere_exe = File.join($BuildPath, 'HAL', 'Windows', 'vswhere.exe')
    def self.import_vswhere(name, pattern, *args)
        make_prerequisite(name) do
            need_cmdline!(VSWhere_exe, '-latest', '-find', pattern, *args)
        end.validate_FileExist!
    end

    def self.make_toolchain_fileset(prereq, cl_exe, version)
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
            File.join(root_base, 'vcruntime140.dll'),
            File.join(root_base, 'vcperf.exe'),)
    end

    def self.import_visualstudio_fileset(version, *args)
        import_vswhere(:"VsWhere_#{version}_Hostx86_x86", '**/Hostx86/x86/cl.exe', *args)
        import_vswhere(:"VsWhere_#{version}_Hostx86_x64", '**/Hostx86/x64/cl.exe', *args)
        import_vswhere(:"VsWhere_#{version}_Hostx64_x86", '**/Hostx64/x86/cl.exe', *args)
        import_vswhere(:"VsWhere_#{version}_Hostx64_x64", '**/Hostx64/x64/cl.exe', *args)

        make_prerequisite(:"VsWhere_#{version}_Hostx86") do
            fileset = Build.os_x64? ?
                Build.send("VsWhere_#{version}_Hostx64_x86") :
                Build.send("VsWhere_#{version}_Hostx86_x86")

            if fileset
                fileset = fileset.collect{|x| File.expand_path(x) }
                fileset.sort!{|a,b| b <=> a } # descending
                Build.make_toolchain_fileset(self, fileset.first, version)
            else
                nil
            end
        end.validate_FileExist!

        make_prerequisite(:"VsWhere_#{version}_Hostx64") do
            fileset = Build.os_x64? ?
                Build.send("VsWhere_#{version}_Hostx64_x64") :
                Build.send("VsWhere_#{version}_Hostx86_x64")

            if fileset
                fileset = fileset.collect{|x| File.expand_path(x) }
                fileset.sort!{|a,b| b <=> a } # descending
                Build.make_toolchain_fileset(self, fileset.first, version)
            else
                nil
            end
        end.validate_FileExist!
    end

    import_visualstudio_fileset('2019', '-version', '[16.0,17.0)')
    import_visualstudio_fileset('2022', '-version', '[17.0,18.0)', '-prerelease') # TODO: remove -prelease when shipped
    import_visualstudio_fileset('Insider', '-prerelease')

    def self.make_visualstudio_compiler(version, fileset)
        return unless fileset

        cl_exe = fileset.first
        fileset = fileset[1..-1]

        Log.log 'Windows: found VS compiler in "%s"', cl_exe

        binpath = File.dirname(cl_exe)
        lib_exe = File.join(binpath, 'lib.exe')
        link_exe = File.join(binpath, 'link.exe')

        return VisualStudioCompiler.new(
            'VisualStudio', version,
            *VisualStudioCompiler.infos_from(cl_exe),
            cl_exe, lib_exe, link_exe, *fileset)
    end

    const_memoize(self, :VisualStudio_2019_Hostx86) do
        Build.make_visualstudio_compiler('2019', Build.VsWhere_2019_Hostx86).inherits!(Build.VisualStudio_Base_2019)
    end
    const_memoize(self, :VisualStudio_2019_Hostx64) do
        Build.make_visualstudio_compiler('2019', Build.VsWhere_2019_Hostx64).inherits!(Build.VisualStudio_Base_2019)
    end

    const_memoize(self, :VisualStudio_2022_Hostx86) do
        Build.make_visualstudio_compiler('2022', Build.VsWhere_2022_Hostx86).inherits!(Build.VisualStudio_Base_2022)
    end
    const_memoize(self, :VisualStudio_2022_Hostx64) do
        Build.make_visualstudio_compiler('2022', Build.VsWhere_2022_Hostx64).inherits!(Build.VisualStudio_Base_2022)
    end

    const_memoize(self, :VisualStudio_Insider_Hostx86) do
        Build.make_visualstudio_compiler('Insider', Build.VsWhere_Insider_Hostx86).inherits!(Build.VisualStudio_Base_2019)
    end
    const_memoize(self, :VisualStudio_Insider_Hostx64) do
        Build.make_visualstudio_compiler('Insider', Build.VsWhere_Insider_Hostx64).inherits!(Build.VisualStudio_Base_2019)
    end

end #~ Build
