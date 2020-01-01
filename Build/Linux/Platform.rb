
FORCE_USAGE = []
YIELD_PARAMS.delete_if do |arg|
    if arg =~ /-llvm\d/i
        FORCE_USAGE << arg[1..-1].downcase.to_sym
        true
    else
        false
    end
end

class FBinaryDependency < FDependency
    attr_reader :binary, :path
    def initialize(name, binary)
        super(name)
        @binary = binary
    end
    def export(header)
        super(header)
        realpath = File.dirname(File.realpath(@path))
        header.set("#{@name}_PATH", @available ? realpath : 'nil')
    end
private
    def eval_()
        @path = %x{which #{@binary}}.chomp.strip
        return $?.success?
    end
end #~ FBinaryDependency

class FLLVMDependency < FBinaryDependency
    attr_reader :version
    def initialize(version, binary)
        @version = version
        @force_tag = "llvm#{@version}"
        super("LLVM_#{@version}", binary)
    end
    def export(header)
        super(header)

        enabled = @available and (FORCE_USAGE.empty? or force_usage)
        header.puts("#{enabled ? '#' : ';'}define USE_LLVM_#{@version}")
    end
end #~ FLLVMDependency

class FLinuxPlatform < FPlatform
    def initialize()
        super('Linux')
        depends 'LLVM_6', FLLVMDependency.new('6', 'clang-6.0')
        depends 'LLVM_7', FLLVMDependency.new('7', 'clang-7')
    end
end #~ FLinuxPlatform

SOLUTION_PLATFORM = FLinuxPlatform.new
SOLUTION_FBUILDCMD = File.join(SOLUTION_FBUILDROOT, 'Linux', 'fbuild')
