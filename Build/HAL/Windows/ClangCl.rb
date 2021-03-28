# frozen_string_literal: true

require_once '../../Common.rb'

require_once './VisualStudio.rb'

module Build

    class LLVMWindowsCompiler < VisualStudioCompiler
        attr_reader :llvmPath, :llvmVersion
        def initialize(
            prefix,
            version, minor_version,
            host, target,
            visualStudioPath, platformToolset,
            clang_cl, llvm_lib, lld_link, *extra_files)
            super(prefix, version, minor_version, host, target,
                visualStudioPath, platformToolset,
                clang_cl, llvm_lib, lld_link, *extra_files )

            @llvmPath = Pathname.new(File.join(File.dirname(clang_cl), '..'))
            @llvmPath = @llvmPath.cleanpath

            Log.fatal 'Windows: invalid LLVM  path "%s"', @llvmPath unless Dir.exist?(@llvmPath)
            self.facet.export!('LLVMPath', @llvmPath)

            versions = Dir.entries(File.join(@llvmPath, 'lib', 'clang'))
            versions.sort!
            @llvmersion = versions.last

            Log.log 'Windows: found LLVM v%s in "%s"', @llvmVersion, @llvmPath
            self.facet.export!('LLVMVersion', @llvmVersion)

            self.inherits!(Build.LLVM_Windows_Base)
            self.inherits!(Build.send "LLVM_Windows_Base_#{target}")
        end
        def family() :'clang-cl' end
        def customize(facet, env, target)
            super(facet, env, target)
            facet.no_compilationFlag!('/WX')
            facet.librarianOptions >> '/SUBSYSTEM:WINDOWS' >> '/WX'
            facet.linkerOptions >> '/LTCG:INCREMENTAL' >> '/LTCG' >> '/LTCG:OFF' >> '/WX'
        end
        def dbg_env()
            env = super()
            env['PATH'].prepend(File.join(@llvmPath, 'bin')<<';')
            return env
        end
        def add_includePath(facet, dirpath)
            super(facet, dirpath)
        end
        def add_externPath(facet, dirpath)
            add_compilationFlag(facet, "/imsvc\"#{dirpath}\"")
        end
        alias add_systemPath add_externPath
        def freeze()
            @llvmPath.freeze
            @llvmVersion.freeze
            super()
        end
    end #~ LLVMWindowsCompiler

    def self.import_llvm_windows(name, path)
        Build.import_fileset(name,
            File.join(path, 'bin', 'clang-cl.exe'),
            File.join(path, 'bin', 'llvm-lib.exe'),
            File.join(path, 'bin', 'lld-link.exe') )
    end

    import_llvm_windows(:LLVM_Windows_x86, 'C:\Program Files (x86)\LLVM')
    import_llvm_windows(:LLVM_Windows_x64, 'C:\Program Files\LLVM')

    def LLVM_Windows_Hostx86_FileSet()
        Build.LLVM_Windows_x86 ? Build.LLVM_Windows_x86 : Build.LLVM_Windows_x64
    end
    def LLVM_Windows_Hostx64_FileSet()
        Build.LLVM_Windows_x64
    end

    make_facet(:LLVM_Windows_Base) do
        defines << 'CPP_CLANG' << 'LLVM_FOR_WINDOWS' << '_CRT_SECURE_NO_WARNINGS'

        warningOptions = [ # fix windows headers
            '-Wno-ignored-pragma-optimize',         # pragma optimize n'est pas supporté
            '-Wno-unused-command-line-argument',    # ignore les options non suportées par CLANG (sinon échoue a cause de /WError)
            '-Wno-ignored-attributes',              # ignore les attributs de classe/fonction non supportées par CLANG (sinon échoue a cause de /WError)
            '-Wno-unknown-pragmas',                 # ignore les directives pragma non supportées par CLANG (sinon échoue a cause de /WError)
            '-Wno-unused-local-typedef',            # ignore les typedefs locaux non utilisés (nécessaire pour STATIC_ASSERT(x))
            '-Wno-#pragma-messages',                # don't consider #pragma message as warnings
            '-Wno-unneeded-internal-declaration',   # ignore unused internal functions beeing stripped
        ]

        warningOptions << (Build.Strict ? '-Werror' : '-Wno-error')

        compilationFlag!(*warningOptions)
        compilationFlag!("-fmsc-version=#{Visual::MSC_VER_2019}")
        compilationFlag!('-msse4.2', '-Xclang', "-std=#{Build.CppStd}")
        compilationFlag!(*%w{ -fms-compatibility -fms-extensions -fcolor-diagnostics })
        compilationFlag!(*%w{ /clang:-fno-elide-type /clang:-fdiagnostics-show-template-tree })

        systemPaths <<
            File.join('$LLVMPath$', 'include', 'clang-c') <<
            File.join('$LLVMPath$', 'include', 'llvm-c') <<
            File.join('$LLVMPath$', 'lib', 'clang', '$LLVMVersion$', 'include')

        libraryPaths <<
            File.join('$LLVMPath$', 'lib') <<
            File.join('$LLVMPath$', 'lib', 'clang', '$LLVMVersion$', 'lib', 'windows')
    end

    make_facet(:LLVM_Windows_Base_x86) do
        compilationFlag!('-m32')
    end
    make_facet(:LLVM_Windows_Base_x64) do
        compilationFlag!('-m64')
    end

    def self.make_llvmwindows_compiler(version, llvm_fileset, vs_fileset)
        return unless llvm_fileset and vs_fileset

        clang_cl, llvm_lib, lld_link = *llvm_fileset

        Log.log 'Windows: found LLVM Windows compiler in "%s"', clang_cl

        cl_exe = vs_fileset.first
        fileset = vs_fileset[1..-1]

        return LLVMWindowsCompiler.new(
            'LLVM_Windows', version,
            *VisualStudioCompiler.infos_from(cl_exe),
            clang_cl, llvm_lib, lld_link, *fileset )
    end

    const_memoize(self, :LLVM_Windows_VS2019_Hostx86) do
        Build.make_llvmwindows_compiler('ClangCl_VS2019',
            Build.LLVM_Windows_Hostx86_FileSet,
            Build.VsWhere_2019_Hostx86 )
    end
    const_memoize(self, :LLVM_Windows_VS2019_Hostx64) do
        Build.make_llvmwindows_compiler('ClangCl_VS2019',
            Build.LLVM_Windows_Hostx64_FileSet,
            Build.VsWhere_2019_Hostx64 )
    end

    def LLVM_Windows_Hostx86() Build.LLVM_Windows_VS2019_Hostx86 end
    def LLVM_Windows_Hostx64() Build.LLVM_Windows_VS2019_Hostx64 end

end #~ Build
