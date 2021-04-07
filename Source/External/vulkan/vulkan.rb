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

    def initialize(rules, symbols=SymbolTable.new)
        @rules = rules
        @symbols = symbols
    end

    def source_files()
        return @symbols.symbols.keys
    end

    def parse_file!(path, resolver=self)
        @symbols.open!(path) do |input, table|
            @rules.values.each do |parser|
                instance_exec(input, &parser)
            end
            table.values.each do |sym|
                sym.resolve!(resolver.symbols)
            end
        end
        return self
    end

    def alias!(name, sym)
        #Build::Log.debug('Declare vulkan symbol "%s" of type <%s>: %s', name, type, expansion)
        Build::Assert.check{ !@symbols.include?(name) }
        @symbols[name] = sym
        return sym
    end
    def declare!(type, name, expansion = nil)
        sym = Symbol.new(type, name, expansion)
        return alias!(sym.name, sym)
    end

end #~ HeaderParser

class VulkanAPIGenerator < HeaderParser

    PARSER_RULES = {
        # vk_typedef_alias: HeaderParser.re(/typedef\s+(\w+)\s+(Vk\w+);/) do |m|
        #     declare!(:alias, m[1], 'typedef %s %s;' % [m[0],m[1]]).depends!(m[0])
        # end,
        # vk_typedef_decline: HeaderParser.re(/^\s*#\s*define\s+(VK_\w+)\s+(.*)/) do |m|
        #     declare!(:define, m[0], '#define %s %s' % [m[0],m[1]]).inner!(m[1])
        # end,
        # vk_decline_handle: HeaderParser.re(/VK_DEFINE_HANDLE\((Vk\w+)\)/) do |m|
        #     declare!(:handle, m[0], 'VK_DEFINE_HANDLE(%s)' % m[0])
        # end,
        # vk_decline_non_dispatchable_handle: HeaderParser.re(/VK_DEFINE_NON_DISPATCHABLE_HANDLE\((Vk\w+)\)/) do |m|
        #     declare!(:non_dispatchable_handle, m[0], 'VK_DEFINE_NON_DISPATCHABLE_HANDLE(%s)' % m[0])
        # end,
        # vk_typedef_enum: HeaderParser.re(/typedef\s+enum\s+(Vk\w+)\s+\{(.*?)\}\s+(Vk\w+)\s*;/m) do |m|
        #     declare!(:enum, m[2], @fwd ? 'enum %s : int;' % m[2] : 'typedef enum %s {%s} %s;' % m )
        # end,
        # vk_typedef_struct: HeaderParser.re(/typedef\s+struct\s+(Vk\w+)\s+\{(.*?)\}\s+(Vk\w+)\s*;/m) do |m|
        #     declare!(:struct, m[2], @fwd ? 'struct %s;' % m[2] : 'typedef struct %s {%s} %s;' % m).inner!(m[1])
        # end,
        vk_typedef_pfn: HeaderParser.re(/typedef\s+(\w+\*?)\s+\(VKAPI_PTR\s+\*(PFN_\w+)\)\((.*?)\)\s*;/) do |m|
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
        exported_function: HeaderParser.re(/^\s*VK_EXPORTED_FUNCTION\(\s*(vk\w+)\s*\)\s*$/) do |m|
            declare!(:exported_function, m[0])
        end,
        global_level_function: HeaderParser.re(/^\s*VK_GLOBAL_LEVEL_FUNCTION\(\s*(vk\w+)\s*\)\s*$/) do |m|
            declare!(:global_level_function, m[0])
        end,
        instance_level_function: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_FUNCTION\(\s*(vk\w+)\s*\)\s*$/) do |m|
            declare!(:instance_level_function, m[0])
        end,
        instance_level_extension: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_EXTENSION\(\s*(VK\w+)\s*\)\s*$/) do |m|
            declare!(:instance_level_extension, m[0])
        end,
        instance_level_function_from_extension: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION\(\s*(vk\w+)\s*,\s*(VK_\w+)\s*\)\s*$/) do |m|
            declare!(:instance_level_function_from_extension, m[0], m[1])
        end,
        device_level_function: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_FUNCTION\(\s*(vk\w+)\s*\)\s*$/) do |m|
            declare!(:device_level_function, m[0])
        end,
        device_level_extension: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_EXTENSION\(\s*(VK\w+)\s*\)\s*$/) do |m|
            declare!(:device_level_extension, m[0])
        end,
        device_level_function_from_extension: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION\(\s*(vk\w+)\s*,\s*(VK_\w+)\s*\)\s*$/) do |m|
            declare!(:device_level_function_from_extension, m[0], m[1])
        end
    }

    attr_reader :functions

    def initialize()
        super(PARSER_RULES)
        @functions = {}
        @lz_macro = nil
        @lz_queue = []
        PARSER_RULES.keys.each {|k| @functions[k] = [] }
    end

    def parse_file!(src, api)
        super(src, api)

        @symbols.each_sym do |sym|
            if PARSER_RULES.has_key?(sym.type)
                Build::Log.debug('Vulkan function "%s" of type <%s>', sym.name, sym.type)
                @functions[sym.type] << sym
            end
        end

        #PARSER_RULES.keys.each {|k| @functions[k].sort!{|a,b| a.name <=> b.name } }

        return self
    end

    def header(api, env, dst, src)
        parse_file!(src, api)

        # fwd_def(dst, api)

        dst.puts!('namespace Vulkan {')

        struct_decl(dst, 'FExportedAPI') do
            pfnvar_decl(dst, :exported_function)
        end

        struct_decl(dst, 'FGlobalAPI') do
            memvar_decl(dst, 'pExportedAPI', 'const FExportedAPI*');
            memfun_decl(dst, 'Attach_ReturnError', 'NODISCARD const char*', 'const FExportedAPI* api')
            pfnvar_decl(dst, :exported_function)
            pfnvar_decl(dst, :global_level_function)
        end

        extenum_decl(dst, 'EInstanceEXT', :instance_level_extension)
        struct_decl(dst, 'FInstanceAPI') do
            memvar_decl(dst, 'pGlobalAPI', 'const FGlobalAPI*');
            memfun_decl(dst, 'Attach_ReturnError', 'NODISCARD const char*', 'const FGlobalAPI* api, VkInstance vkInstance')
            memvar_decl(dst, 'Extensions', 'EInstanceEXT', "{ EInstanceEXT::Unknown }")
            pfnvar_decl(dst, :instance_level_function)
            pfnvar_decl(dst, :instance_level_function_from_extension)
        end

        extenum_decl(dst, 'EDeviceEXT', :device_level_extension)
        struct_decl(dst, 'FDeviceAPI') do
            memvar_decl(dst, 'pInstanceAPI', 'const FInstanceAPI*');
            memfun_decl(dst, 'Attach_ReturnError', 'NODISCARD const char*', 'const FInstanceAPI* api, VkDevice vkDevice')
            memvar_decl(dst, 'Extensions', 'EDeviceEXT', "{ EDeviceEXT::Unknown }")
            pfnvar_decl(dst, :device_level_function)
            pfnvar_decl(dst, :device_level_function_from_extension)
        end

        struct_decl(dst, 'FInstanceFunctions') do
            memvar_decl(dst, '_pInstanceAPI', 'const FInstanceAPI*');
            # memfun_decl(dst, 'InstanceEXT', 'NODISCARD EInstanceEXT', '', true) do
            #     dst.puts!('return _pInstanceAPI->Extensions;')
            # end
            pfnfun_decl(dst, :instance_level_function, '_pInstanceAPI', api)
            pfnfun_decl(dst, :instance_level_function_from_extension, '_pInstanceAPI', api)
        end

        struct_decl(dst, 'FDeviceFunctions') do
            memvar_decl(dst, '_pDeviceAPI', 'const FDeviceAPI*');
            # memfun_decl(dst, 'DeviceEXT', 'NODISCARD EDeviceEXT', '', true) do
            #     dst.puts!('return _pDeviceAPI->Extensions;')
            # end
            pfnfun_decl(dst, :device_level_function, '_pDeviceAPI', api)
            pfnfun_decl(dst, :device_level_function_from_extension, '_pDeviceAPI', api)
        end

        dst.puts!('} //!namespace Vulkan')

        return self
    end

    def source(api, env, dst, src)
        parse_file!(src, api)

        dst.puts!('namespace Vulkan {')

        memfun_def(dst, 'Attach_ReturnError', 'FGlobalAPI', 'const char*', 'const FExportedAPI* api') do
            dst.puts!("pExportedAPI = api;")
            each_func(:exported_function, api) do |name|
                dst.puts!("if (nullptr == (#{name} = api->#{name}))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            each_func(:global_level_function, api) do |name|
                dst.puts!("if (nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetInstanceProcAddr(VK_NULL_HANDLE, \"#{name}\"))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            dst.puts!("return nullptr; // no error")
        end

        memfun_def(dst, 'Attach_ReturnError', 'FInstanceAPI', 'const char*', 'const FGlobalAPI* api, VkInstance vkInstance') do
            dst.puts!("pGlobalAPI = api;")
            each_func(:instance_level_function, api) do |name, pfn|
                dst.puts!("if (nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetInstanceProcAddr(vkInstance, \"#{name}\"))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            each_func(:instance_level_function_from_extension, api) do |name, sym|
                lazy_ifdef(dst, sym.expansion) do
                    dst.puts!("if ((Extensions & EInstanceEXT::#{extflag(sym.expansion)}) &&")
                    dst.puts!("    nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetInstanceProcAddr(vkInstance, \"#{name}\"))))")
                    dst.puts!("#{dst.tab}return \"#{name}\";")
                end
            end
            flush_ifdef!(dst)
            dst.puts!("return nullptr; // no error")
        end

        memfun_def(dst, 'Attach_ReturnError', 'FDeviceAPI', 'const char*', 'const FInstanceAPI* api, VkDevice vkDevice') do
            dst.puts!("pInstanceAPI = api;")
            each_func(:device_level_function, api) do |name|
                dst.puts!("if (nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetDeviceProcAddr(vkDevice, \"#{name}\"))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            each_func(:device_level_function_from_extension, api) do |name, sym|
                lazy_ifdef(dst, sym.expansion) do
                    dst.puts!("if ((Extensions & EDeviceEXT::#{extflag(sym.expansion)}) &&")
                    dst.puts!("    nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetDeviceProcAddr(vkDevice, \"#{name}\"))))")
                    dst.puts!("#{dst.tab}return \"#{name}\";")
                end
            end
            flush_ifdef!(dst)
            dst.puts!("return nullptr; // no error")
        end

        # # pfnfun_def(dst, :global_level_function, 'FInstanceFunctions', '_pInstanceAPI->pGlobalAPI', api)
        # pfnfun_def(dst, :instance_level_function, 'FInstanceFunctions', '_pInstanceAPI', api)
        # pfnfun_def(dst, :instance_level_function_from_extension, 'FInstanceFunctions', '_pInstanceAPI', api)

        # # pfnfun_def(dst, :global_level_function, 'FDeviceFunctions', '_pDeviceAPI->_pInstanceAPI->pGlobalAPI', api)
        # # pfnfun_def(dst, :instance_level_function, 'FDeviceFunctions', '_pDeviceAPI->_pInstanceAPI', api)
        # # pfnfun_def(dst, :instance_level_function_from_extension, 'FDeviceFunctions', '_pDeviceAPI->_pInstanceAPI', api)
        # pfnfun_def(dst, :device_level_function, 'FDeviceFunctions', '_pDeviceAPI', api)
        # pfnfun_def(dst, :device_level_function_from_extension, 'FDeviceFunctions', '_pDeviceAPI', api)

        dst.puts!('} //!namespace Vulkan')

        return self
    end

private
    def each_func(type, api, &block)
        @functions[type].each do |sym|
            pfn = api.symbols[sym.name]
            block.call(sym.name, sym)
        end
    end
    def extflag(extmacro)
        extmacro.delete_suffix('_EXTENSION_NAME')
    end

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
    def ifdef(dst, macro, &scope)
        return scope.call() if macro.nil?
        lvl = dst.reset_indent()
        dst.puts!("#ifdef #{macro}")
        dst.reset_indent(lvl)
        scope.call()
        dst.reset_indent()
        dst.puts!("#endif //!#{macro}")
        dst.reset_indent(lvl)
    end
    def lazy_ifdef(dst, macro, &scope)
        return scope.call() if macro.nil?
        if @lz_macro != macro
            flush_ifdef!(dst)
            @lz_macro = macro
        end
        @lz_queue << scope
        return
    end
    def flush_ifdef!(dst)
        unless @lz_queue.empty?
            ifdef(dst, @lz_macro) do
                @lz_queue.map(&:call)
            end
            @lz_queue.clear
        end
        @lz_macro = nil
        return self
    end
    def scope_decl(dst, type, name, &scope)
        dst.puts!("#{type} #{name} {")
        dst.indent!(&scope)
        dst.puts!("}; //!#{name}")
    end
    def enum_decl(dst, name, &scope) scope_decl(dst, 'enum class', name, &scope) end
    def struct_decl(dst, name, &scope) scope_decl(dst, 'struct', name, &scope) end
    def memfun_decl(dst, name, result, args, const=false)
        dst.puts!("#{result} #{name}(#{args})#{const ? ' const' : ''}#{block_given? ? ' {' : ';'}")
        if block_given?
            dst.indent! do
                yield name, result, args
            end
            dst.puts!('}')
        end
    end
    def memfun_def(dst, name, struct, result, args, const=false, &block)
        dst.puts!("#{result} #{struct}::#{name}(#{args})#{const ? ' const' : ''} {")
        dst.indent! do
            block.call(name, struct, result, args)
        end
        dst.puts!('}')
    end
    def memvar_decl(dst, name, type, init = "{ nullptr }")
        dst.puts!("#{type} #{name}#{init};")
    end

    def pfnvar_decl(dst, type)
        @functions[type].each do |sym|
            lazy_ifdef(dst, sym.expansion) do
                memvar_decl(dst, sym.name, "PFN_#{sym.name}")
            end
        end
        flush_ifdef!(dst)
    end
    def pfnfun_decl(dst, type, level, api)
        @functions[type].each do |sym|
            pfn = api.symbols[sym.name]
            lazy_ifdef(dst, sym.expansion) do
                unless pfn.nil?
                    result = pfn.front?
                    result = 'VKAPI_ATTR FORCE_INLINE '+result
                    result = 'NODISCARD '+result if result != 'void'
                    memfun_decl(dst, sym.name, result, pfn.inner, true) do |name, result, args|
                        prms = args.split(/\s*,\s*/).collect! do |arg|
                            arg.split(/\s+/).last
                        end
                        funcall = "#{level}->#{name}(#{prms.join(', ')});"
                        if result == 'void'
                            dst.puts!(funcall)
                        # elsif result == 'VkResult'
                        #     dst.puts!(funcall.prepend('const VkResult res = '))
                        #     dst.puts!("VULKAN_CHECKERROR(#{name}, res);")
                        #     dst.puts!('return res;')
                        else
                            dst.puts!(funcall.prepend('return '))
                        end
                    end
                else
                    dst.puts!("// #{sym.name}()")
                end
            end
        end
        flush_ifdef!(dst)
    end
    def pfnfun_def(dst, type, struct, level, api)
        @functions[type].each do |sym|
            pfn = api.symbols[sym.name]
            lazy_ifdef(dst, sym.expansion) do
                unless pfn.nil?
                    memfun_def(dst, sym.name, struct, pfn.front?, pfn.inner, true) do |name, struct, result, args|
                        prms = args.split(/\s*,\s*/).collect! do |arg|
                            arg.split(/\s+/).last
                        end
                        funcall = "#{level}->#{name}(#{prms.join(', ')});"
                        if result == 'void'
                            dst.puts!(funcall)
                        # elsif result == 'VkResult'
                        #     dst.puts!(funcall.prepend('const VkResult res = '))
                        #     dst.puts!("VULKAN_CHECKERROR(#{name}, res);")
                        #     dst.puts!('return res;')
                        else
                            dst.puts!(funcall.prepend('return '))
                        end
                    end
                else
                    dst.puts!("// #{sym.name}()")
                end
            end
        end
        flush_ifdef!(dst)
    end

    def extenum_decl(dst, name, type)
        exts = @functions[type]
        enum_decl(dst, name) do
            all = 0
            exts.each_with_index do |ext, i|
                ifdef(dst, ext.name) do
                    dst.puts!("#{extflag(ext.name)} = 1u << #{i}u,")
                end
                all |= (1 << i)
            end
            dst.puts!("All = 0x#{all.to_s(16)}u,")
            dst.puts!("Unknown = 0u,")
        end
        dst.puts!("ENUM_FLAGS(#{name});")
        dst.puts!("constexpr const char* #{name}_Name(#{name} ext) {")
        dst.indent!
            dst.puts!('switch(ext) {')
            exts.each do |ext|
                ifdef(dst, ext.name) do
                    dst.puts!("case #{name}::#{extflag(ext.name)}: return #{ext.name};")
                end
            end
            dst.puts!('default: return nullptr;')
            dst.puts!('}')
        dst.unindent!
        dst.puts!("}")
    end

end #~ VulkanDeviceFunctionsGenerator

def vk_system_headers(env, target)
    vk_path = target.expand_path('Vulkan-Header.git/include/vulkan')

    vk_headers = [ File.join(vk_path, 'vulkan_core.h') ]

    case env.platform.os
    when :Windows
        vk_headers << File.join(vk_path, 'vulkan_win32.h')
    when :Linux
        vk_headers << File.join(vk_path, 'vulkan_xlib.h')
    else
        Log.fatal 'unsupported os <%s>', env.platform.os
    end

    return vk_headers
end
def vk_generate_minimal(exports_inl, env, dst, *src)
    api = VulkanAPIGenerator.new(false)
    src.each{ |path| api.parse_file!(path) }

    dst.puts!('extern "C" {')

    api.expand_header(dst, exports_inl)

    dst.puts!('} //! extern "C"')
    return api
end
def vk_generate_header(exports_inl, env, dst, *src)
    dst.puts!("// Vulkan header generated by #{Build::Script} v#{Build::VERSION}")
    dst.puts!("#pragma once")
    dst.puts!('#include "Meta/Aliases.h"')
    dst.puts!('#include "Meta/Enum.h"')
    src.each {|p| dst.puts!("#include \"#{p}\"") }

    api = VulkanAPIGenerator.new(true)
    src.each{ |path| api.parse_file!(env.source_path(path)) }
    fns = VulkanFunctionsGenerator.new()
    fns.header(api, env, dst, exports_inl)
    return fns
end
def vk_generate_source(exports_inl, env, dst, *src)
    dst.puts!("// Vulkan source generated by #{Build::Script} v#{Build::VERSION}")
    dst.puts!("#pragma once")
    dst.puts!('#include "vulkan-exports.generated.h"')
    # dst.puts!('#ifndef VULKAN_CHECKERROR')
    # dst.puts!('  #define VULKAN_CHECKERROR(_VK_FUNC, _VK_RES) NOOP()')
    # dst.puts!('#endif //!VULKAN_CHECKERROR')
    api = VulkanAPIGenerator.new(true)
    src.each{ |path| api.parse_file!(env.source_path(path)) }
    fns = VulkanFunctionsGenerator.new()
    fns.source(api, env, dst, exports_inl)
    return fns
end

$Build.ppe_external!(:vulkan) do
    extra_files!(*%w{
        Public/vulkan-exports.h
        Public/vulkan-exports.inl
        Public/vulkan-external.h
        Public/vulkan-minimal.h
        Public/vulkan-fwd.h
        Public/vulkan-platform.h
    })
    source_files!(*%w{
        Private/vulkan.cpp
    })
    generate!('vulkan-exports.generated.h', :public) do |facet, env, io|
        vk_headers = vk_system_headers(env, self)
        exports_inl = source_path(env, 'Public/vulkan-exports.inl')
        vk_generate_header(exports_inl, env, io, *vk_headers)
    end
    generate!('vulkan-exports.generated.cpp', :private) do |facet, env, io|
        vk_headers = vk_system_headers(env, self)
        exports_inl = source_path(env, 'Public/vulkan-exports.inl')
        vk_generate_source(exports_inl, env, io, *vk_headers)
    end
end
