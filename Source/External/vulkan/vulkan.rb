# frozen_string_literal: true

require 'set'

class HeaderParser
    class Symbol
        attr_reader :type, :name, :expansion
        attr_reader :dependencies, :inner
        def initialize(type, name, expansion)
            @type = type
            @name = name.to_s
            @expansion = expansion
            @dependencies = Set.new
            @inner = nil
            @opaque = {}
        end
        def to_s() @expansion end
        def front?() return @dependencies.first end
        def depends!(name) @dependencies << name.to_s; return self end
        def inner!(inner) @inner = inner; return self end

        def opaque!(key, value) @opaque[key] = value; return self end
        def opaque?(key) return @opaque[key] end

        def resolve!(symbols)
            unless @inner.nil?
                tokens = @inner.split(/\W+/)
                tokens.each do |token|
                    if symbols.include?(token)
                        depends!(token)
                    end
                end
            end
            return self
        end

        def expand_rec(dst, symbols, visited)
            if visited.add?(@name)
                @dependencies.each do |dep|
                    sym = symbols[dep]
                    sym.expand_rec(dst, symbols, visited) unless sym.nil?
                end
                dst.puts!(@expansion)
            end
            return
        end
    end

    class SymbolTable
        attr_reader :symbols, :parent
        def initialize(parent = nil)
            @table = nil
            @symbols = {}
        end
        @@_symcache = {}
        def self.clear_cache()
            @@_symcache = {}
        end
        def open!(fname, &parser)
            Build::Assert.check{ @table.nil? }
            @table = @@_symcache[fname]
            if @table.nil?
                Build::Log.verbose('Parsing header "%s"', fname)
                @table = @@_symcache[fname] = {}
                @symbols[fname] = @table
                input = File.read(fname)
                parser.call(input, @table)
            else
                @symbols[fname] = @table
            end
            @table = nil
            return self
        end
        def include?(name)
            @symbols.values.each do |table|
                return true if table.include?(name)
            end
            return false
        end
        def [](name)
            @symbols.values.each do |table|
                sym = table[name]
                return sym unless sym.nil?
            end
            return nil
        end
        def []=(name, sym)
            Build::Assert.expect(name, String)
            Build::Assert.expect(sym, Symbol)
            @table[name] = sym
        end
        def each_sym(&foreach)
            @symbols.values.each do |table|
                table.values.each(&foreach)
            end
            return self
        end
    end

    def self.re(re, &for_each)
        lambda do |input|
            owner = self
            input.scan(re) do |match|
                owner.instance_exec(match, &for_each)
            end
        end
    end

    attr_reader :rules, :symbols

    def initialize(rules)
        @rules = rules
        @symbols = SymbolTable.new
    end

    def source_files()
        return @symbols.symbols.keys
    end

    def parse_file!(path, resolver=self)
        @symbols.open!(path) do |input, table|
            @rules.each do |key, parser|
                instance_exec(input, &parser)
            end
            table.values.each do |sym|
                sym.resolve!(resolver.symbols)
            end
        end
        return self
    end

    def declare!(type, name, expansion = nil)
        sym = Symbol.new(type, name, expansion)
        #Build::Log.debug('Declare vulkan symbol "%s" of type <%s>: %s', name, type, expansion)
        Build::Log.fatal("Vulkan symbol '%s' of type <%s> declared twice:\n#{dst.tab}previous: %s\n#{dst.tab}new: %s", sym.name, sym.type, @symbols[sym.name], expansion) if @symbols.include?(sym.name)
        @symbols[sym.name] = sym
        return sym
    end

end #~ HeaderParser

