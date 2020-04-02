# frozen_string_literal: true

$Build.namespace(:External) do
    include!('double-conversion/double-conversion.rb')
    include!('farmhash/farmhash.rb')
    include!('iaca/iaca.rb')
    include!('lz4/lz4.rb')
    include!('stb/stb.rb')
    include!('renderdoc/renderdoc.rb')
    include!('vstools/vstools.rb')
    include!('xxHash/xxHash.rb')
end
