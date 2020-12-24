# frozen_string_literal: true

require 'set'

class VulkanHeaderGenerator
    VK_TYPEDEF_DEFINE = /^\s*#\s*define\s+(VK_[\w\d]+)\s+(.*)/
    VK_TYPEDEF_ALIAS = /typedef\s+([\w\d]+)\s+(Vk[\w\d]+);/
    VK_DEFINE_HANDLE = /VK_DEFINE_HANDLE\((Vk[\w\d]+)\)/
    VK_DEFINE_NON_DISPATCHABLE_HANDLE = /VK_DEFINE_NON_DISPATCHABLE_HANDLE\((Vk[\w\d]+)\)/
    VK_TYPEDEF_ENUM = /typedef\s+enum\s+(Vk[\w\d]+)\s+\{(.*?)\}\s+(Vk[\w\d]+)\s*;/m
    VK_TYPEDEF_STRUCT = /typedef\s+struct\s+(Vk[\w\d]+)\s+\{(.*?)\}\s+(Vk[\w\d]+)\s*;/m
    VK_TYPEDEF_PFN = /typedef\s+([\w\d]+\*?)\s+\(VKAPI_PTR\s+\*(PFN_[\w\d]+)\)\((.*?)\)\s*;/m

    attr_reader :fwd, :symbols

    class Symbol
        attr_reader :type, :name, :expansion
        attr_reader :dependencies, :inner
        def initialize(type, name, expansion)
            @type = type
            @name = name.to_s
            @expansion = expansion
            @dependencies = Set.new
            @inner = nil
        end
        def to_s() @expansion end
        def depends!(name) @dependencies << name.to_s; return self end
        def inner!(inner) @inner = inner; return self end

        def resolve!(generator)
            unless @inner.nil?
                tokens = @inner.split(/\W+/)
                tokens.each do |token|
                    depends!(token) if generator.symbols.include?(token)
                end
            end
            return self
        end
    end

    def initialize(fwd)
        @fwd = fwd ? true : false
        @symbols = {}
    end

    def declare!(type, name, expansion)
        sym = Symbol.new(type, name, expansion)
        Build::Log.fatal('Vulkan symbol <%s> declared twice', sym.name) if @symbols.include?(sym.name)
        @symbols[sym.name] = sym
        return sym
    end

    def parse_file!(path)
        header = File.read(path)
        header.scan(VK_TYPEDEF_DEFINE) do |m|
            declare!(:define, m[0], '#define %s %s' % [m[0],m[1]]).inner!(m[1])
        end
        header.scan(VK_TYPEDEF_ALIAS) do |m|
            declare!(:alias, m[1], 'typedef %s %s;' % [m[0],m[1]]).depends!(m[0])
        end
        header.scan(VK_DEFINE_HANDLE) do |m|
            declare!(:handle, m[0], 'VK_DEFINE_HANDLE(%s)' % m[0])
        end
        header.scan(VK_DEFINE_NON_DISPATCHABLE_HANDLE) do |m|
            declare!(:non_dispatchable_handle, m[0], 'VK_DEFINE_NON_DISPATCHABLE_HANDLE(%s)' % m[0])
        end
        header.scan(VK_TYPEDEF_ENUM) do |m|
            declare!(:enum, m[2], @fwd ?
                'enum %s : int;' % m[2] :
                'typedef enum %s {%s} %s;' % m )
        end
        header.scan(VK_TYPEDEF_STRUCT) do |m|
            declare!(:struct, m[2], @fwd ?
                'struct %s;' % m[2] :
                'typedef struct %s {%s} %s;' % m).inner!(m[1])
        end
        header.scan(VK_TYPEDEF_PFN) do |m|
            @symbols[m[1].delete_prefix('PFN_')] =
                declare!(:pfn, m[1], 'typedef %s (VKAPI_PTR *%s)(%s);' % [m[0],m[1],m[2]]).depends!(m[0]).inner!(m[2])
        end
        return self
    end

    def resolve_depends!()
        @symbols.values.each do |sym|
            sym.resolve!(self)
        end
        return self
    end

    def expand_header(dst, inl)
        head = Symbol.new(:tmp, inl, "// #{File.basename(inl)}")
        head.inner!(File.read(inl))
        head.resolve!(self)
        visited = Set.new
        @symbols.values.each do |sym|
            if sym.type == :define
                visited << sym.name
                expand_header_rec(dst, sym, visited)
            end
        end
        expand_header_rec(dst, head, visited)
        return head
    end

private
    def expand_header_rec(dst, sym, visited)
        sym.dependencies.each do |name|
            if !visited.include?(name)
                visited << name
                dep = @symbols[name]
                expand_header_rec(dst, dep, visited) unless dep.nil?
            end
        end
        dst.puts!(sym)
        return
    end

end

def vk_generate_fwd(inl, dst, *src)
    fwd = VulkanHeaderGenerator.new(true)
    src.each{ |path| fwd.parse_file!(path) }
    fwd.resolve_depends!
    fwd.expand_header(dst, inl)
end
def vk_generate_minimal(inl, dst, *src)
    fwd = VulkanHeaderGenerator.new(false)
    src.each{ |path| fwd.parse_file!(path) }
    fwd.resolve_depends!
    fwd.expand_header(dst, inl)
end

$Build.ppe_external!(:vulkan) do
    extra_files!(*%w{
        Public/vulkan-external.h
        Public/vulkan-exports.inl
        Public/vulkan-fwd.h
        Public/vulkan-fwd.generated.h
    })
    source_files!(*%w{
        Private/vulkan.cpp
    })
    generate!('vulkan-minimal.generated.h', :public) do |facet, env, io|
        inc = expand_path('Vulkan-Header.git/include/vulkan')
        inc = env.source_path(inc)

        src = [ File.join(inc, 'vulkan_core.h') ]

        case env.platform.os
        when :Windows
            src << File.join(inc, 'vulkan_win32.h')
        when :Linux
            src << File.join(inc, 'vulkan_xlib.h')
        else
            Log.fatal 'unsupported os <%s>', env.platform.os
        end

        io.puts!("// Vulkan minimal header generated by #{Build::Script} v#{Build::VERSION}")
        io.puts!('extern "C" {')
        io.puts!('#include "%s"' % expand_path('Vulkan-Header.git/include/vulkan/vk_platform.h'))

        inl = expand_path('Public/vulkan-exports.inl')
        inl = env.source_path(inl)
        vk_generate_minimal(inl, io, *src)

        io.puts!('} //! extern "C"')
    end
end