class VulkanAPIGenerator < HeaderParser

    PARSER_RULES = {
        vk_typedef_alias: HeaderParser.re(/typedef\s+([\w\d]+)\s+(Vk[\w\d]+);/) do |m|
            declare!(:alias, m[1], 'typedef %s %s;' % [m[0],m[1]]).depends!(m[0])
        end,
        vk_typedef_decline: HeaderParser.re(/^\s*#\s*define\s+(VK_[\w\d]+)\s+(.*)/) do |m|
            declare!(:define, m[0], '#define %s %s' % [m[0],m[1]]).inner!(m[1])
        end,
        vk_decline_handle: HeaderParser.re(/VK_DEFINE_HANDLE\((Vk[\w\d]+)\)/) do |m|
            declare!(:handle, m[0], 'VK_DEFINE_HANDLE(%s)' % m[0])
        end,
        vk_decline_non_dispatchable_handle: HeaderParser.re(/VK_DEFINE_NON_DISPATCHABLE_HANDLE\((Vk[\w\d]+)\)/) do |m|
            declare!(:non_dispatchable_handle, m[0], 'VK_DEFINE_NON_DISPATCHABLE_HANDLE(%s)' % m[0])
        end,
        vk_typedef_enum: HeaderParser.re(/typedef\s+enum\s+(Vk[\w\d]+)\s+\{(.*?)\}\s+(Vk[\w\d]+)\s*;/m) do |m|
            declare!(:enum, m[2], @fwd ? 'enum %s : int;' % m[2] : 'typedef enum %s {%s} %s;' % m )
        end,
        vk_typedef_struct: HeaderParser.re(/typedef\s+struct\s+(Vk[\w\d]+)\s+\{(.*?)\}\s+(Vk[\w\d]+)\s*;/m) do |m|
            declare!(:struct, m[2], @fwd ? 'struct %s;' % m[2] : 'typedef struct %s {%s} %s;' % m).inner!(m[1])
        end,
        vk_typedef_pfn: HeaderParser.re(/typedef\s+([\w\d]+\*?)\s+\(VKAPI_PTR\s+\*(PFN_[\w\d]+)\)\((.*?)\)\s*;/) do |m|
            @symbols[m[1].delete_prefix('PFN_')] = declare!(:pfn, m[1], 'typedef %s (VKAPI_PTR *%s)(%s);' % [m[0],m[1],m[2]]).
                depends!(m[0]).
                inner!(m[2])
        end
    }

    attr_reader :fwd

    def initialize(fwd)
        super(PARSER_RULES)
        @fwd = fwd
    end

    def expand_header(dst, inl)
        link!()
        head = Symbol.new(:tmp, inl, "// #{File.basename(inl)}")
        head.inner!(File.read(inl))
        head.resolve!(self)
        visited = Set.new
        @symbols.each_sym do |sym|
            sym.expand_rec(dst, @symbols, visited) if sym.type == :define
        end
        head.expand_rec(dst, @symbols, visited)
        return head
    end

end #~ VulkanAPIGenerator

class VulkanFunctionsGenerator < HeaderParser

    PARSER_RULES = {
        exported_function: HeaderParser.re(/^\s*VK_EXPORTED_FUNCTION\(\s*(vk[\w\d]+)\s*\)\s*$/) do |m|
            declare!(:exported_function, m[0])
        end,
        global_level_function: HeaderParser.re(/^\s*VK_GLOBAL_LEVEL_FUNCTION\(\s*(vk[\w\d]+)\s*\)\s*$/) do |m|
            declare!(:global_level_function, m[0])
        end,
        instance_level_function: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_FUNCTION\(\s*(vk[\w\d]+)\s*\)\s*$/) do |m|
            declare!(:instance_level_function, m[0])
        end,
        instance_level_function_from_extension: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION\(\s*(vk[\w\d]+)\s*,\s*(VK_[\w\d_]+)\)\s*$/) do |m|
            declare!(:instance_level_function_from_extension, m[0], m[1])
        end,
        device_level_function: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_FUNCTION\(\s*(vk[\w\d]+)\s*\)\s*$/) do |m|
            declare!(:device_level_function, m[0])
        end,
        device_level_function_from_extension: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION\(\s*(vk[\w\d]+)\s*,\s*(VK_[\w\d_]+)\)\s*$/) do |m|
            declare!(:device_level_function_from_extension, m[0], m[1])
        end
    }

    attr_reader :functions

    def initialize()
        super(PARSER_RULES)
        @functions = {}
        PARSER_RULES.keys.each {|k| @functions[k] = [] }
    end

    def header(api, env, dst, src)
        parse_file!(src, api)

        @symbols.each_sym do |sym|
            if PARSER_RULES.has_key?(sym.type)
                Build::Log.debug('Vulkan function "%s" of type <%s>', sym.name, sym.type)
                @functions[sym.type] << sym
            end
        end
        PARSER_RULES.keys.each {|k| @functions[k].sort!{|a,b| a.name <=> b.name } }

        api.source_files.each do |incl|
            dst.puts!("#include \"#{env.relative_path($SourcePath, incl)}\"")
        end

        # fwd_def(dst, api)

        dst.puts!('namespace Vulkan {')

        struct_decl(dst, 'FExported') do
            pfnvar_decl(dst, :exported_function)
        end

        struct_decl(dst, 'FGlobalLevelAPI') do
            memfun_decl(dst, 'AttachTo', '[[nodiscard]] const char*', 'const FExported& exported')
            pfnvar_decl(dst, :exported_function)
            pfnvar_decl(dst, :global_level_function)
        end
        memfun_def(dst, 'AttachTo', 'FGlobalLevelAPI', 'const char*', 'const FExported& exported') do
            each_func(:exported_function, api) do |name|
                dst.puts!("if (!(#{name} = exported.#{name}))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            each_func(:global_level_function, api) do |name|
                dst.puts!("if (!(#{name} = reinterpret_cast<PFN_#{name}>(exported.vkGetInstanceProcAddr(VK_NULL_HANDLE, \"#{name}\"))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            dst.puts!("return nullptr; // no error")
        end

        struct_decl(dst, 'FInstanceLevelAPI') do
            memfun_decl(dst, 'AttachTo', '[[nodiscard]] const char*', 'const FGlobalLevelAPI& global, VkInstance vkInstance')
            pfnvar_decl(dst, :instance_level_function)
            pfnvar_decl(dst, :instance_level_function_from_extension)
        end
        memfun_def(dst, 'AttachTo', 'FInstanceLevelAPI', 'const char*', 'const FGlobalLevelAPI& global, VkInstance vkInstance') do
            instance_attach = lambda do |name, result, args|
                dst.puts!("if (!(#{name} = reinterpret_cast<PFN_#{name}>(global.vkGetInstanceProcAddr(vkInstance, \"#{name}\"))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            each_func(:instance_level_function, api, &instance_attach)
            each_func(:instance_level_function_from_extension, api, &instance_attach)
            dst.puts!("return nullptr; // no error")
        end

        struct_decl(dst, 'FDeviceLevelAPI') do
            memfun_decl(dst, 'AttachTo', '[[nodiscard]] const char*', 'const FInstanceLevelAPI& instance, VkDevice vkDevice')
            pfnvar_decl(dst, :device_level_function)
            pfnvar_decl(dst, :device_level_function_from_extension)
        end
        memfun_def(dst, 'AttachTo', 'FDeviceLevelAPI', 'const char*', 'const FInstanceLevelAPI& instance, VkDevice vkDevice') do
            each_func(:device_level_function, api) do |name|
                dst.puts!("if (!(#{name} = reinterpret_cast<PFN_#{name}>(instance.vkGetDeviceProcAddr(vkDevice, \"#{name}\"))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            each_func(:device_level_function_from_extension, api) do |name|
                dst.puts!("if (!(#{name} = reinterpret_cast<PFN_#{name}>(instance.vkGetDeviceProcAddr(vkDevice, \"#{name}\"))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            dst.puts!("return nullptr; // no error")
        end

        struct_decl(dst, 'FInstanceFunctions') do
            memvar_decl(dst, '_pInstanceLevel', 'const FInstanceLevelAPI*');
            pfnfun_decl(dst, :instance_level_function, '_pInstanceLevel', api)
            pfnfun_decl(dst, :instance_level_function_from_extension, '_pInstanceLevel', api)
        end
        pfnfun_def(dst, :instance_level_function, 'FInstanceFunctions', '_pInstanceLevel', api)
        pfnfun_def(dst, :instance_level_function_from_extension, 'FInstanceFunctions', '_pInstanceLevel', api)

        struct_decl(dst, 'FDeviceFunctions') do
            memvar_decl(dst, '_pDeviceLevel', 'const FDeviceLevelAPI*');
            pfnfun_decl(dst, :device_level_function, '_pDeviceLevel', api)
            pfnfun_decl(dst, :device_level_function_from_extension, '_pDeviceLevel', api)
        end
        pfnfun_def(dst, :device_level_function, 'FDeviceFunctions', '_pDeviceLevel', api)
        pfnfun_def(dst, :device_level_function_from_extension, 'FDeviceFunctions', '_pDeviceLevel', api)

        dst.puts!('} //!namespace Vulkan')

        return self
    end

private
    def fwd_def(dst, api)
        dst.heredoc! %{
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#if !defined(VK_DEFINE_NON_DISPATCHABLE_HANDLE)
#   if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
        #define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#   else
        #define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#   endif
#endif
}

        fwd = Set.new
        dst.heredoc! %{
#ifdef __cplusplus
extern "C" \{
#endif

}
        dst.puts!(api.symbols['VkFlags'].expansion) if fwd.add?('VkFlags')
        fwd_decl(dst, :exported_function, api, fwd)
        fwd_decl(dst, :global_level_function, api, fwd)
        fwd_decl(dst, :instance_level_function, api, fwd)
        fwd_decl(dst, :instance_level_function_from_extension, api, fwd)
        fwd_decl(dst, :device_level_function, api, fwd)
        fwd_decl(dst, :device_level_function_from_extension, api, fwd)
        dst.heredoc! %{

#ifdef __cplusplus
\} //! extern "C"
#endif

}
    end
    def fwd_decl(dst, type, api, fwd=Set.new)
        @functions[type].each do |fun|
            sym = api.symbols[fun.name]
            sym.dependencies.each do |dep|
                if fwd.add?(dep)
                    dep = api.symbols[dep]
                    next if dep.nil?
                    dst.puts!(dep.expansion)
                end
            end
            dst.puts!(sym)
        end
    end
    def struct_decl(dst, name, &scope)
        dst.puts!("struct #{name} {")
        dst.indent!
        scope.call()
        dst.unindent!
        dst.puts!('};')
    end
    def memfun_decl(dst, name, result, args, const=false)
        dst.puts!("#{result} #{name}(#{args})#{const ? ' const' : ''}#{block_given? ? ' {' : ';'}")
        if block_given?
            dst.indent!
            yield name, result, args
            dst.unindent!
            dst.puts!('}')
        end
    end
    def memfun_def(dst, name, struct, result, args, const=false, &block)
        dst.puts!("#{result} #{struct}::#{name}(#{args})#{const ? ' const' : ''} {")
        dst.indent!
        block.call(name, struct, result, args)
        dst.unindent!
        dst.puts!('}')
    end
    def memvar_decl(dst, name, type, init = "{ nullptr }")
        dst.puts!("#{type} #{name}#{init};")
    end
    def pfnvar_decl(dst, type)
        @functions[type].each do |sym|
            memvar_decl(dst, sym.name, "PFN_#{sym.name}")
        end
    end
    def pfnfun_decl(dst, type, level, api)
        @functions[type].each do |sym|
            pfn = api.symbols[sym.name]
            memfun_decl(dst, sym.name, pfn.front?, pfn.inner, true)# do |name, result, args|
            #     is_void = (result == 'void')
            #     prms = args.split(/\s*,\s*/).collect! do |arg|
            #         arg.split(/\s+/).last
            #     end
            #     dst.puts!("#{is_void ? '' : 'return '}#{level}->_#{name}(#{prms.join(', ')});")
            # end
        end
    end
    def pfnfun_def(dst, type, struct, level, api)
        @functions[type].each do |sym|
            pfn = api.symbols[sym.name]
            memfun_def(dst, sym.name, struct, pfn.front?, pfn.inner, true) do |name, struct, result, args|
                is_void = (result == 'void')
                prms = args.split(/\s*,\s*/).collect! do |arg|
                    arg.split(/\s+/).last
                end
                dst.puts!("#{is_void ? '' : 'return '}#{level}->#{name}(#{prms.join(', ')});")
            end
        end
    end
    def each_func(type, api, &block)
        @functions[type].each do |sym|
            pfn = api.symbols[sym.name]
            block.call(sym.name, pfn.front?, pfn.inner)
        end
    end

end #~ VulkanDeviceFunctionsGenerator

def vk_generate_minimal(exports_inl, env, dst, *src)
    api = VulkanAPIGenerator.new(false)
    src.each{ |path| api.parse_file!(path) }

    dst.puts!('extern "C" {')

    api.expand_header(dst, exports_inl)

    dst.puts!('} //! extern "C"')
    return api
end
def vk_generate_api(exports_inl, env, dst, *src)
    api = VulkanAPIGenerator.new(true)
    src.each{ |path| api.parse_file!(path) }

    fns = VulkanFunctionsGenerator.new()
    fns.header(api, env, dst, exports_inl)
    return fns
end

$Build.ppe_external!(:vulkan) do
    extra_files!(*%w{
        Public/vulkan-external.h
        Public/vulkan-exports.inl
        Public/vulkan-fwd.h
        Public/vulkan-platform.h
    })
    source_files!(*%w{
        Private/vulkan.cpp
    })
    generate!('vulkan-exports.generated.h', :generated) do |facet, env, io|
        vk_path = expand_path('Vulkan-Header.git/include/vulkan')
        vk_path = env.source_path(vk_path)

        vk_headers = [ File.join(vk_path, 'vulkan_core.h') ]

        case env.platform.os
        when :Windows
            vk_headers << File.join(vk_path, 'vulkan_win32.h')
        when :Linux
            vk_headers << File.join(vk_path, 'vulkan_xlib.h')
        else
            Log.fatal 'unsupported os <%s>', env.platform.os
        end

        exports_inl = expand_path('Public/vulkan-exports.inl')
        exports_inl = env.source_path(exports_inl)

        io.puts!("// Vulkan header generated by #{Build::Script} v#{Build::VERSION}")
        io.puts!("#pragma once")
        io.puts!('#include "%s"' % (expand_path('Vulkan-Header.git/include/vulkan/vk_platform.h')))

        vk_generate_api(exports_inl, env, io, *vk_headers)
    end
end
