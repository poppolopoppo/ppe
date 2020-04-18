# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

require 'fileutils'
require 'find'
require 'pathname'
require 'set'

module Build

    make_command(:cpphint, 'Generate cpp.hint for Intellisense') do |&namespace|
        environments = Build.fetch_environments
        targets = namespace[].select(*Build::Args)

        resolver = CppHint::MacroResolver.new
        parser = CppHint::DefineParser.new(resolver)

        environments.each do |env|
            targets.each do |target|
                next unless target.executable? or target.library?
                expanded = env.expand(target)
                expanded.defines.each do |it|
                    # assumes every we don't defined macro functions, but only macros aliases
                    data = it.split('=')
                    resolver.add_macro!(data.first, nil, data[1])
                end
                expanded.includes.each do |it|
                    parser.parse_file(env.source_path(it))
                end
                target.source_files.each do |it|
                    parser.parse_file(env.source_path(it))
                end
                parser.parse_dir(env.source_path(target.source_path))
            end
        end

        cpphint = File.join($SourcePath, 'cpp.hint')
        Log.info('Hint: generating CPP hint file with %d macros (%d disruptives)', resolver.macros.length, resolver.disruptives.length)

        File.open(cpphint, 'w') do |fd|
            fd.puts <<~HEADER
// Hint files help the Visual Studio IDE interpret Visual C++ identifiers
// such as names of functions and macros.
// For more information see https://go.microsoft.com/fwlink/?linkid=865984
            HEADER
            resolver.disruptives.each do |macro|
                decl = "#define %s%s" % [ macro.id, macro.params ]
                if macro.unbalanced
                    decl << ' '
                    decl << macro.value
                end
                fd.puts(decl)
            end
        end

    end #~genhint

    module CppHint

        class Macro
            attr_reader :id, :params, :value
            attr_reader :keywords, :unbalanced
            def initialize(id, params, value, keywords=false, unbalanced=false)
                @id = id
                @params = params
                @value = value
                @keywords = keywords
                @unbalanced = unbalanced
            end
            def disruptive?() @keywords || @unbalanced end
            def self.sanitize(id, params, value)
                keywords = false
                unbalanced = false
                if value
                    value.gsub!(/\/\/.*$/, '') # // strip comments
                    value.gsub!(/\/\*.*?\*\/$/m, '') # /* strip comments */
                    keywords = Macro.keywords?(value)
                    unbalanced = Macro.unbalanced_braces?(value)
                end
                return Macro.new(id, params, value, keywords, unbalanced)
            end
        private
            # From M$: https://docs.microsoft.com/en-us/cpp/cpp/keywords-cpp?view=vs-2019
            RE_KEYWORDS=Regexp.new(%w{
                __abstract	__alignof __asm	__assume
                __based	__box	__cdecl	__declspec
                __delegate	__event	__except	__fastcall
                __finally	__forceinline	__gc	__hook
                __identifier	__if_exists	__if_not_exists	__inline
                __int16	__int32	__int64	__int8
                __interface	__leave	__m128	__m128d
                __m128i	__m64	__multiple_inheritance	__nogc
                __noop	__pin	__property	__ptr32
                __ptr64	__raise	__restrict	__sealed
                __single_inheritance	__sptr	__stdcall	__super
                __thiscall	__try_cast	__unaligned	__unhook
                __uptr	__uuidof	__value	__vectorcall
                __virtual_inheritance	__w64	__wchar_t
                alignas	auto	bool
                break	case	catch	char
                char16_t	char32_t	class	const
                const_cast	constexpr	continue	decltype
                default	delete	deprecated
                dllexport	dllimport	do	double
                dynamic_cast	else	enum	enum class
                enum struct		explicit	extern
                false	finally	float	for
                for each in	friend	friend_as
                generic	goto	if	initonly
                inline	int interface
                long	mutable
                naked	namespace new
                noexcept	noinline	noreturn	nothrow
                novtable	nullptr	operator	private
                property	protected	public
                register	reinterpret_cast
                return	safecast	selectany
                short	signed	sizeof	static
                static_assert	static_cast	struct	switch
                template	this	thread	throw
                true	try	typedef	typeid
                typeid	typename	union	unsigned
                using declaration	using	uuid
                virtual	void	volatile
                while
                }.sort.uniq.join('|'))
            def self.keywords?(value)
                return !!(value =~ RE_KEYWORDS)
            end
            def self.unbalanced_braces?(value)
                balance = 0
                value.each_char do |c|
                    case c
                    when '{'
                        balance += 1
                    when '}'
                        balance -= 1
                    end
                end
                return (0 != balance)
            end
        end #~Macro

        class MacroResolver
            attr_reader :macros, :disruptives
            def initialize()
                @macros = Hash.new
                @disruptives = Array.new
            end
            def add_macro!(id, params=nil, value=nil)
                return if @macros.include?(id)
                m = Macro.sanitize(id, params, value)
                if m.disruptive?
                    Log.warning('Hint: disruptive macro <%s%s> = "%s" (keywords=%s unbalanced=%s)', m.id, m.params, m.value, m.keywords, m.unbalanced)
                    @disruptives << m
                else
                    Log.verbose('Hint: new macro <%s%s> = "%s"', m.id, m.params, m.value)
                end
                @macros[id] = m
                return self
            end
            def each(&block) @macros.each(&block) end
        end #~ MacroResolver

        class DefineParser
            RE_DEFINE=/#\s*define\s+(\w+)(\(.*?\))?\s*(.*?)(?<!\\)(?:\\\\)*$/im
            attr_reader :resolver
            def initialize(resolver)
                @resolver = resolver
                @visiteds = Set.new
            end
            def parse_dir(path)
                return unless @visiteds.add?(path)
                Find.find(path) do |entry|
                    parse_file(entry) if entry =~ /\.(h|cpp|cc|c)$/i
                end
            end
            def parse_file(filename)
                return unless @visiteds.add?(filename)
                File.read(filename).scan(RE_DEFINE) do |(id, params, value)|
                    @resolver.add_macro!(id, params, value)
                end
            end
        end #~ DefineParser

    end #~ CppHint

end #~ Build
