# frozen_string_literal: true

require_once '../../Common.rb'

module Build

    class LLVMPosixCompiler < Compiler
        attr_reader :llvmPath, :llvmVersion
        def initialize(
            prefix, version,
            host, target,
            clang, lld, *extra_files)
            super(prefix, version, minor_version, host, target,
                clang, clang, lld, *extra_files )

            @llvmPath = Pathname.new(File.join(File.dirname(clang), '..'))
            @llvmPath = @llvmPath.cleanpath

            Log.fatal 'Posix: invalid LLVM  path "%s"', @llvmPath unless Dir.exist?(@llvmPath)
            self.facet.export!('LLVMPath', @llvmPath)

            versions = Dir.entries(File.join(@llvmPath, 'lib', 'clang'))
            versions.sort!
            @llvmersion = versions.last

            Log.verbose 'Posix: found LLVM v%s in "%s"', @llvmVersion, @llvmPath
            self.facet.export!('LLVMVersion', @llvmVersion)

            self.inherits!(Build.LLVM_Posix_Base)
            self.inherits!(Build.send "LLVM_Posix_Base_#{target}")
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
            add_compilerOption(facet, "-D#{token}")
        end
        def add_forceInclude(facet, filename)
            add_compilerOption(facet, '-include' "\"#{filename}\"")
        end
        def add_includePath(facet, dirpath)
            add_compilerOption(facet, "-I\"#{dirpath}\"")
        end
        alias add_externPath add_includePath
        alias add_systemPath add_includePath
        def add_library(facet, filename)
            facet.linkerOptions << "\"#{filename}\""
        end
        def add_libraryPath(facet, dirpath)
            facet.linkerOptions << "-I\"#{dirpath}\""
        end

        def customize(facet, env, target)
            super(facet, env, target)
            # TODO: PDB %NOCOMMIT%
            # TODO: PCH %NOCOMMIT%
            # TODO: ASAN %NOCOMMIT%
            # TODO: TSAN %NOCOMMIT%
        end

    end #~ LLVMPosixCompiler

    def self.import_llvm_posix(name, binary)
        make_prerequisite(name) do
            if fullpath = need_cmdline!('realpath', '$(', 'which', binary, ')')
                dirpath = File.dirname(fullpath)
                need_fileset!(
                    File.join(dirpath, 'clang'),
                    File.join(dirpath, 'lld') )
            end
        end
    end

    import_llvm_posix(:LLVM_Posix_Fileset, 'clang++')

    make_facet(:LLVM_Posix_Base) do
        defines << 'CPP_CLANG' << 'LLVM_FOR_POSIX'

        compilerOptions.append("-std=#{Build.CppStd}")
        compilerOptions.append('-mavx2','-msse4.2')
        compilerOptions.append('-mlzcnt','-mpopcnt')

        compilerOptions.append('-Wall', '-Wextra', '-Wshadow')
        compilerOptions.append('-Werror', '-Wfatal-errors')
        compilerOptions.append('-fcolor-diagnostics')

        compilerOptions.append('-c') # compile
        compilerOptions.append('-g') # generate debug infos
        compilerOptions.append('-o', '%2', '%1')

        compilerOptions.append('-pthread')

        includePaths <<
            File.join('$LLVMPath$', 'include', 'clang-c') <<
            File.join('$LLVMPath$', 'include', 'llvm-c') <<
            File.join('$LLVMPath$', 'lib', 'clang', '$LLVMVersion$', 'include')
        libraryPaths <<
            File.join('$LLVMPath$', 'lib') <<
            File.join('$LLVMPath$', 'lib', 'clang', '$LLVMVersion$', 'lib', 'windows')
    end

    make_facet(:LLVM_Posix_Base_x86) do
        compilerOptions.append('-m32')
        linkerOptions.append('-m32')
    end
    make_facet(:LLVM_Posix_Base_x64) do
        compilerOptions.append('-m64')
        linkerOptions.append('-m64')
    end

    make_facet(:LLVM_Posix_LTO_Disabled) do
        librarianOptions << '-fno-lto'
        linkerOptions << '-fno-lto'
    end

    make_facet(:LLVM_Posix_LTO_Enabled) do
        if Build.LTO
            if Build.Incremental
                Log.verbose 'Linux: using incremental link-time code generation'
                librarianOptions << '-flto=thin'
                linkerOptions << '-flto=thin'
            else
                Log.verbose 'Linux: using link-time code generation'
                librarianOptions << '-flto'
                linkerOptions << '-flto'
            end
        else
            Log.verbose 'Linux: using compile-time code generation'
            self << Build.LLVM_Posix_LTO_Disabled
        end
    end

    make_facet(:LLVM_Posix_Debug) do
        analysisOptions.append('-O0')
        compilerOptions.append('-O0')
        linkerOptions.append('-fno-pie', '-frtti')
        self << Build.LLVM_Posix_LTO_Disabled
    end
    make_facet(:LLVM_Posix_FastDebug) do
        analysisOptions.append('/O1')
        compilerOptions.append('/O1')
        linkerOptions.append('-fno-pie', '-frtti')
        self << Build.LLVM_Posix_LTO_Disabled
    end
    make_facet(:LLVM_Posix_Release) do
        analysisOptions.append('/O2')
        compilerOptions.append('/O2')
        linkerOptions.append('-fno-pie', '-fno-rtti')
        self << Build.LLVM_Posix_LTO_Enabled
    end
    make_facet(:LLVM_Posix_Profiling) do
        analysisOptions.append('/O3')
        compilerOptions.append('/O3')
        linkerOptions.append('-fpie', '-fno-rtti')
        self << Build.LLVM_Posix_LTO_Enabled
    end
    make_facet(:LLVM_Posix_Final) do
        analysisOptions.append('/O3')
        compilerOptions.append('/O3')
        linkerOptions.append('-fpie', '-fno-rtti')
        self << Build.LLVM_Posix_LTO_Enabled
    end

    def self.make_llvmposix_compiler(version, target, llvm_fileset)
        return unless llvm_fileset
        clang, lld = *llvm_fileset

        Log.debug 'Posix: found LLVM posix compiler in "%s"', clang

        clang = llvm_fileset.first
        fileset = llvm_fileset[1..-1]

        return LLVMPosixCompiler.new(
            'LLVM_Posix', version,
            host, target,
            clang, lld, *fileset )
    end

    const_memoize(self, :LLVM_Posix_Hostx86) do
        Build.make_llvmposix_compiler('Clang', 'x86',
            Build.LLVM_Posix_Fileset )
    end
    const_memoize(self, :LLVM_Posix_Hostx64) do
        Build.make_llvmposix_compiler('Clang', 'x64',
            Build.LLVM_Posix_Fileset )
    end

end #~ Build