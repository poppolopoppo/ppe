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
                @table.freeze
            else
                Build::Assert.check{ @table.frozen? }
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
        #Build::Log.debug('Declare vulkan symbol "%s": %s', name, sym.expansion)
        Build::Assert.check{
            if @symbols.include?(name)
                Build::Log.error('symbol registered twice <'+name+'>')
                false
            else
                true
            end
        }
        @symbols[name] = sym
        return sym
    end
    def declare!(type, name, expansion = nil)
        sym = Symbol.new(type, name, expansion)
        return alias!(sym.name, sym)
    end

    def inspect() return 'HeaderParser' end

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
        # vk_structure_type: HeaderParser.re(/^\s*(VK_STRUCTURE_TYPE_\w+)\s*=\s*(.*)\s*,\s*$/) do |m|
        #     @symbols[m[0]] = declare!(:structure_type, m[0], m[1])
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

    def self.parse_stype(type)
        stype = type.delete_prefix('Vk')
        suff = ''; stype.gsub!(/([A-Z]+)$/){|m| suff += m.to_s; '' }
        suff.insert(0, '_') unless suff.empty?
        return "VK_STRUCTURE_TYPE_#{stype.split(/(?=[A-Z0-9]+)/).collect{|x| x.upcase }.join('_')}#{suff}"
    end

end #~ VulkanAPIGenerator

