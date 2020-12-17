# frozen_string_literal: true

require_once '../../Common.rb'

# TODO: debug symbols
# TODO: diagnostics
# TODO: incremental linker
# TODO: precompiled headers
# TODO: runtime checks (ASAN, TSAN)

module Build

    class LLVMPosixCompiler < Compiler
        attr_reader :llvmPath, :llvmVersion
        def initialize(
            prefix, version, target,
            clang, ar, lld, *extra_files)
            super("#{prefix}_#{version.tr('.', '_')}_#{target}", clang, ar, lld, *extra_files)

            @llvmPath = Pathname.new(File.join(File.dirname(clang), '..'))
            @llvmPath = @llvmPath.cleanpath

            Log.fatal 'Posix: invalid LLVM  path "%s"', @llvmPath unless Dir.exist?(@llvmPath)
            self.facet.export!('LLVMPath', @llvmPath)

            @llvmVersion = File.basename(@llvmPath)

            Log.log 'Posix: found %s-%s in "%s" (%s)', @llvmVersion, target, @llvmPath, version
            self.facet.export!('LLVMVersion', @llvmVersion)

            self.inherits!(Build.LLVM_Posix_Base)
            self.inherits!(Build.send("LLVM_Posix_Base_#{target}"))
        end

        def ext_binary() '.out' end
        def ext_debug() '.dSYM' end
        def ext_library() '.a' end
        def ext_obj() '.o' end
        def ext_pch() '.pch' end
        def ext_shared() '.so' end

        def add_linkType(facet, link)
            # nothing to do
        end
        def add_define(facet, key, value=nil)
            token = value.nil? ? key : "#{key}=#{value}"
            add_compilationFlag(facet, "-D#{token}")
        end
        def add_forceInclude(facet, filename)
            add_compilationFlag(facet, "-include#{filename}")
        end
        def add_includePath(facet, dirpath)
            add_compilationFlag(facet, "-I#{dirpath}")
        end
        def add_externPath(facet, dirpath)
            add_compilationFlag(facet, "-iframework#{dirpath}")
        end
        def add_systemPath(facet, dirpath)
            add_compilationFlag(facet, "-isystem#{dirpath}")
        end
        def add_library(facet, filename)
            facet.linkerOptions << filename
            facet.librarianOptions << filename
        end
        def add_libraryPath(facet, dirpath)
            facet.linkerOptions << "-I#{dirpath}"
            facet.librarianOptions << "-I#{dirpath}"
        end

        def customize(facet, env, target)
            super(facet, env, target)
            # TODO: PCH,PDB
        end

    end #~ LLVMPosixCompiler

    def self.import_llvm_posix(name, binary)
        make_prerequisite(name) do
            if fullpath = need_cmdline!('realpath', '$(', 'which', binary, ')')
                dirpath = File.dirname(fullpath.first)
                Log.log 'LLVM: found binary path "%s"', dirpath
                need_fileset!(
                    File.join(dirpath, 'clang'),
                    File.join(dirpath, 'llvm-ar'),
                    File.join(dirpath, 'lld-link') )
            end
        end
    end

    import_llvm_posix(:LLVM_Posix_Fileset, 'clang++')

    make_facet(:LLVM_Posix_Base) do
        defines << 'CPP_CLANG' << 'LLVM_FOR_POSIX'

        compilationFlag!(
            "-std=#{Build.CppStd}",
            '-Wall', '-Wextra', '-Wshadow',
            '-Werror', '-Wfatal-errors',
            '-mavx2','-msse4.2',
            '-mlzcnt','-mpopcnt',
            '-fcolor-diagnostics',
            '-c', # compile
            '-g', # generate debug infos
            '-o', '%2', '%1' )

        librarianOptions << 'rc' << '%2' << '%1'
        linkerOptions << '-o' << '%2' << '%1'

        systemPaths <<
            File.join('$LLVMPath$', 'include', 'clang-c') <<
            File.join('$LLVMPath$', 'include', 'llvm-c') <<
            File.join('$LLVMPath$', 'lib', 'clang', '$LLVMVersion$', 'include')

        libraryPaths <<
            File.join('$LLVMPath$', 'lib') <<
            File.join('$LLVMPath$', 'lib', 'clang', '$LLVMVersion$', 'lib', 'windows')
    end

    make_facet(:LLVM_Posix_Base_x86) do
        compilationFlag!('-m32')
        linkerOptions.append('-m32')
    end
    make_facet(:LLVM_Posix_Base_x64) do
        compilationFlag!('-m64')
        linkerOptions.append('-m64')
    end

    make_facet(:LLVM_Posix_LTO_Disabled) do
        linkerOptions << '-fno-lto'
    end
    make_facet(:LLVM_Posix_LTO_Enabled) do
        if Build.LTO
            if Build.Incremental
                Log.log 'Linux: using incremental link-time code generation'
                librarianOptions << '-T'
                linkerOptions << '-flto=thin'
            else
                Log.log 'Linux: using link-time code generation'
                librarianOptions << '-T'
                linkerOptions << '-flto'
            end
        else
            Log.log 'Linux: using compile-time code generation'
            self << Build.LLVM_Posix_LTO_Disabled
        end
    end

    make_facet(:LLVM_Posix_RTTI_Disabled) do
        defines.append('PPE_HAS_CXXRTTI=0')
        linkerOptions.append('-fno-rtti')
    end
    make_facet(:LLVM_Posix_RTTI_Enabled) do
        defines.append('PPE_HAS_CXXRTTI=1')
        linkerOptions.append('-frtti')
    end

    make_facet(:LLVM_Posix_Debug) do
        compilationFlag!('-O0')
        linkerOptions.append('-fno-pie')
        self <<
            Build.LLVM_Posix_LTO_Disabled <<
            Build.LLVM_Posix_RTTI_Enabled
    end
    make_facet(:LLVM_Posix_FastDebug) do
        compilationFlag!('-O1')
        linkerOptions.append('-fno-pie')
        self <<
            Build.LLVM_Posix_LTO_Disabled <<
            Build.LLVM_Posix_RTTI_Enabled
    end
    make_facet(:LLVM_Posix_Release) do
        compilationFlag!('-O2')
        linkerOptions.append('-fno-pie')
        self <<
            Build.LLVM_Posix_LTO_Enabled <<
            Build.LLVM_Posix_RTTI_Disabled
    end
    make_facet(:LLVM_Posix_Profiling) do
        compilationFlag!('-O3')
        linkerOptions.append('-fpie')
        self <<
            Build.LLVM_Posix_LTO_Enabled <<
            Build.LLVM_Posix_RTTI_Disabled
    end
    make_facet(:LLVM_Posix_Final) do
        compilationFlag!('-O3')
        linkerOptions.append('-fpie')
        self <<
            Build.LLVM_Posix_LTO_Enabled <<
            Build.LLVM_Posix_RTTI_Disabled
    end

    def self.make_llvmposix_compiler(target, llvm_fileset)
        Assert.expect(llvm_fileset, Array)

        clang, ar, lld = *llvm_fileset

        Log.debug 'Posix: found LLVM posix compiler in "%s"', clang

        clang = llvm_fileset.first
        fileset = llvm_fileset[3..-1]
        version = %x{clang -dumpversion}.lines.first.chomp.strip

        return LLVMPosixCompiler.new(
            'LLVM_Posix', version, target,
            clang, ar, lld, *fileset )
    end

    const_memoize(self, :LLVM_Posix_Hostx86) do
        Build.make_llvmposix_compiler('x86', Build.LLVM_Posix_Fileset)
    end
    const_memoize(self, :LLVM_Posix_Hostx64) do
        Build.make_llvmposix_compiler('x64', Build.LLVM_Posix_Fileset)
    end

end #~ Build
