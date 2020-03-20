# frozen_string_literal: true

require_once '../Common.rb'

require_once 'Log.rb'

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
            @line = @column = 1
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
        def checksum() FileChecksum.new(@filename, Checksum.from_memfile(self)) end
        def write_to_disk()
            Log.info 'writing memfile to disk: "%s"', @filename
            dirname = File.dirname(@filename)
            FileUtils.mkdir_p(dirname) unless Dir.exist?(dirname)
            File.write(@filename, @io.string, mode: 'wb')
        end
        def export_ifn?(external_checksum)
            in_memory_checksum = self.checksum()
            if external_checksum.check?(in_memory_checksum)
                Log.verbose 'skip saving "%s" since it did not change', @filename
                return false
            else
                Log.debug "invalidate '%s' since checksum did not match:\n\t- cfg: %s\n\t- new: %s", @filename, external_checksum.checksum, in_memory_checksum.checksum
                write_to_disk()
                external_checksum.apply!(in_memory_checksum)
                return true
            end
        end
    end #~ MemFile

end #~ Build
