# frozen_string_literal: true

require_once '../Common.rb'

require_once 'MemFile.rb'
require_once 'StringCompress.rb'

module Build

    module BFF

        def self.modified_fileslist_path()
            return File.join($OutputPath, '.modified_files')
        end
        def self.write_modified_fileslist()
            source = MemFile.new(BFF.modified_fileslist_path)

            Build.modified_files?.each do |filename|
                source.puts!(filename)
            end

            source.export_ifn?(Build.modified_fileslist_output)
        end

        def self.escape(str)
            return str.gsub('"', '^"')
        end

        class Source < MemFile
            attr_reader :source, :aliases, :minify
            def initialize(filename, minify: false)
                super(filename, tab: minify ? '' : '  ')
                @source = self
                @aliases = Set.new
                @minify = minify
                @spacer = minify ? '' : ' '
                @tokenizer = minify ? Tokenizer.new : nil
            end

            def once?(key, &block)
                if @aliases.add?(key)
                    instance_exec(&block)
                end
                return self
            end
            def comment!(text, *args)
                unless @minify
                    text = text % args unless args.nil? or args.empty?
                    @source.print!('; ')
                    @source.puts!(text)
                end
                return self
            end
            def value!(value)
                case value
                when TrueClass,FalseClass
                    @source.print!(value ? 'true' : 'false')
                when Integer,Float
                    @source.print!(value)
                when String
                    @source.print!('"')
                    if @minify
                        @source.print!(minify_string(value))
                    else
                        @source.print!(value.gsub('"', '^"'))
                    end
                    @source.print!('"')
                when Symbol
                    @source.print!(value.to_s)
                when Array
                    if value.empty?
                        @source.print!('{}')
                    else
                        @source.print!('{')
                        self.scope!() do
                            value.each_with_index do |x, i|
                                if i > 0
                                    if self.minify
                                        @source.print!(',')
                                    else
                                        @source.puts!(',')
                                    end
                                end
                                value!(x)
                            end
                        end
                        if @minify
                            @source.print!('}')
                        else
                            @source.newline?
                            @source.puts!('}')
                        end
                    end
                when ValueSet
                    self.value!(value.data)
                when Hash
                    if value.empty?
                        @source.puts!('[]')
                    else
                        @source.puts!('[')
                        self.scope!() do
                            value.each do |var, value|
                                set!(var, value)
                            end
                        end
                        if @minify
                            @source.print!(']')
                        else
                            @source.newline?
                            @source.puts!(']')
                        end
                    end
                when Set
                    value!(value.to_a)
                else
                    Log.fatal 'unsupported value type <%s>: %s', value.class, value.to_s
                end
                return self
            end
            def facet!(facet, *attrs)
                return facet!(facet, Facet::ATTRS) if attrs.nil? or attrs.empty?
                attrs.each do |x|
                    if x.is_a?(Hash)
                        x.each do |key, id|
                            value = facet[id]
                            next if value.empty?
                            set!(key, value.data.join(' '))
                        end
                    else
                        value = facet[x]
                        next if value.empty?
                        var = x.to_s[1..-1]
                        var[0] = var[0].upcase
                        set!(var, value.data.join(' '))
                    end
                end
                return self
            end
            def append!(var, value, parent:false, force:false)
                set!(var, value, op:'+', parent:parent, force:force)
            end
            def set!(var, value, op:'=', parent:false, force:false)
                case value
                when Array,Set,String
                    return if force == false && value.empty?
                end
                @source.print!("#{parent ? '^' : '.'}#{var}#{@spacer}#{op}#{@spacer}")
                self.value!(value)
                @source.newline?
                return self
            end
            def func!(name, *args, &block)
                @source.print!(name)
                unless args.empty?
                    @source.print!('(')
                    args.each_with_index do |x, i|
                        @source.print!(' ') if i > 0
                        value!(x)
                    end
                    @source.print!(')')
                end
                @source.puts!("#{@spacer}{")
                @source.scope!(&block)
                if @minify
                    @source.print!('}')
                else
                    @source.puts!('}')
                end
                return self
            end
            def using!(var)
                @source.puts!("Using(.#{var})")
                return self
            end
            def scope!(&block)
                if @minify
                    instance_exec(&block)
                else
                    super(&block)
                end
                return self
            end
            def struct!(name, &block)
                @source.puts!(".#{name}#{@spacer}=#{@spacer}[")
                self.scope!(&block)
                @source.puts!(']')
                return self
            end
            def content_string()
                if @minify
                    return export_minified_strings() + super()
                else
                    return super()
                end
            end
        private
            def minify_string_seq(str, sep: ' ', next_sep: nil)
                return str if str.include?('$') # ignore compression on strings with substitution
                seq = str.split(sep)
                seq.collect! do |sub|
                    if sub.length >= @tokenizer.min_length
                        sub = minify_string_seq(sub, sep: next_sep) if next_sep && sub.include?(next_sep)
                        sub = @tokenizer.token?(sub, refs: 2)
                    end
                    sub
                end
                return seq.join(sep)
                result = String.new
                i = 0; while i < seq.length do
                    result << sep if i != 0
                    consume, ins, sub = 1, nil, seq[i]
                    (seq.length - i - 1).times do |n|
                        exp = (sub+sep)<<seq[i+n+1]
                        nxt = @tokenizer.token?(exp) { ins = sub if ins.nil? }
                        break unless ins.nil?
                        sub = nxt
                        consume += 1
                    end
                    result << (ins.nil? ? sub : ins)
                    i += consume
                end
                return result
            end
            def minify_string(str)
                return minify_string_seq(str, next_sep: '/')
                #return minify_string_seq(str)
            end
            def minify_string_old(str)
                return str.split(/\s+/).collect do |sub|
                    if sub.length > 5 && !sub.include?('$')
                        "$#{@tokenizer.token?(sub)}$"
                    else
                        BFF.escape(sub)
                    end
                end.join(' ')
            end
            def export_minified_strings()
                Assert.check{ @minify }
                minified = StringIO.new
                @tokenizer.each do |str, token|
                    if token.refs > 1
                        minified.puts(".#{token.key}=\"#{BFF.escape(str)}\"")
                    end
                end
                return minified.string
            end
        end #~ Source

    end #~ BFF

end #~ Build