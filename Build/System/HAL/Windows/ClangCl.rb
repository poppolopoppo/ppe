# frozen_string_literal: true

require_once '../../Common.rb'

require_once './VisualStudio.rb'

module Build

    class LLVMWindowsCompiler < VisualStudioCompiler
        attr_reader :llvmWindowsPath, :llvmWindowsVersion
        def initialize(
            prefix,
            version, minor_version,
            host, target,
            visualStudioPath, platformToolset,
            clang_cl, llvm_lib, lld_link, rc, *extra_files)
            super(prefix, version, minor_version, host, target,
                visualStudioPath, platformToolset,
                clang_cl, llvm_lib, lld_link, rc, *extra_files )

            @llvmWindowsPath = Pathname.new(File.join(File.dirname(clang_cl), '..'))
            @llvmWindowsPath = @llvmWindowsPath.cleanpath

            Log.fatal 'invalid LLVMWindows path "%s"', @llvmWindowsPath unless Dir.exist?(@llvmWindowsPath)
            self.facet.export!('LLVMWindowsPath', @llvmWindowsPath)

            versions = Dir.entries(File.join(@llvmWindowsPath, 'lib', 'clang'))
            versions.sort!
            @llvmWindowsVersion = versions.last

            Log.verbose 'Windows: found LLVM for Windows v%s in "%s"', @llvmWindowsVersion, @llvmWindowsPath
            self.facet.export!('LLVMWindowsVersion', @llvmWindowsVersion)

            self.inherits!(Build.LLVM_Windows_Base)
            self.inherits!(Build.send "LLVM_Windows_Base_#{target}")
        end
        def add_includePath(facet, dirpath)
            if dirpath.start_with?($SourcePath)
                super(facet, dirpath)
            else
                add_compilerOption(facet, "/imsvc\"#{dirpath}\"")
            end
        end
    end #~ LLVM

    def self.import_llvm(name, path)
        Build.import_fileset(name,
            File.join(path, 'bin', 'clang-cl.exe'),
            File.join(path, 'bin', 'llvm-lib.exe'),
            File.join(path, 'bin', 'lld-link.exe') )
    end

    import_llvm(:LLVM_Windows_x86, 'C:\Program Files (x86)\LLVM')
    import_llvm(:LLVM_Windows_x64, 'C:\Program Files\LLVM')

    def LLVM_Windows_Hostx86_FileSet()
        Build.LLVM_Windows_x86 ? Build.LLVM_Windows_x86 : Build.LLVM_Windows_x64
    end
    def LLVM_Windows_Hostx64_FileSet()
        Build.LLVM_Windows_x64
    end

    make_facet(:LLVM_Windows_Base) do
        defines << 'CPP_CLANG' << 'LLVM_FOR_WINDOWS' << '_CRT_SECURE_NO_WARNINGS'

        warningOptions = [ # fix windows headers
            '-Wno-error',
            '-Wno-ignored-pragma-optimize',         # pragma optimize n'est pas supporté
            '-Wno-unused-command-line-argument',    # ignore les options non suportées par CLANG (sinon échoue a cause de /WError)
            '-Wno-ignored-attributes',              # ignore les attributs de classe/fonction non supportées par CLANG (sinon échoue a cause de /WError)
            '-Wno-unknown-pragmas',                 # ignore les directives pragma non supportées par CLANG (sinon échoue a cause de /WError)
            '-Wno-unused-local-typedef',            # ignore les typedefs locaux non utilisés (nécessaire pour STATIC_ASSERT(x))
            '-Wno-#pragma-messages',                # don't consider #pragma message as warnings
        ]
        extraWarning = [
            '-Wno-invalid-noreturn',                # nécessaire pour STL M$
            '-Wno-dllimport-static-field-def',      # definition of dllimport static field (M$TL)
            '-Wno-nonportable-include-path',        # windows libs are filled with includes not matching file system case, this will ignore those
            '-Wno-inconsistent-missing-override',   # <optional> has a method without override ...
            '-Wno-expansion-to-defined',            # macro expansion producing 'defined' has undefined behavior (nécessaire pour WinAPI)
            '-Wno-int-to-void-pointer-cast',        # cast to 'void *' from smaller integer type 'unsigned long' (nécessaire pour WinAPI)
            '-Wno-macro-redefined',                 # '_MM_HINT_T0' macro redefined (nécessaire pour WinAPI)
            '-Wno-microsoft-enum-value',            # ignore les dépassements de valeurs d'enums (nécessaire pour WinAPI)
            '-Wno-missing-declarations',            # typedef requires a name (nécessaire pour DbgHelp)
            '-Wno-unused-value',                    # ignore les expressions non utilisées (UNUSED(x) ou CRT)
            '-Wno-microsoft-explicit-constructor-call',
            '-Wno-pragma-pack',
        ]

        compilerOptions.append(*warningOptions)
        compilerOptions.append("-fmsc-version=#{Visual::MSC_VER_2019}")
        compilerOptions.append('-msse4.2', '-Xclang', "-std=#{Build.CppStd}")
        compilerOptions.append(*%w{ -fms-compatibility -fms-extensions -fcolor-diagnostics })

        includePaths <<
            File.join('$LLVMWindowsPath$', 'include', 'clang-c') <<
            File.join('$LLVMWindowsPath$', 'include', 'llvm-c') <<
            File.join('$LLVMWindowsPath$', 'lib', 'clang', '$LLVMWindowsVersion$', 'include')
        libraryPaths <<
            File.join('$LLVMWindowsPath$', 'lib') <<
            File.join('$LLVMWindowsPath$', 'lib', 'clang', '$LLVMWindowsVersion$', 'lib', 'windows')
    end

    make_facet(:LLVM_Windows_Base_x86) do
        compilerOptions.append('-m32')
    end
    make_facet(:LLVM_Windows_Base_x64) do
        compilerOptions.append('-m64')
    end

    def self.make_llvmwindows_compiler(version, llvm_fileset, vs_fileset)
        return unless llvm_fileset and vs_fileset

        clang_cl, llvm_lib, lld_link = *llvm_fileset

        Log.debug 'Windows: found LLVM Windows compiler in "%s"', clang_cl

        cl_exe = vs_fileset.first
        fileset = vs_fileset[1..-1]

        rc_exe = Build.WindowsSDK_RC_exe

        return LLVMWindowsCompiler.new(
            'LLVM_Windows', version,
            *VisualStudioCompiler.infos_from(cl_exe),
            clang_cl, llvm_lib, lld_link, rc_exe, *fileset )
    end

    const_memoize(self, :LLVM_Windows_VS2019_Hostx86) do
        Build.make_llvmwindows_compiler('ClangCl_VS2019',
            Build.LLVM_Windows_Hostx86_FileSet,
            Build.VS2019_Hostx86_FileSet )
    end
    const_memoize(self, :LLVM_Windows_VS2019_Hostx64) do
        Build.make_llvmwindows_compiler('ClangCl_VS2019',
            Build.LLVM_Windows_Hostx64_FileSet,
            Build.VS2019_Hostx64_FileSet )
    end

    def LLVM_Windows_Hostx86() Build.LLVM_Windows_VS2019_Hostx86 end
    def LLVM_Windows_Hostx64() Build.LLVM_Windows_VS2019_Hostx64 end

end #~ Build