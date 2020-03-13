# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Utils/Log.rb'

require 'digest' # sha1
require 'fileutils' # mkdir_p

module Build

    class MemFile
        attr_reader :filename, :io
        attr_reader :line, :column
        def initialize(filename, tab: '  ')
            @filename = filename
            @io = StringIO.new
            @tab = tab
            @indent = ''
            @line = 1
            @column = 1
        end
        def indent!()
            @indent += @tab
            return self
        end
        def unindent!()
            @indent = @indent[0..(-1-@tab.length)]
            return self
        end
        def indent?()
            if @column == 1
                @column += @indent.length
                @io.print(@indent)
            end
            return self
        end
        def print!(fmt, *args)
            str = args.empty? ? fmt.to_s : (fmt.to_s % args)
            indent?; @io.print(str)
            @column += str.length
            return self
        end
        def puts!(fmt, *args)
            str = args.empty? ? fmt.to_s : (fmt.to_s % args)
            indent?; @io.puts(str)
            @column = 1
            @line += 1
            return self
        end
        def newline?()
            if @column > 1
                @io.puts ''
                @column = 1
                @line += 1
            end
            return self
        end
        def scope!(instance=self, &block)
            newline?.indent!
            instance.instance_exec(&block)
            unindent!
        end
        def str() @io.string end
        def digest() MemFile.str_digest(@io.string) end
        def write_to_disk()
            Log.debug 'writing memfile to disk: "%s"', @filename
            FileUtils.mkdir_p(File.dirname(@filename))
            File.write(@filename, @io.string)
        end
        def self.str_digest(str) Digest::SHA1.hexdigest(str) end
        def self.file_digest(fname) Digest::SHA1.hexdigest(File.read(fname)) end
    end #~ MemFile

end #~ Build
