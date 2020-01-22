require "rubygems"
gem "ffi"
require "ffi"

module CPUID
  X32_CODE = [
    0x55,                     # pushl   %ebp
    0x89, 0xE5,               # movl    %esp,%ebp
    0x57,                     # pushl   %edi
    0x56,                     # pushl   %esi
    0x8B, 0x45, 0x08,         # movl    0x08(%ebp),%eax
    0x89, 0xDE,               # movl    %ebx,%esi
    0x0F, 0xA2,               # cpuid
    0x87, 0xDE,               # xchgl   %esi,%ebx
    0x8B, 0x7D, 0x0C,         # movl    0x0c(%ebp),%edi
    0x89, 0x07,               # movl    %eax,(%edi)
    0x89, 0x77, 0x04,         # movl    %esi,0x04(%edi)
    0x89, 0x4F, 0x08,         # movl    %ecx,0x08(%edi)
    0x89, 0x57, 0x0C,         # movl    %edx,0x0c(%edi)
    0x5E,                     # popl    %esi
    0x5F,                     # popl    %edi
    0x5D,                     # popl    %ebp
    0xC3,                     # ret
  ]

  X64_CODE = [
    0x55,                     # pushq   %rbp
    0x48, 0x89, 0xE5,         # movq    %rsp,%rbp
    0x49, 0x89, 0xF0,         # movq    %rsi,%r8
    0x89, 0xF8,               # movl    %edi,%eax
    0x89, 0xDE,               # movl    %ebx,%esi
    0x0F, 0xA2,               # cpuid
    0x87, 0xDE,               # xchgl   %esi,%ebx
    0x41, 0x89, 0x00,         # movl    %eax,(%r8)
    0x41, 0x89, 0x70, 0x04,   # movl    %esi,0x04(%r8)
    0x41, 0x89, 0x48, 0x08,   # movl    %ecx,0x08(%r8)
    0x41, 0x89, 0x50, 0x0C,   # movl    %edx,0x0c(%r8)
    0x5D,                     # popq    %rbp
    0xC3,                     # ret
  ]

  def self.run_cpuid(fn)
    buffer = FFI::MemoryPointer.new(:uint32, 4)
    CPUID_FUNCTION.call(fn, buffer)
    buffer.get_array_of_uint32(0, 4)
  end

  class Flags
    attr_accessor :MMX,:SSE,:SSE2,:SSE3,:SSSE3,:SSE41,:SSE42,:AES,:AVX,:FMA3,:RDRAND
    attr_accessor :AVX2,:BMI1,:BMI2,:ADX,:SHA,:PREFETCHWT1
    attr_accessor :AVX512F,:AVX512CD,:AVX512PF,:AVX512ER,:AVX512VL,:AVX512BW,:AVX512DQ,:AVX512IFMA,:AVX512VBMI
    attr_accessor :x64,:ABM,:SSE4a,:FMA4,:XOP
    def each(&flag)
      instance_variables.each do |name|
        flag.call(name.to_s[1..-1], instance_variable_get(name))
      end
    end
  end #~ Flags

  def self.flags
    flags = Flags.new
    # Identify CPU
    eax, ebx, ecx, edx = run_cpuid(0)
    nIds = eax
    eax, ebx, ecx, edx = run_cpuid(0x80000000)
    nExIds = eax
    # Detect Features
    if (nIds >= 0x00000001)
      _, ebx, ecx, edx  = run_cpuid(0x00000001)
      flags.MMX         = (edx & (1 << 23)) != 0
      flags.SSE         = (edx & (1 << 25)) != 0
      flags.SSE2        = (edx & (1 << 26)) != 0
      flags.SSE3        = (ecx & (1 <<  0)) != 0
      flags.SSSE3       = (ecx & (1 <<  9)) != 0
      flags.SSE41       = (ecx & (1 << 19)) != 0
      flags.SSE42       = (ecx & (1 << 20)) != 0
      flags.AES         = (ecx & (1 << 25)) != 0
      flags.AVX         = (ecx & (1 << 28)) != 0
      flags.FMA3        = (ecx & (1 << 12)) != 0
      flags.RDRAND      = (ecx & (1 << 30)) != 0
    end
    if (nIds >= 0x00000007)
      _, ebx, ecx, edx  = run_cpuid(0x00000007)
      flags.AVX2        = (ebx & (1 <<  5)) != 0
      flags.BMI1        = (ebx & (1 <<  3)) != 0
      flags.BMI2        = (ebx & (1 <<  8)) != 0
      flags.ADX         = (ebx & (1 << 19)) != 0
      flags.SHA         = (ebx & (1 << 29)) != 0
      flags.PREFETCHWT1 = (ecx & (1 <<  0)) != 0
      flags.AVX512F     = (ebx & (1 << 16)) != 0
      flags.AVX512CD    = (ebx & (1 << 28)) != 0
      flags.AVX512PF    = (ebx & (1 << 26)) != 0
      flags.AVX512ER    = (ebx & (1 << 27)) != 0
      flags.AVX512VL    = (ebx & (1 << 31)) != 0
      flags.AVX512BW    = (ebx & (1 << 30)) != 0
      flags.AVX512DQ    = (ebx & (1 << 17)) != 0
      flags.AVX512IFMA  = (ebx & (1 << 21)) != 0
      flags.AVX512VBMI  = (ecx & (1 <<  1)) != 0
    end
    if (nExIds >= 0x80000001)
      _, ebx, ecx, edx  = run_cpuid(0x80000001)
      flags.x64         = (edx & (1 << 29)) != 0
      flags.ABM         = (ecx & (1 <<  5)) != 0
      flags.SSE4a       = (ecx & (1 <<  6)) != 0
      flags.FMA4        = (ecx & (1 << 16)) != 0
      flags.XOP         = (ecx & (1 << 11)) != 0
    end
    return flags
  end

  def self.vendor_id
    str = ""
    _, ebx, ecx, edx = run_cpuid(0)
    [ebx, edx, ecx].each do |reg|
      0.upto(3) do |idx|
        str << ((reg >> (idx * 8)) & 0xFF).chr
      end
    end
    str
  end

  module MemUtil extend FFI::Library
    ffi_lib "c"
    attach_function :mprotect, [:pointer, :size_t, :int], :int
    attach_function :posix_memalign, [:pointer, :size_t, :size_t], :int
    attach_function :getpagesize, [], :int
    attach_function :memset, [:pointer, :int, :size_t], :void
    PROT_NONE = 0x00
    PROT_READ = 0x01
    PROT_WRITE = 0x02
    PROT_EXEC = 0x04
    def self.allocate_pages(count)
    out_pointer = FFI::MemoryPointer.new(:pointer, 1)
    posix_memalign(out_pointer, page_size, page_size * count)
    out_pointer.read_pointer
  end
    def self.allocate_executable(code)
    pages = (code.size / page_size.to_f).ceil
    mem = allocate_pages(pages)
    memset(mem, 0x90, pages * page_size)
    mem.put_array_of_uint8(0, code)
    mprotect(mem, code.size, PROT_READ | PROT_EXEC)
    mem
  end
    def self.page_size
    @page_size ||= getpagesize
  end   end

  CPUID_FUNCTION = FFI::Function.new(:void, [:uint, :pointer],
                                     MemUtil.allocate_executable(FFI::Pointer.size == 4 ? X32_CODE : X64_CODE))
end
