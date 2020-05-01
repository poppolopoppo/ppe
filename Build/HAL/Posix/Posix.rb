
require_once '../../Core/Environment.rb'

require_once 'LLVM.rb'
require_once 'PosixSDK.rb'

module Build

    const_memoize(self, :PosixConfig_Debug) { SharedConfigs::Debug.deep_dup.inherits!(Build.LLVM_Posix_Debug) }
    const_memoize(self, :PosixConfig_FastDebug) { SharedConfigs::FastDebug.deep_dup.inherits!(Build.LLVM_Posix_FastDebug) }
    const_memoize(self, :PosixConfig_Release) { SharedConfigs::Release.deep_dup.inherits!(Build.LLVM_Posix_Release) }
    const_memoize(self, :PosixConfig_Profiling) { SharedConfigs::Profiling.deep_dup.inherits!(Build.LLVM_Posix_Profiling) }
    const_memoize(self, :PosixConfig_Final) { SharedConfigs::Final.deep_dup.inherits!(Build.LLVM_Posix_Final) }

    const_memoize(self, :PosixCompiler_X86) do
        compiler = Build.LLVM_Posix_Hostx86
        compiler.deep_dup.inherits!(Build.PosixSDK_X86)
    end
    const_memoize(self, :PosixCompiler_X64) do
        compiler = Build.LLVM_Posix_Hostx64
        compiler.deep_dup.inherits!(Build.PosixSDK_X64)
    end

end #~ Build
