
require_once '../Common.rb'

require_once 'Log.rb'
require_once 'MemFile.rb'
require_once 'Options.rb'
require_once 'Prerequisite.rb'

require 'digest' # sha1

module Build

    class Checksum
        attr_reader :digest, :mtime, :size
        def initialize(digest, mtime, size)
            @digest = digest
            @mtime = mtime
            @size = size
        end
        def reset!()
            @mtime = Time.now
            @size = -1
            return self
        end
        def match?(other)
            return (@digest == other.digest and
                    @mtime == other.mtime and
                    @size == other.size )
        end
        def check?(filename, digest)
            return (match?(digest) and filesystem_check?(filename))
        end
        def filesystem_check?(filename)
            st = File.stat(filename)
            if st.mtime != @mtime
                Log.warning '%s: invalidate due to mtime (%s <=> %s)', filename, @mtime, st.mtime
                return false
            end
            if st.size != @size
                Log.warning '%s: invalidate due to size (%d <=> %d)', filename, @size, st.size
                return false
            end
            return true
        end
        def to_s() "#{@digest}-#{@mtime.to_i}-#{@size}" end
        def self.from_file(filename)
            if File.exist?(filename)
                st = File.stat(filename)
                return Checksum.from_str(File.read(filename), st.mtime, st.size)
            else
                return Checksum::INVALID
            end
        end
        def self.from_str(str, mtime=Time.now, size=str.bytesize)
            return Checksum.new(Digest::SHA1.hexdigest(str), mtime, size)
        end
        INVALID = Checksum.new(nil, nil, nil)
    end #~ Checksum

    class FileChecksum
        attr_reader :filename, :checksum
        def initialize(filename, checksum)
            @filename = filename
            @checksum = checksum
        end
        def digest() @checksum.digest end
        def mtime() @checksum.mtime end
        def size() @checksum.size end
        def set!(checksum)
            @checksum = checksum
            return self
        end
        def force!()
            FileUtils.touch(@filename, mtime: @checksum.mtime)
            if $DEBUG
                from_fs = Checksum.from_file(@filename)
                if not @checksum.check?(@filename, from_fs)
                    Log.fatal("checksum failed for \"%s\":\n"+
                        "\t- digest = <%s> <=> <%s>\n"+
                        "\t- mtime = %s <=> %s\n"+
                        "\t- size = %d <=> %d\n",
                        @filename,
                        @checksum.digest, from_fs.digest,
                        @checksum.mtime, from_fs.mtime,
                        @checksum.size, from_fs.size )
                end
            end
            return self
        end
        def apply!(other)
            Assert.check { @filename == other.filename }
            set!(other.checksum)
            force!()
        end
        def check?(other)
            return ((@filename == other.filename) and (@checksum.digest == other.checksum.digest))
        end
        def to_s() "#{@filename}-#{@checksum}" end
        def hash() self.to_s.hash end
        def self.from(filename)
            return FileChecksum.new(filename, Checksum.from_file(filename))
        end
        def validate_checksum!()
            unless @checksum.filesystem_check?(@filename)
                @checksum.reset!
            end
            return true
        end
    end #~ FileChecksum

    def make_persistent_file(name, &filename)
        Build.make_prerequisite(name, namespace: 'Checksum') do
            FileChecksum.from(filename.call())
        end.validate!{|x| x.validate_checksum! }
    end

end #~ Build