class VulkanFunctionsGenerator < HeaderParser

    VK_TYPE_RE = /(Vk\w+)/

    PARSER_RULES = {
        exported_function: HeaderParser.re(/^\s*VK_EXPORTED_FUNCTION\(\s*(vk\w+)\s*\)\s*$/) do |m|
            declare!(:exported_function, m[0])
        end,
        global_level_function: HeaderParser.re(/^\s*VK_GLOBAL_LEVEL_FUNCTION\(\s*(vk\w+)\s*\)\s*$/) do |m|
            declare!(:global_level_function, m[0])
        end,
        instance_level_function: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_FUNCTION\(\s*(vk\w+)\s*,\s*VK_API_VERSION_(\d)_(\d)\s*\)\s*$/) do |m|
            vkver = "VK_API_VERSION_#{m[1]}_#{m[2]}"
            declare!(:instance_level_function, m[0]).inner!(vkver)
        end,
        instance_level_extension: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_EXTENSION\(\s*(VK.+?)\s*\)\s*$/) do |m|
            reqs = m[0].split(/\s*,\s*/)
            declare!(:instance_level_extension, reqs[0], reqs[1].nil? ? nil : reqs.drop(1))
        end,
        instance_level_function_from_extension: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION\(\s*(vk\w+)\s*,\s*(VK_\w+)\s*\)\s*$/) do |m|
            declare!(:instance_level_function_from_extension, m[0], m[1])
        end,
        instance_level_backward_compatibility: HeaderParser.re(/^\s*VK_INSTANCE_LEVEL_BACKWARD_COMPATIBILITY\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(vk\w+)\s*,\s*(vk\w+)\s*,\s*(VK_\w+)\s*\)\s*$/) do |m|
            sym = declare!(:instance_level_backward_compatibility, "#{m[2]}-#{m[0]}_#{m[1]}", m[3])
            sym.opaque!(:vk_version_major, m[0].to_i)
            sym.opaque!(:vk_version_minor, m[1].to_i)
            sym.opaque!(:vk_function_name, m[2])
            sym.opaque!(:vk_extension_name, m[4])
            sym
        end,
        device_level_function: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_FUNCTION\(\s*(vk\w+)\s*,\s*VK_API_VERSION_(\d)_(\d)\s*\)\s*$/) do |m|
            vkver = "VK_API_VERSION_#{m[1]}_#{m[2]}"
            declare!(:device_level_function, m[0]).inner!(vkver)
        end,
        device_level_extension: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_EXTENSION\(\s*(VK.+?)\s*\)\s*$/) do |m|
            reqs = m[0].split(/\s*,\s*/)
            declare!(:device_level_extension, reqs[0], reqs[1].nil? ? nil : reqs.drop(1))
        end,
        device_level_function_from_extension: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_FUNCTION_FROM_EXTENSION\(\s*(vk\w+)\s*,\s*(VK_\w+)\s*\)\s*$/) do |m|
            declare!(:device_level_function_from_extension, m[0], m[1])
        end,
        device_level_backward_compatibility: HeaderParser.re(/^\s*VK_DEVICE_LEVEL_BACKWARD_COMPATIBILITY\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(vk\w+)\s*,\s*(vk\w+)\s*,\s*(VK_\w+)\s*\)\s*$/) do |m|
            sym = declare!(:device_level_backward_compatibility, "#{m[2]}-#{m[0]}_#{m[1]}", m[3])
            sym.opaque!(:vk_version_major, m[0].to_i)
            sym.opaque!(:vk_version_minor, m[1].to_i)
            sym.opaque!(:vk_function_name, m[2])
            sym.opaque!(:vk_extension_name, m[4])
            sym
        end,
    }

    attr_reader :functions, :versions, :types_by_ext

    def initialize()
        super(PARSER_RULES)
        @functions = {}
        @types_by_ext = {}
        @versions = Set.new
        @lz_macro = nil
        @lz_queue = []
        PARSER_RULES.keys.each {|k| @functions[k] = [] }
    end

    def parse_file!(src, api)
        super(src, api)

        @symbols.each_sym do |sym|
            if PARSER_RULES.has_key?(sym.type)
                #Build::Log.debug('Vulkan function "%s" of type <%s>', sym.name, sym.type)
                (@functions[sym.type] |= []) << sym
                vkver = sym.inner
                @versions << vkver unless vkver.nil?
            end
        end

        # registerTypeByExt = lambda do |type, ext|
        #     stype = VulkanAPIGenerator.parse_stype(type)
        #     stype_decl = api.symbols[stype]
        #     next if stype_decl.nil?
        #     if ext.nil? || !@types_by_ext[nil].include?(type)
        #         set = @types_by_ext[ext]
        #         set = @types_by_ext[ext] = Set.new() if set.nil?
        #         set << type
        #     end
        # end
        # parseSignatureByExt = lambda do |sym, pfn, ext|
        #     next if pfn.nil?
        #     registerTypeByExt.call(pfn.front?, ext) if pfn.front? =~ VK_TYPE_RE
        #     pfn.inner.scan(VK_TYPE_RE) do |m|
        #         registerTypeByExt.call(m[0], ext)
        #     end
        # end

        # each_func(:instance_level_function, api) do |name, sym|
        #     parseSignatureByExt.call(sym, api.symbols[name], nil)
        # end
        # each_func(:device_level_function, api) do |name, sym|
        #     parseSignatureByExt.call(sym, api.symbols[name], nil)
        # end
        # each_func(:instance_level_function_from_extension, api) do |name, sym|
        #     parseSignatureByExt.call(sym, api.symbols[name], sym.expansion)
        # end
        # each_func(:device_level_function_from_extension, api) do |name, sym|
        #     parseSignatureByExt.call(sym, api.symbols[name], sym.expansion)
        # end

        #PARSER_RULES.keys.each {|k| @functions[k].sort!{|a,b| a.name <=> b.name } }

        return self
    end

    def header(api, env, dst, src)
        parse_file!(src, api)

        # fwd_def(dst, api)

        dst.puts!('namespace vk {')

        enum_decl(dst, 'api_version : uint32_t') do
            vklast = nil
            @versions.to_a.each do |vkver|
                vklast = vkver
                dst.puts!("#{verflag(vkver)} = #{vkver},")
            end
            dst.puts!("API_version_latest = #{verflag(vklast)},")
        end

        extenum_decl(dst, 'instance_extension', :instance_level_extension)
        extenum_decl(dst, 'device_extension', :device_level_extension)
        dst.puts!("instance_extension_set instance_extensions_require(const device_extension_set& in);")

        struct_decl(dst, 'exported_api') do
            pfnvar_decl(dst, :exported_function)
        end

        struct_decl(dst, 'global_api') do
            memvar_decl(dst, 'g_dummy', 'static const global_api', '');
            memvar_decl(dst, 'exported_api_', 'const exported_api*');
            memfun_decl(dst, 'attach_return_error', 'NODISCARD const char*', 'const exported_api* api')
            pfnvar_decl(dst, :exported_function)
            pfnvar_decl(dst, :global_level_function)
        end

        struct_decl(dst, 'instance_api') do
            memvar_decl(dst, 'g_dummy', 'static const instance_api', '');
            memvar_decl(dst, 'version_', 'api_version', '{ API_version_latest }');
            memvar_decl(dst, 'global_api_', 'const global_api*');
            memvar_decl(dst, 'instance_extensions_', 'instance_extension_set', "{}")
            memfun_decl(dst, 'attach_return_error', 'NODISCARD const char*', 'const global_api* api, VkInstance vkInstance, api_version version, const instance_extension_set& required, instance_extension_set optional = PPE::Default')
            memfun_decl(dst, 'setup_backward_compatibility', 'void', 'void')
            pfnvar_decl(dst, :instance_level_function)
            pfnvar_decl(dst, :instance_level_function_from_extension)
        end

        struct_decl(dst, 'device_api') do
            memvar_decl(dst, 'g_dummy', 'static const device_api', '');
            memvar_decl(dst, 'instance_api_', 'const instance_api*');
            memvar_decl(dst, 'device_extensions_', 'device_extension_set', "{}")
            memfun_decl(dst, 'attach_return_error', 'NODISCARD const char*', 'const class instance_fn& fn, VkDevice vkDevice, const device_extension_set& required, device_extension_set optional = PPE::Default')
            memfun_decl(dst, 'attach_return_error', 'NODISCARD const char*', 'const instance_api* api, VkDevice vkDevice, const device_extension_set& required, device_extension_set optional = PPE::Default')
            memfun_decl(dst, 'setup_backward_compatibility', 'void', 'void')
            pfnvar_decl(dst, :device_level_function)
            pfnvar_decl(dst, :device_level_function_from_extension)
        end

        class_decl(dst, 'instance_fn') do
            dst.puts!('protected:')
            dst.puts!('friend struct device_api;')
            memvar_decl(dst, 'instance_api_', 'const instance_api*');
            dst.puts!('public:')
            dst.puts!('instance_fn() = default;')
            dst.puts!('constexpr explicit instance_fn(const instance_api* api) : instance_api_(api) {}')
            dst.puts!('api_version version() const { return instance_api_->version_; }')
            dst.puts!('const instance_extension_set& instance_extensions() const { return instance_api_->instance_extensions_; }')
            pfnfun_decl(dst, :global_level_function, 'instance_api_->global_api_', api)
            pfnfun_decl(dst, :instance_level_function, 'instance_api_', api)
            pfnfun_decl(dst, :instance_level_function_from_extension, 'instance_api_', api)
        end

        class_decl(dst, 'device_fn') do
            dst.puts!('protected:')
            memvar_decl(dst, 'device_api_', 'const device_api*');
            dst.puts!('public:')
            dst.puts!('device_fn() = default;')
            dst.puts!('constexpr explicit device_fn(const device_api* api) : device_api_(api) {}')
            dst.puts!('api_version version() const { return device_api_->instance_api_->version_; }')
            dst.puts!('const device_api* api() const { return device_api_; }')
            dst.puts!('const device_extension_set& device_extensions() const { return device_api_->device_extensions_; }')
            pfnfun_decl(dst, :instance_level_function, 'device_api_->instance_api_', api)
            pfnfun_decl(dst, :instance_level_function_from_extension, 'device_api_->instance_api_', api)
            pfnfun_decl(dst, :device_level_function, 'device_api_', api)
            pfnfun_decl(dst, :device_level_function_from_extension, 'device_api_', api)
        end

        # dst.puts!("template <typename T> using type_t = PPE::Meta::TType<T>;")
        # @types_by_ext.each do |ext, types|
        #     lazy_ifdef(dst, ext) do
        #         types.each do |type|
        #             stype = VulkanAPIGenerator.parse_stype(type)
        #             dst.puts!("CONSTEXPR VkStructureType structure_type(type_t<#{type}>) { return #{stype}; }")
        #         end
        #     end
        #     flush_ifdef!(dst)
        # end

        dst.puts!('} //!namespace vk')

        return self
    end

    def source(api, env, dst, src)
        parse_file!(src, api)

        dst.puts!('namespace vk {')

        dst.puts!("const global_api global_api::g_dummy{")
            dst.scope!(self) do
                dst.puts!("nullptr, // exported_api_")
                pfnfun_dummy(dst, :exported_function, api)
                pfnfun_dummy(dst, :global_level_function, api)
            end
        dst.puts!("};")

        memfun_def(dst, 'attach_return_error', 'global_api', 'const char*', 'const exported_api* api') do
            dst.puts!("exported_api_ = api;")
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

        extenum_def(dst, 'instance_extension', :instance_level_extension)
        extenum_def(dst, 'device_extension', :device_level_extension)

        # list dependencies from device to instance extensions
        dst.puts!("instance_extension_set instance_extensions_require(const device_extension_set& in) {")
        dst.indent!
            exts = @functions[:device_level_extension]
            dst.puts!("instance_extension_set required{ PPE::Meta::ForceInit };")
            exts.reverse_each do |ext| # assume that extensions are sorted by dependency
                requires = ext.expansion
                next if requires.nil?
                requires = requires.dup
                requires.delete_if do |dep| # remove device extensions
                    exts.find {|x| x.name == dep }
                end
                next if requires.empty?
                ifdef(dst, ext.name) do
                    dst.puts!("if (in & #{extflag(ext.name)}) {")
                    dst.indent! do
                        requires.each do |dep|
                            dst.puts!("required += #{extflag(dep)};")
                        end
                    end
                    dst.puts!("}")
                end
            end
            dst.puts!('return instance_extensions_require(required);')
        dst.unindent!
        dst.puts!("}")

        dst.puts!("const instance_api instance_api::g_dummy{")
            dst.scope!(self) do
                dst.puts!("API_version_latest,")
                dst.puts!("nullptr, // global_api_")
                dst.puts!("{ PPE::Meta::ForceInit }, // instance_extensions_")
                pfnfun_dummy(dst, :instance_level_function, api)
                pfnfun_dummy(dst, :instance_level_function_from_extension, api)
            end
        dst.puts!("};")

        memfun_def(dst, 'attach_return_error', 'instance_api', 'const char*', 'const global_api* api, VkInstance vkInstance, api_version version, const instance_extension_set& required, instance_extension_set optional') do
            dst.puts!("version_ = version;")
            dst.puts!("global_api_ = api;")
            dst.puts!("instance_extensions_ = required | optional;")
            each_func(:instance_level_function, api) do |name, pfn|
                vkver = pfn.inner
                dst.puts!("if ((version_ >= #{verflag(vkver)}) &&")
                dst.puts!("    (nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetInstanceProcAddr(vkInstance, \"#{name}\")))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            each_func(:instance_level_function_from_extension, api) do |name, sym|
                lazy_ifdef(dst, sym.expansion) do
                    dst.puts!("if ((instance_extensions_ & #{extflag(sym.expansion)}) &&")
                    dst.puts!("    nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetInstanceProcAddr(vkInstance, \"#{name}\")))) {")
                    dst.scope!(self) do
                        dst.puts!("instance_extensions_ -= #{extflag(sym.expansion)};")
                        dst.puts!("if (required & #{extflag(sym.expansion)}) return \"#{name}\";")
                        dst.puts!("#{name} = g_dummy.#{name};")
                    end
                    dst.puts!("}")
                end
            end
            flush_ifdef!(dst)
            dst.puts!("return nullptr; // no error")
        end

        memfun_def(dst, 'setup_backward_compatibility', 'instance_api', 'void', 'void') do
            dst.puts!('const u32 vkVersion = static_cast<u32>(version_);')
            dst.puts!('UNUSED(vkVersion);')
            by_version = {}
            each_func(:instance_level_backward_compatibility, api) do |name, sym|
                version = [sym.opaque?(:vk_version_major), sym.opaque?(:vk_version_minor)]
                funcs = by_version[version]
                funcs = by_version[version] = [] if funcs.nil?
                funcs << [sym.opaque?(:vk_function_name), sym]
            end
            by_version.each do |(major, minor), funcs|
                ifdef(dst, "VK_VERSION_#{major}_#{minor}") do
                    dst.puts!("if (VK_VERSION_MAJOR(vkVersion) == #{major} || (VK_VERSION_MAJOR(vkVersion) == #{major} && VK_VERSION_MINOR(vkVersion) >= #{minor})) {")
                    dst.indent! do
                        funcs.each do |(name, sym)|
                            extensionName = sym.opaque?(:vk_extension_name)
                            lazy_ifdef(dst, extensionName) do
                                dst.puts!("Assert(#{sym.expansion});")
                                dst.puts!("#{sym.opaque?(:vk_function_name)} = #{sym.expansion};")
                            end
                        end
                        flush_ifdef!(dst)
                    end
                    dst.puts!('}')
                end
            end
        end

        dst.puts!("const device_api device_api::g_dummy{")
            dst.scope!(self) do
                dst.puts!("nullptr, // instance_api_")
                dst.puts!("{ PPE::Meta::ForceInit }, // device_extensions_")
                pfnfun_dummy(dst, :device_level_function, api)
                pfnfun_dummy(dst, :device_level_function_from_extension, api)
            end
        dst.puts!("};")

        memfun_def(dst, 'attach_return_error', 'device_api', 'const char*', 'const instance_fn& fn, VkDevice vkDevice, const device_extension_set& required, device_extension_set optional') do
            dst.puts!("return attach_return_error(fn.instance_api_, vkDevice, required, optional);")
        end
        memfun_def(dst, 'attach_return_error', 'device_api', 'const char*', 'const instance_api* api, VkDevice vkDevice, const device_extension_set& required, device_extension_set optional') do
            dst.puts!("instance_api_ = api;")
            dst.puts!("device_extensions_ = required | optional;")
            each_func(:device_level_function, api) do |name, pfn|
                vkver = pfn.inner
                dst.puts!("if ((instance_api_->version_ >= #{verflag(vkver)}) &&")
                dst.puts!("    (nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetDeviceProcAddr(vkDevice, \"#{name}\")))))")
                dst.puts!("#{dst.tab}return \"#{name}\";")
            end
            each_func(:device_level_function_from_extension, api) do |name, sym|
                lazy_ifdef(dst, sym.expansion) do
                    dst.puts!("if ((device_extensions_ & #{extflag(sym.expansion)}) &&")
                    dst.puts!("    nullptr == (#{name} = reinterpret_cast<PFN_#{name}>(api->vkGetDeviceProcAddr(vkDevice, \"#{name}\")))) {")
                    dst.scope!(self) do
                        dst.puts!("device_extensions_ -= #{extflag(sym.expansion)};")
                        dst.puts!("if (required & #{extflag(sym.expansion)}) return \"#{name}\";")
                        dst.puts!("#{name} = g_dummy.#{name};")
                    end
                    dst.puts!("}")

                end
            end
            flush_ifdef!(dst)
            dst.puts!("return nullptr; // no error")
        end

        memfun_def(dst, 'setup_backward_compatibility', 'device_api', 'void', 'void') do
            dst.puts!('const u32 vkVersion = static_cast<u32>(instance_api_->version_);')
            dst.puts!('UNUSED(vkVersion);')
            by_version = {}
            each_func(:device_level_backward_compatibility, api) do |name, sym|
                version = [sym.opaque?(:vk_version_major), sym.opaque?(:vk_version_minor)]
                funcs = by_version[version]
                funcs = by_version[version] = [] if funcs.nil?
                funcs << [name, sym]
            end
            by_version.each do |(major, minor), funcs|
                ifdef(dst, "VK_VERSION_#{major}_#{minor}") do
                    dst.puts!("if (VK_VERSION_MAJOR(vkVersion) == #{major} || (VK_VERSION_MAJOR(vkVersion) == #{major} && VK_VERSION_MINOR(vkVersion) >= #{minor})) {")
                    dst.indent! do
                        funcs.each do |(name, sym)|
                            extensionName = sym.opaque?(:vk_extension_name)
                            lazy_ifdef(dst, extensionName) do
                                dst.puts!("Assert(#{sym.expansion});")
                                dst.puts!("#{sym.opaque?(:vk_function_name)} = #{sym.expansion};")
                            end
                        end
                        flush_ifdef!(dst)
                    end
                    dst.puts!('}')
                end
            end
        end

        dst.puts!('} //!namespace vk')

        return self
    end

private
    def each_func(type, api, &block)
        @functions[type].each do |sym|
            #pfn = api.symbols[sym.name]
            block.call(sym.name, sym)
        end
    end
    def verflag(vkver)
        flag = vkver.delete_prefix('VK_')
        sep = flag.index('_')
        flag = "#{flag[0..sep]}#{flag[(sep+1)..-1].downcase}"
        return flag
    end
    def extflag(extmacro)
        flag = extmacro.delete_suffix('_EXTENSION_NAME').delete_prefix('VK_')
        sep = flag.index('_')
        flag = "#{flag[0..sep]}#{flag[(sep+1)..-1].downcase}"
        return flag
    end
    def exthref(extmacro)
        flag = extflag(extmacro)
        return flag
        #return "<a href=\"https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_#{flag}.html\">VK_#{flag}</a>"
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
            pfn = api.symbols[fun.name]
            pfn.dependencies.each do |dep|
                if fwd.add?(dep)
                    dep = api.symbols[dep]
                    next if dep.nil?
                    dst.puts!(dep.expansion)
                end
            end
            dst.puts!(pfn)
        end
    end
    def ifdef(dst, macro, &scope)
        return scope.call() if macro.nil?
        lvl = dst.reset_indent()
        dst.puts!("#ifdef #{macro}")
        dst.reset_indent(lvl)
        scope.call()
        dst.reset_indent()
        dst.puts!("#endif")
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
        dst.puts!("};")
    end
    def enum_decl(dst, name, &scope) scope_decl(dst, 'enum', name, &scope) end
    def class_decl(dst, name, &scope) scope_decl(dst, 'class DLL_EXPORT', name, &scope) end
    def struct_decl(dst, name, &scope) scope_decl(dst, 'struct DLL_EXPORT', name, &scope) end
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
                    case type
                    when :global_level_function
                        #dst.puts! '/// Global function'
                    when :instance_level_function
                        #dst.puts! '/// Instance function'
                    when :instance_level_function_from_extension
                        dst.puts! "/// Imported from instance #{exthref(sym.expansion)}"
                    when :device_level_function
                        #dst.puts! '/// Device function'
                    when :device_level_function_from_extension
                        dst.puts! "/// Imported from device #{exthref(sym.expansion)}"
                    end
                    result = pfn.front?
                    result = 'VKAPI_ATTR FORCE_INLINE '+result
                    result = 'NODISCARD '+result if pfn.front? != 'void'
                    memfun_decl(dst, sym.name, result, pfn.inner, true) do |name, result, args|
                        prms = args.split(/\s*,\s*/).collect! do |arg|
                            arg = arg.split(/\s+/).last
                            arg.gsub!(/\[\d+\]/, '') # remove array type expression
                            arg
                        end
                        funcall = "#{level}->#{name}(#{prms.join(', ')});"
                        if pfn.front? == 'void'
                            dst.puts!(funcall)
                        else
                            dst.puts!('return '+funcall)
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
                            dst.puts!('return '+funcall)
                        end
                    end
                else
                    dst.puts!("// #{sym.name}()")
                end
            end
        end
        flush_ifdef!(dst)
    end
    def pfnfun_dummy(dst, type, api)
        @functions[type].each do |sym|
            pfn = api.symbols[sym.name]
            lazy_ifdef(dst, sym.expansion) do
                unless pfn.nil?
                    result = pfn.front?
                    if result == 'void'
                        dst.puts!('[](auto...) -> void {},')
                    elsif result == 'VkResult'
                        err = sym.expansion.nil? ? 'VK_NOT_READY' : 'VK_ERROR_EXTENSION_NOT_PRESENT'
                        dst.puts!('[](auto...) -> VkResult { return %s; },' % err)
                    elsif result == 'VkBool32'
                        dst.puts!('[](auto...) -> VkBool32 { return VK_FALSE; }')
                    elsif result == 'uint64_t' || result == 'uint32_t' || result == 'VkDeviceSize' || result == 'VkDeviceAddress' || result == 'VkFlags'
                        dst.puts!('[](auto...) -> %s { return 0; },' % result)
                    else
                        dst.puts!('[](auto...) -> %s { return VK_NULL_HANDLE; },' % result)
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
        enum_decl(dst, name+' : uint32_t') do
            dst.puts!("#{name}_unknown = 0,")
            exts.each_with_index do |ext, i|
                ifdef(dst, ext.name) do
                    dst.puts!('/// Requires '+ext.expansion.collect{|x| exthref(x) }.join(', ')) unless ext.expansion.nil?
                    dst.puts!("#{extflag(ext.name)},")
                end
            end
            dst.puts!("#{name}_count,")
        end
        dst.puts!("using #{name}_set = PPE::TFixedSizeBitMask<static_cast<uint32_t>(#{name}_count)>;")
        dst.puts!("const char* #{name}_name(#{name} ext);")
        dst.puts!("#{name} #{name}_from(const char* name);")
        dst.puts!("#{name}_set #{name}s_available();")
        dst.puts!("#{name}_set #{name}s_require(const #{name}_set& in);")
        dst.puts!("CONSTEXPR bool operator & (const #{name}_set& bits, #{name} ext) { return bits.Get(static_cast<size_t>(ext)); }")
        dst.puts!("CONSTEXPR #{name}_set& operator +=(#{name}_set& bits, #{name} ext) { bits.SetTrue(static_cast<size_t>(ext)); return bits; }")
        dst.puts!("CONSTEXPR #{name}_set& operator -=(#{name}_set& bits, #{name} ext) { bits.SetFalse(static_cast<size_t>(ext)); return bits; }")
    end
    def extenum_def(dst, name, type)
        exts = @functions[type]

        dst.puts!("const char* #{name}_name(#{name} ext) {")
        dst.indent!
            dst.puts!('switch(ext) {')
            exts.each do |ext|
                ifdef(dst, ext.name) do
                    dst.puts!("case #{extflag(ext.name)}: return #{ext.name};")
                end
            end
            dst.puts!('default: return nullptr;')
            dst.puts!('}')
        dst.unindent!
        dst.puts!('}')

        dst.puts!("#{name} #{name}_from(const char* name) {")
        dst.indent!
            dst.puts!('switch(PPE::hash_str_constexpr(name)) {')
            exts.each do |ext|
                ifdef(dst, ext.name) do
                    dst.puts!("case PPE::hash_str_constexpr(#{ext.name}): return #{extflag(ext.name)};")
                end
            end
            dst.puts!('default: AssertNotReached();')
            dst.puts!('}')
        dst.unindent!
        dst.puts!('}')

        dst.puts!("#{name}_set #{name}s_available() {")
        dst.indent!
            dst.puts!("#{name}_set avail{};")
            exts.each do |ext|
                ifdef(dst, ext.name) do
                    dst.puts!("avail += #{extflag(ext.name)};")
                end
            end
            dst.puts!('return avail;')
        dst.unindent!
        dst.puts!("}")

        dst.puts!("#{name}_set #{name}s_require(const #{name}_set& in) {")
        dst.indent!
            dst.puts!("#{name}_set required{ in };")
            exts.reverse_each do |ext| # assume that extensions are sorted by dependency
                requires = ext.expansion
                next if requires.nil?
                requires = requires.dup
                requires.delete_if do |dep| # remove instance extensions
                    not exts.find {|x| x.name == dep }
                end
                next if requires.empty?
                ifdef(dst, ext.name) do
                    dst.puts!("if (in & #{extflag(ext.name)}) {")
                    dst.indent! do
                        requires.each do |dep|
                            dst.puts!("required += #{extflag(dep)};")
                        end
                    end
                    dst.puts!("}")
                end
            end
            dst.puts!('return required;')
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
    dst.puts!('#include "Container/BitMask.h"')
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
