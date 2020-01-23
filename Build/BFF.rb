
require 'ostruct'

module BFF

    SKIP_MACROS = true

    def self.dputs(*args)
        if $DEBUG
            puts(*args)
        end
    end

    class Site
        attr_reader :fname, :off, :line, :column
        def initialize(fname, off, line=1, column=1)
            @fname = File.realpath(fname)
            @off = off
            @line = line
            @column = column
        end
        #def to_s() "#{@fname}:#{@line}:#{@column}@#{@off}" end
        def to_s() "#{@fname}:#{@line}:#{@column}" end
        def next!(off, linefeed=false)
            @off = off
            if linefeed
                @line += 1
                @column = 1
            else
                @column += 1
            end
        end
    end #~ Site

    class Token
        attr_reader :type, :str, :ord, :site
        def initialize(type, str, ord, site)
            @type = type
            @str = str
            @ord = ord
            @site = site
        end

        def match?(v)
            result = @type == v || @ord == v ? self : nil
            BFF.dputs("match <#{v}> with <#{details}> = #{result}")
            return result
        end

        def details() return "#{@type}:#{@str}(#{@ord.inspect})" end
        def to_s() return @ord.to_s end

        def self.bool(str, site) Token.new(:bool, str, str =~ /true/i ? true : false, site) end
        def self.keyword(str, site) Token.new(:keyword, str, str.to_sym, site) end
        def self.number(str, site) Token.new(:number, str, str.to_i, site) end
        def self.symbol(str, site) Token.new(:symbol, str, str.to_sym, site) end
        def self.operator(str, site) Token.new(:operator, str, str.to_sym, site) end
        def self.identifier(str, site) Token.new(:identifier, str, str, site) end
        def self.string(str, site) Token.new(:string, str, ('"' << str << '"').undump, site) end

    end #~ Token

    class Lexer
        attr_reader :fname, :site
        def initialize(fname)
            @fname = fname
            @in = File.open(@fname, 'r')
            @site = Site.new(@fname, @in.tell)
            @peeked_ = nil
        end
        def close() @in.close end

        def try?(&block)
            old = @site
            @site = old.clone
            begin
                result = block.call
            #rescue => e
            #	$stderr.puts(e)
            #	result = nil
            end
            if result
                return result
            else
                @site = old
                @in.seek(@site.off)
                return nil
            end
        end

        def eat_spaces?() skip?(/\s/) end
        def skip?(sep)
            while true do
                ch = peekc_()
                break unless !ch.nil? && ch.match(sep)
                getc_()
            end
            return
        end

        def token!()
            eat_spaces?
            str = nil
            while true do
                ch = getc_()
                break if ch.nil?
                if str
                    str << ch
                else
                    str = ch
                end
                break unless ch =~ /\w/
            end
            if str
                str.rstrip!
                return str
            else
                return nil
            end
        end
        def expect!(rule)
            m = token!
            m = m.match(rule) if m
            return m.nil? ? nil : m[0]
        end

        STRING_QUOTES=/["']/

        def bool!() expect!(/^(true|false)$/) end
        def keyword!() expect!(/^(in|using|foreach)$/i) end
        def number!() expect!(/^(-?[0-9]+(\.[0-9]*)?)$/) end
        def symbol!() expect!(/^(\^|\.|,|\{|\}|\[|\]|\(|\)|#)$/) end
        def operator!() expect!(/^(=|\?|\+|-|\+=|\-=)$/) end
        def identifier!() expect!(/^([a-zA-Z]\w*)$/) end
        def string!()
            eat_spaces?
            quote = getc_()
            if quote =~ STRING_QUOTES
                pv = nil
                str = ''
                while true do
                    ch = getc_(true)
                    if ch.nil?
                        break
                    elsif ch == quote && (pv.nil? || pv != '^')
                        return str
                    end
                    pv = ch
                    if ch =~ STRING_QUOTES
                        str << '\\' << ch
                    else
                        str << ch
                    end
                end
            end
            return
        end

        def bool?() try? { bool! } end
        def keyword?() try? { keyword! } end
        def number?() try? { number! } end
        def symbol?() try? { symbol! } end
        def operator?() try? { operator! } end
        def identifier?() try? { identifier! } end
        def string?() try? { string! } end

        def peek!()
            if @peeked_
                result = @peeked_
                @peeked_ = nil
                return result
            end

            eat_spaces?
            lsite = @site.clone

            read = nil
            return Token.keyword(read, lsite) if read = keyword? # keep it first
            return Token.string(read, lsite) if read = string?
            return Token.identifier(read, lsite) if read = identifier?
            return Token.operator(read, lsite) if read = operator?
            return Token.symbol(read, lsite) if read = symbol?
            return Token.number(read, lsite) if read = number?
            return Token.bool(read, lsite) if read = bool?

            if peekc_
                line = @in.gets.chomp
                @in.seek(lsite.off)
                raise "invalid token #{token!.dump} !\n#{line}\n\tat #{lsite}"
            else
                return nil
            end
        end

        def peek?()
            @peeked_ = peek! unless @peeked_
            return @peeked_
        end

    private

        def peekc_()
            ch = @in.getc()
            @in.ungetc(ch)
            return ch
        end

        def getc_internal_()
            ch = @in.getc()
            @site.next!(@in.tell, ch == "\n") if ch
            return ch
        end

        def skip_comments_?(ch)
            comment = false
            if ch == ';' || (SKIP_MACROS && ch == '#')
                comment = true
            elsif ch == '/'
                if @in.getc == '/'
                    @site.next!(@in.tell)
                    comment = true
                else
                    @in.ungetc(ch)
                end
            end
            if comment # skip the line
                while true do
                    ch = getc_internal_()
                    break if ch.nil? || ch == "\n"
                end
                eat_spaces?
                ch = getc_()
            end
            return ch
        end

        def getc_(ignore_comments=false)
            ch = getc_internal_()
            ch = skip_comments_?(ch) unless ignore_comments
            return ch
        end
        def gets_(sep)
            str = ''
            while true do
                ch = getc_()
                break unless ch
                return str if ch.match(sep)
                str << ch
            end
            return
        end

    end #~ Lexer

    class Expr
        @@_depth = 0

        attr_reader :name, :axiom
        def initialize(name, axiom)
            @name = name
            @axiom = axiom
        end

        def tag!(name)
            @name = name
            return self
        end
        def to_s() @name end

        def parse!(lexer)
            BFF.dputs "#{"  " * @@_depth}[#{@@_depth}]#{@name} : #{lexer.site}"
            @@_depth += 1
            result = @axiom.call(lexer)
            @@_depth -= 1
            return result
        end
        def parse?(lexer) lexer.try?{ parse!(lexer) } end

        def except(other)
            Expr.new(:except, lambda do |lexer|
                first = parse!(lexer)
                first ? other.parse!(lexer) : first
            end)
        end
        def then(other)
            Expr.new(:then, lambda do |lexer|
                first = parse!(lexer)
                first ? other.parse!(lexer) : first
            end)
        end
        def optional(other)
            Expr.new(:optional, lambda do |lexer|
                if first = parse!(lexer)
                    return [first, other.parse?(lexer)]
                end
                return nil
            end)
        end
        def select(transform)
            Expr.new(:select, lambda do |lexer|
                tmp = parse!(lexer)
                tmp ? transform.call(tmp) : nil
            end)
        end

        def many() once(0).tag!(:many) end
        def once(at_least=1)
            Expr.new(:once, lambda do |lexer|
                lexer.try? do
                    arr = []
                    while it = parse?(lexer) do
                        arr << it
                    end
                    return arr.length < at_least ? nil : arr
                end
            end)
        end

        def self.ref(get_expr)
            Expr.new(:ref, lambda do |lexer|
                get_expr.call().parse!(lexer)
            end)
        end
        def self.literal(v)
            Expr.new(:literal, lambda do |lexer|
                p = nil
                p = p.match?(v) if p = lexer.peek!
                return p
            end)
        end
        def self.and(*exprs)
            Expr.new(:and, lambda do |lexer|
                results = []
                exprs.each do |expr|
                    a = expr.parse!(lexer)
                    return nil unless a
                    results << a
                end
                return results
            end)
        end
        def self.or(*exprs)
            Expr.new(:or, lambda do |lexer|
                exprs.each do |expr|
                    a = expr.parse?(lexer)
                    return a if a
                end
                return nil
            end)
        end
        def self.list(item, sep)
            Expr.new(:list, lambda do |lexer|
                results = []
                while it = item.parse!(lexer)
                    results << it
                    break unless sep.parse?(lexer)
                end
                return results
            end)
        end

    end #~ Expr

    class Parser
        attr_reader :env

        def initialize()
            @env = {}
        end

        def parse(lexer)
            Parser.scope_begin
            result = P_EXPR.parse!(lexer)
            Parser.scope_end
            return result
        end

    private
        @@_scope = nil
        def self.scope_begin()
            var = OpenStruct.new
            var.inner = []
            var.outer = @@_scope
            return @@_scope = var
        end
        def self.scope_end()
            return @@_scope = @@_scope.outer
        end
        def self.scope_current()
            return @@_scope
        end
        def self.scope_get(name)
            scope = @@_scope
            while scope do
                value = scope[name]
                return value if value
                scope = scope.outer
            end
            return nil
        end
        def self.scope_set(name, op, rhs)
            raise 'nooope' unless @@_scope
            lhs = Parser.scope_get(name)
            if lhs.nil?
                raise op if op != :'='
                value = rhs
            else
                value = lhs.send(op, rhs)
            end
            return (@@_scope[name] = value)
        end

        class Variable
            attr_reader :name
            def initialize(name)
                @name = name.to_sym
            end
            def get() return Parser.scope_get(@name) end
            def set(op, value) Parser.scope_set(@name, op, value) end
        end #~ Variable

        P_ARRAY = Expr.and(
            Expr.literal(:'{'),
            Expr.list(
                Expr.ref(lambda{P_RVALUE}),
                Expr.literal(:',') ),
            Expr.literal(:'}') ).
                select(lambda {|(pre, inf, suf)| inf }).
                tag!(:array)
        P_VARREF = Expr.and(
            Expr.literal(:'.'),
            Expr.literal(:identifier) ).
            select(lambda do |(dot, id)|
                name = id.ord
                Variable.new(name)
            end).
            tag!(:varref)
        P_RVALUE = Expr.or(
            Expr.literal(:bool).select(lambda{|e| e.ord }),
            Expr.literal(:number).select(lambda{|e| e.ord }),
            Expr.literal(:string).select(lambda{|e| e.ord }),
            P_ARRAY, P_VARREF ).tag!(:rvalue)
        P_ASSIGN = Expr.and(
            Expr.literal(:operator),
            P_RVALUE ).tag!(:assign)
        P_PROPERTY = Expr.and(
            P_VARREF,
            P_ASSIGN.once()).
                select(lambda do |(var, assign)|
                    first_op = nil
                    assign.each do |(op, v)|
                        var.set(op.ord, v.kind_of?(Variable) ? v.get() : v)
                        first_op = op.ord if first_op.nil?
                    end
                    return [first_op, var]
                end).
                tag!(:property)
        P_INNER = Expr.ref(lambda{P_LVALUE}).many().
            select(lambda do |lvalues|
                current = Parser.scope_current
                lvalues.each do |lvalue|
                    if lvalue.kind_of?(OpenStruct)
                        lvalue.outer = current
                        current.inner << lvalue
                    else
                        op, var = *lvalue
                        case op
                        when :'='; current[var.name] = var.get()
                        else; current[var.name].send(op, var.get())
                        end
                    end
                end
                return current
            end).
            tag!(:inner)
        P_SCOPE = Expr.and(
            Expr.literal(:'{').select(lambda{|lbracket| Parser.scope_begin }).tag!(:scope_begin),
            P_INNER,
            Expr.literal(:'}').select(lambda{|rbracket| Parser.scope_end }).tag!(:scope_end) ).
                select(lambda do |(open, content, close)|
                    return content
                end).
                tag!(:scope)
        P_LVALUE = Expr.or(
            Expr.ref(lambda{P_PROPERTY}),
            Expr.ref(lambda{P_SCOPE}) ).tag!(:lvalue)
        P_PRAGMA_ONCE =	Expr.literal('once').tag!(:once)
        P_PRAGMA_INCLUDE = Expr.and(
            Expr.literal('include'),
            Expr.literal(:string) ).tag!(:include)
        P_MACRO = Expr.and(
            Expr.literal('#').tag!(:'#'),
            Expr.or(
                Expr.ref(lambda{P_PRAGMA_ONCE}),
                Expr.ref(lambda{P_PRAGMA_INCLUDE})) ).tag!(:macro)
    if false
        P_EXPR = Expr.or(
            Expr.ref(lambda{P_MACRO}),
            Expr.ref(lambda{P_LVALUE}) ).tag!(:expr)
    else
        P_EXPR = P_INNER.tag!(:expr)
    end

    end #~ Parser

    def self.parse(fname)
        puts "Parsing BFF file '#{fname}'..."
        begin
            lexer = BFF::Lexer.new(fname)
            parser = BFF::Parser.new()
            doc = []
            while expr = parser.parse(lexer) do
                doc << expr
                break
            end
            return doc
        ensure
            lexer.close() unless lexer.nil?
        end
        return nil
    end

    def self.dump(ast, depth=0)
        if ast.kind_of?(Array)
            ast.each do |it|
                self.dump(it, depth + 1)
            end
        else
            #puts "#{'  '*depth} #{ast.type}:#{ast.ord} '#{ast.str}'"
            puts "#{'  '*depth} #{ast.inspect}"
        end
    end

end #~ BFF

if __FILE__==$0
    require 'pp'
    ast = BFF.parse(ARGV[0])
    if true
        BFF.dump(ast)
    else
        require 'json'
        def nested_to_hash(obj)
            case obj
            when OpenStruct then hash = {}; obj.each_pair {|k,v| hash[k] = nested_to_hash(v) }; hash
            when Array then obj.map { |v| nested_to_hash(v) }
            else obj
            end
        end
        puts JSON.pretty_generate(nested_to_hash(ast))
    end
end
