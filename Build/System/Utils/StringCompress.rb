
require_once '../Common.rb'

module Build

    class Tokenizer
        class Token
            attr_reader :key, :value, :var, :refs
            def initialize(key, value, var_fmt, refs=1)
                @key = key
                @value = value
                @var = var_fmt % @key
                @refs = refs
            end
            def inc!(r=1) @refs += r; return self end
        end #~ Token
        attr_reader :var_fmt, :min_length, :str_to_token
        def initialize(min_length: 5, var_fmt: '$%s$')
            @str_to_token = {}
            @min_length = min_length
            @var_fmt = var_fmt
            @token_count = 0
        end
        def each(&block) @str_to_token.each(&block) end
        def token?(str, refs: 1)
            #return str if str.length < @min_length
            str.freeze
            token = @str_to_token[str]
            if token
                token.inc!(refs)
            else
                token = Token.new(next_token!(), str, @var_fmt, refs)
                @str_to_token[str] = token
                yield token if block_given?
            end
            return token.var
        end
    private
        ALPHABET = ('a'..'z').to_a+('A'..'Z').to_a+('0'..'9').to_a<<'_'
        ALPHABET_DIM = ALPHABET.length
        def next_token!()
            token_id = @token_count
            @token_count += 1
            name = '_'
            loop do
                name += ALPHABET[token_id % ALPHABET_DIM]
                token_id /= ALPHABET_DIM
                break if token_id == 0
            end
            return name.to_sym
        end
    end #~ Tokenizer

    class StringCompress

        class PrefixNode
            attr_reader :prefix
            attr_reader :parent, :children
            def initialize(prefix, parent: nil)
                @prefix = prefix
                @parent = parent
                @children = []
            end
            def common_chars?(text)
                n = @prefix.length
                n = text.length if @prefix.length > text.length
                n.times do |i|
                    return i if text[i] != @prefix[i]
                end
                return n
            end
            def insert(text)
                Assert.check{ text.start_with?(@prefix) }
                suff = text[@prefix.length..-1]
                return self if suff.empty?
                @children.each_with_index do |child, i|
                    n = child.common_chars?(suff)
                    case n
                    when 0
                        next
                    when suff.length
                        return child
                    when child.prefix.length
                        return child.insert(suff)
                    else
                        new_node = child.split_after(n)
                        @children[i] = new_node
                        return new_node.insert(suff)
                    end
                end
            end
            def split_after(n)
                pref = @prefix[0..(n-1)]
                @prefix = @prefix[n..-1]
                split = PrefixNode.new(pref, parent: @parent)
                split.children << self
                @parent = split
                return split
            end
        end #~ PrefixNode

        attr_reader :token_to_trie, :tokens
        attr_reader :prefix_trie

        def initialize()
            @tokens = Tokenizer.new
            @prefix_trie = PrefixNode.new('')
            @token_to_trie = {}
        end

        def compress?(str)
            return @tokens.token?(str) do |token|
                @token_to_trie[token] = @prefix_trie.insert(str)
            end
        end

    end #~ StringCompress


end #~ Build