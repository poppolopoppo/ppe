
require_once '../Common.rb'

module Build

    class UUID
        ALPHABET = ('a'..'z').to_a+('A'..'Z').to_a+('0'..'9').to_a<<'_'
        def initialize(alphabet: ALPHABET, prefix: '_')
            @alphabet = alphabet
            @count = 0
            @prefix = '_'
        end
        def next!
            uuid = @count
            @count += 1
            dim = @alphabet.length
            name = String.new<<@prefix
            loop do
                name << ALPHABET[uuid % dim]
                uuid /= dim
                break if uuid == 0
            end
            return name.to_sym
        end
    end #~UUID

    class Tokenizer

        class Token
            attr_reader :key, :value, :refs
            def initialize(key, value, refs=0)
                @key, @value, @refs = key, value, refs
                @key.freeze
                @value.freeze
            end
            def inc!(add_ref=1) @refs += add_ref end
            def reset!(new_refs=0) @refs = new_refs end
            def override!(str) @value = str end
            def to_s() return @var end
            def ==(other) @key == other.key end
            def <=>(other) @key <=> other.key end
            def hash() @key.hash end
        end #~ Token

        attr_reader :uuid, :tokens

        def initialize(uuid)
            @uuid = uuid
            @tokens = Hash.new
        end
        def get?(str) return @tokens[str] end
        def var!(str)
            token = @tokens[str]
            return token ? token.var : str
        end
        def token?(str, add_ref: 1, allow_create: true)
            token = @tokens[str]
            if token
                yield token, true if block_given?
            elsif allow_create
                str.freeze
                token = Token.new(@uuid.next!, str)
                #Log.debug 'LZW: new token .%s = "%s"', token.key, token.value
                @tokens[str] = token
                yield token, false if block_given?
            else
                yield nil, false if block_given?
                return nil
            end
            token.inc!(add_ref)
            return token
        end

        def tokenize(str, sep, add_ref: 1, allow_create: true, min_length: 6)
            seq = str.split(sep)
            mrg = []
            seq.each do |sub|
                Assert.check{!sub.nil?}
                if mrg.empty? || mrg.last.length >= min_length
                    mrg << sub
                else
                    (mrg[-1] += sep)<<sub
                end
            end
            mrg.collect! do |x|
                x = yield x if block_given?
                if x.length < min_length
                    x
                else
                    token?(x, add_ref: add_ref, allow_create: allow_create).key
                end
            end
            return mrg
        end

    end #~ Tokenizer

    class StringSlicer
        class Slice
            attr_reader :uid, :seq, :refs
            def initialize(uid, seq)
                @uid = uid
                @seq = seq
                @uid.freeze
                @seq.freeze
                @refs = 1
            end
            def inc!() @refs+=1; return self end
        end #~Slice
        attr_reader :slice_size
        def initialize(slice_size)
            @slice_size = slice_size
            @slices = Hash.new
        end
        def insert!(seq)
            loop do
                sub = seq[0..(@slice_size-1)]
                seq = seq[@slice_size..-1]

                break if sub.length < @slice_size

                slc = @slices[sub]
                if slc.nil?
                    slc = Slice.new(@slices.length, sub)
                    @slices[sub] = slc
                end

                slc.inc!
            end
        end
        def finalize!()
            @slices.delete_if do |seq, slc|
                slc.refs < 2
            end
            return self
        end
        def subst?(seq, &uid_to_token)
            key = []
            loop do
                break if seq.nil? || seq.empty?

                sub = seq[0..(@slice_size-1)]
                seq = seq[@slice_size..-1]

                slc = @slices[sub]
                if slc.nil?
                    key.concat(sub)
                else
                    key << uid_to_token.call(slc)
                end
            end
            return key
        end
    end #~StringSlicer

    class StringCompress
        attr_reader :uuid, :tokenizer, :suffix_array
        attr_reader :min_length
        attr_reader :inputs

        def initialize(min_length: 6)
            @uuid = UUID.new
            @tokenizer = Tokenizer.new(uuid)
            @suffix_array = StringSlicer.new(5)
            @min_length = min_length
            @inputs = Hash.new
        end

        def first_pass(str)
            str = str.strip
            return str if str.length < @min_length

            seq = tokenize(str)
            return seq.first if seq.length == 1

            uid = @inputs[seq]
            if uid.nil?
                uid = @inputs[seq] = @uuid.next!
                @suffix_array.insert!(seq)
            end
            return uid
        end

        def second_pass(&block)
            @suffix_array.finalize!
            @tokenizer.tokens.each do |str, token|
                yield(token.key, [token.value]) if token.refs > 1
            end
            index_to_uuid = Hash.new
            @inputs.each do |seq, iuid|
                seq = @suffix_array.subst?(seq) do |slice|
                    uuid = index_to_uuid[slice.uid]
                    if uuid.nil?
                        uuid = index_to_uuid[slice.uid] = @uuid.next!
                        yield(uuid, slice.seq)
                    end
                    uuid
                end
                yield(iuid, seq)
            end
        end

    private

        def lempel_ziv_welch(seq, add_ref: 1, allow_create: true)
            w = ''
            result = String.new

            seq.each_with_index do |c, i|
                if w.empty?
                    w = c.to_s
                    next
                end
                p = w+' '+c.to_s
                @tokenizer.token?(p, add_ref: add_ref, allow_create: allow_create) do |token, exist|
                    if exist
                        w = p
                    else
                        result << ' ' unless result.empty?
                        result << @tokenizer.var!(w)
                        w = c.to_s
                    end
                end
            end

            unless w.empty?
                result << ' ' unless result.empty?
                result << @tokenizer.var!(w)
            end

            return result
        end

        def tokenize(str)
            Assert.check{ str.length >= @min_length }
            str = str.gsub(/\s+/, ' ')
            return @tokenizer.tokenize(str, ' ', add_ref: 2, min_length: @min_length) do |sub|
                if sub.start_with?('/')
                    sub
                else
                    sub = @tokenizer.tokenize(sub, '/', add_ref: 2, min_length: @min_length)
                    sub.collect! do |x|
                        case x
                        when String
                            x
                        when Symbol
                            "$#{x}$"
                        else
                            Assert.unexpected(x)
                        end
                    end
                    sub.join('/')
                end
            end
        end

    end #~ StringCompress

end #~ Build