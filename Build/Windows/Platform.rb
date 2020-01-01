
FORCE_USAGE = []
YIELD_PARAMS.delete_if do |arg|
    if arg =~ /-vs20\d{2}/i
        FORCE_USAGE << arg[1..-1].downcase.to_sym
        true
    else
        false
    end
end

class FVisualStudio < FDependency
    attr_reader :major_version, :toolset, :default_comntools, :comntools, :cluid
    def initialize(major_version, toolset, default_comntools)
        @major_version = major_version
        @toolset = toolset
        @default_comntools = default_comntools
        @force_tag = "vs#{@major_version}".to_sym
        super("VISUALSTUDIO_TOOLSET_#{@toolset}")
    end
    def export(header)
        super(header)
        header.set("VS#{@toolset}CLUID", @available ? @cluid : nil)
        header.set("VS#{@toolset}COMNTOOLS", @available ? @comntools : nil)

        enabled = @available and (FORCE_USAGE.empty? or force_usage)
        header.puts("#{enabled ? '#' : ';'}define USE_VISUALSTUDIO_#{@major_version}")
    end
private
    def eval_comntools_()
        return false if @comntools.nil?
        binpath = File.join(@comntools, '..', '..', 'VC', 'bin')
        '00'.upto('99') do |id|
            @cluid = "10#{id}" if File.exist?(File.join(binpath, "10#{id}", 'clui.dll'))
        end
        return !@cluid.nil?
    end
    def eval_()
        envname = "VS#{@toolset}COMNTOOLS"
        @comntools = ENV[envname]
        return true if eval_comntools_()
        @comntools = @default_comntools
        success = eval_comntools_()
        ENV[envname] = @comntools if success
        return success
    end
end

class FVisualStudioPost2015 < FVisualStudio
    attr_reader :version
    def initialize(major_version, toolset)
        root = 'C:\Program Files (x86)\Microsoft Visual Studio\\' + major_version
        comntools_communuty = root + '\Community\Common7\Tools\\'
        comntools_professional = root + '\Professional\Common7\Tools\\'
        super(major_version, toolset, Dir.exist?(comntools_professional) ?
            comntools_professional :
            comntools_communuty )
    end
    def export(header)
        super(header)
        header.set("VS#{@toolset}VERSION", @available ? @version : nil)
    end
private
    def eval_()
        @comntools = @default_comntools
        msvcpath = File.join(@comntools, '..', '..', 'VC', 'Tools', 'MSVC')
        return false unless Dir.exist?(msvcpath)
        versions = Dir.entries(msvcpath)
        return false if versions.empty?
        @version = versions.sort.last
        binpath = File.join(@comntools, '..', '..', 'VC', 'Tools', 'MSVC', @version, 'bin', 'HostX64')
        return false unless Dir.exist?(binpath)
        '00'.upto('99') do |id|
            @cluid = "10#{id}" if File.exist?(File.join(binpath, 'x64', "10#{id}", 'clui.dll'))
        end
        return !@cluid.nil?
    end
end

class FWindowsSDK < FDependency
    BASEPATH='C:\Program Files (x86)\Windows Kits\\'
    attr_reader :winver, :version
    def initialize(winver)
        @winver = winver
        super("WINDOWS_SDK#{winver_s}")
    end
    def sdkpath() File.join(BASEPATH, @winver) end
    def winver_s() winver.to_s().gsub('.','') end
    def export(header)
        super(header)
        header.set("WindowsSDKBasePath#{winver_s}", @available ? sdkpath : nil)
        header.set("WindowsSDKVersion#{winver_s}", @available ? @version : nil)
    end
private
    def eval_()
        return false unless Dir.exist?(sdkpath)
        libpath = File.join(sdkpath, 'lib')
        return false unless Dir.exist?(libpath)
        sdkversions = Dir.entries(libpath)
        sdkversions.delete_if {|p| !Dir.exist?(File.join(libpath, p, 'um')) }
        raise "Can't find Windows SDK #{@winver} version !" if sdkversions.empty?
        @version = sdkversions.sort.last
        return true
    end
end

class FLLVMWindows < FDependency
    BASEPATH_X86 = 'C:\Program Files (x86)\LLVM'
    BASEPATH_X64 = 'C:\Program Files\LLVM'

    attr_reader :platform, :llvm_path, :clang_version
    def initialize()
        super('LLVM_WINDOWS')
    end
    def export(header)
        super(header)
        header.set("LLVMBasePath#{@platform}", @available ? @llvm_path : nil)
        header.set("LLVMClangVer#{@platform}", @available ? @clang_version : nil)
        header.set("LLVMWindowsBasePath", @available ? @llvm_path : nil)
        header.set("LLVMWindowsClangVer", @available ? @clang_version : nil)
    end
    def self.fetch_version?(llvm_path)
        clang_lib_path = File.join(llvm_path, 'lib', 'clang')
        entries = Dir.entries(clang_lib_path)
        if entries
            entries.delete_if{|x| x =~ /^\.+$/}
            entries.sort!
            return entries[0]
        else
            return nil
        end
    end
private
    def eval_()
        if Dir.exist?(BASEPATH_X64)
            @platform = 'X64'
            @llvm_path = BASEPATH_X64
        elsif Dir.exist?(BASEPATH_X86)
            @platform = 'X86'
            @llvm_path = BASEPATH_X86
        else
            @platform = nil
            @llvm_path = nil
            return false
        end
        @clang_version = FLLVMWindows.fetch_version?(@llvm_path)
        return (@clang_version != nil)
    end
end

class FWindowsPlatform < FPlatform
    def initialize()
        super("Windows")

        depends 'Visual Studio 2012'    , FVisualStudio.new('2012', '110', 'C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\\')
        depends 'Visual Studio 2013'    , FVisualStudio.new('2013', '120', 'C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\\')
        depends 'Visual Studio 2015'    , FVisualStudio.new('2015', '140', 'C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\\')
        depends 'Visual Studio 2017'    , FVisualStudioPost2015.new('2017', 141)
        depends 'Visual Studio 2019'    , FVisualStudioPost2015.new('2019', 142)

        depends 'Windows SDK 8.1'       , FWindowsSDK.new('8.1')
        depends 'Windows SDK 10'        , FWindowsSDK.new('10')

        depends 'LLVM for Windows'      , FLLVMWindows.new()
    end
end #~ FWindowsPlatform

SOLUTION_PLATFORM = FWindowsPlatform.new
SOLUTION_FBUILDCMD = File.join(SOLUTION_FBUILDROOT, 'Windows', 'FBuild.exe')
