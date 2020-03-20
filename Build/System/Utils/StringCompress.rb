
require_once '../Common.rb'

module Build

    class StringCompress
        class PrefixNode

        end #~ PrefixNode

        attr_reader :token_to_trie, :str_to_token
        attr_reader :prefix_trie
        def initialize()
            @str_to_token = {}
            @token_to_trie = {}
            @prefix_trie = PrefixNode.root
            @token_count = 0
        end

        def token?(str)
            token = @str_to_token[str]
            token = @str_to_token[str] = next_token!() unless token


            return token
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
            return ('_'+name).to_sym
        end

    end #~ StringCompress


end #~ Build