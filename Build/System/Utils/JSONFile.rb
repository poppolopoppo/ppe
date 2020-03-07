require_once '../Common.rb'
require_once '../Utils/MemFile.rb'

module Build

    class JSONFile < MemFile
        def initialize(filename)
            super(filename, tab: "\t")
        end
        def set!(key, value)
            print!(key.to_s.inspect)
            print!(': ')
            value!(value)
            return self
        end
        def value!(value)
            case value
            when NilClass
                print!('null')
            when TrueClass
                print!('true')
            when FalseClass
                print!('false')
            when Integer,Float
                print!(value.to_s)
            when String
                print!(value.inspect)
            when Set
                value!(value.to_a)
            when Array
                if value.empty?
                    puts!('[]')
                else
                    puts!('[')
                    scope!() do
                        value.each_with_index do |x,i|
                            puts!(',') if i > 0
                            value!(x)
                        end
                    end
                    puts!(']')
                end
            when Hash
                if value.empty?
                    puts!('{}')
                else
                    puts!('{')
                    scope!() do
                        first = true
                        value.each do |k,v|
                            puts!(',') unless first
                            set!(k, v)
                            first = false
                        end
                    end
                    puts!('}')
                end
            else
                Log.fatal 'unsupported value type: %s', value.inspect
            end
            return self
        end
        def self.serialize(filename, data)
            json = JSONFile.new(filename)
            json.value!(data)
            json.write_to_disk()
            Log.verbose 'wrote JSON file to "%s"', filename
        end
    end #~ JSONFile

end #~ Build
