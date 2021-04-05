# frozen_string_literal: true

require_once '../Common.rb'

require_once 'Log.rb'

require 'fileutils' # mkdir_p

module Build

    class MemFile
        attr_reader :filename, :io, :tab
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
        def heredoc!(str)
            @io.print(str)
            str.each_char do |ch|
                if ch == "\n"
                    @column = 1
                    @line += 1
                else
                    @column += 1
                end
            end
            return self
        end
        def scope!(instance=self, &block)
            newline?.indent!
            instance.instance_exec(&block)
            unindent!
        end
        def str() @io.string end
        def write_to_disk()
            content_write(content_string())
        end
        def export_ifn?(external_checksum)
            data = content_string()
            in_memory_checksum = FileChecksum.new(@filename, Checksum.from_str(data))
            if external_checksum.check?(in_memory_checksum)
                Log.debug 'skip saving "%s" since it did not change', @filename
                return false
            else
                Log.debug "invalidate '%s' since checksum did not match:\n\t- cfg: %s\n\t- new: %s", @filename, external_checksum.checksum, in_memory_checksum.checksum
                content_write(data)
                external_checksum.apply!(in_memory_checksum)
                return true
            end
        end
    protected
        def content_string()
            return @io.string
        end
        def content_write(data)
            Log.log 'writing memfile to disk: "%s"', @filename
            dirname = File.dirname(@filename)
            FileUtils.mkdir_p(dirname, :verbose => Log.verbose?) unless Dir.exist?(dirname)
            File.write(@filename, data, mode: 'wb')
        end
    end #~ MemFile

end #~ Build
