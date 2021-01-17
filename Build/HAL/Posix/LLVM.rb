# frozen_string_literal: true

require_once '../../Common.rb'

# TODO: debug symbols
# TODO: diagnostics
# TODO: incremental linker
# TODO: precompiled headers
# TODO: runtime checks (ASAN, TSAN)

module Build

    LLVM_DEBS = %w{
        libncurses-dev
        libvulkan1
        libxcb1-dev
    }

    make_command(:deb, 'list needed deb packages') do |&namespace|
        $stdout.puts LLVM_DEBS
    end

    class LLVMPosixCompiler < Compiler
        attr_reader :llvmPath, :llvmVersion
        def initialize(
            prefix, version, target,
            clang, ar, link, *extra_files)
            super("#{prefix}_#{version.tr('.', '_')}_#{target}", clang, ar, link, *extra_files)

            @llvmPath = Pathname.new(File.join(File.dirname(clang), '..'))
            @llvmPath = @llvmPath.cleanpath

            Log.fatal 'Posix: invalid LLVM  path "%s"', @llvmPath unless Dir.exist?(@llvmPath)
            self.facet.export!('LLVMPath', @llvmPath)

            @llvmVersion = File.basename(@llvmPath)

            Log.log 'Posix: found %s-%s in "%s" (%s)', @llvmVersion, target, @llvmPath, version
            self.facet.export!('LLVMVersion', @llvmVersion)
            self.facet.export!('LLVMVersionFull', version)

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
            add_compilationFlag(facet, "-L#{dirpath}")
            add_compilationFlag(facet, "-Wno-unused-command-line-argument")
            #facet.linkerOptions << "-I#{dirpath}" # *NOT* for llvm-link
            #facet.librarianOptions << "-I#{dirpath}" # *NOT* for llvm-ar
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
                    File.join(dirpath, 'clang++'),
                    File.join(dirpath, 'llvm-ar'),
                    File.join(dirpath, 'llvm-link') )
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
            File.join('$LLVMPath$', 'include', 'llvm') <<
            File.join('$LLVMPath$', 'include', 'llvm-c') <<
            File.join('$LLVMPath$', 'lib', 'clang', '$LLVMVersionFull$', 'include')

        libraryPaths <<
            File.join('$LLVMPath$', 'lib') <<
            File.join('$LLVMPath$', 'lib', 'clang', '$LLVMVersionFull$', 'lib', 'linux')
    end

    make_facet(:LLVM_Posix_Base_x86) do
        compilationFlag!('-m32')
    end
    make_facet(:LLVM_Posix_Base_x64) do
        compilationFlag!('-m64')
    end

    make_facet(:LLVM_Posix_LTO_Disabled) do
        compilationFlag!('-fno-lto')
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
        compilationFlag!('-fno-rtti')
    end
    make_facet(:LLVM_Posix_RTTI_Enabled) do
        defines.append('PPE_HAS_CXXRTTI=1')
        compilationFlag!('-frtti')
    end

    make_facet(:LLVM_Posix_Debug) do
        compilationFlag!('-O0', '-fno-pie')
        self <<
            Build.LLVM_Posix_LTO_Disabled <<
            Build.LLVM_Posix_RTTI_Enabled
    end
    make_facet(:LLVM_Posix_FastDebug) do
        compilationFlag!('-O1', '-fno-pie')
        self <<
            Build.LLVM_Posix_LTO_Disabled <<
            Build.LLVM_Posix_RTTI_Enabled
    end
    make_facet(:LLVM_Posix_Release) do
        compilationFlag!('-O2', '-fno-pie')
        self <<
            Build.LLVM_Posix_LTO_Enabled <<
            Build.LLVM_Posix_RTTI_Disabled
    end
    make_facet(:LLVM_Posix_Profiling) do
        compilationFlag!('-O3', '-fpie')
        self <<
            Build.LLVM_Posix_LTO_Enabled <<
            Build.LLVM_Posix_RTTI_Disabled
    end
    make_facet(:LLVM_Posix_Final) do
        compilationFlag!('-O3', '-fpie')
        self <<
            Build.LLVM_Posix_LTO_Enabled <<
            Build.LLVM_Posix_RTTI_Disabled
    end

    def self.make_llvmposix_compiler(target, llvm_fileset)
        Assert.expect(llvm_fileset, Array)

        clang, ar, link = *llvm_fileset

        Log.debug 'Posix: found LLVM posix compiler in "%s"', clang

        clang = llvm_fileset.first
        fileset = llvm_fileset[3..-1]
        version = %x{clang -dumpversion}.lines.first.chomp.strip

        return LLVMPosixCompiler.new(
            'LLVM_Posix', version, target,
            clang, ar, link, *fileset )
    end

    const_memoize(self, :LLVM_Posix_Hostx86) do
        Build.make_llvmposix_compiler('x86', Build.LLVM_Posix_Fileset)
    end
    const_memoize(self, :LLVM_Posix_Hostx64) do
        Build.make_llvmposix_compiler('x64', Build.LLVM_Posix_Fileset)
    end

end #~ Build
