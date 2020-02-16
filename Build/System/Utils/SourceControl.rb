
require './Common.rb'

module Build

    class SourceControl
        def initialize(path, *cmdline)
            @path = path
            @modified_files = []
            @handle = IO.popen(cmdline, chdir: @path)
        end
        def file_modified?(fname)
            fname = File.expand_path(fname)
            return self.join().include?(fname)
        end
        def modified_files?()
            return self.join()
        end
    protected
        def retrieve_status(fd)
            Log.error('%s: retrieve_status(fd) is not implemented', self.class)
        end
    private
        def join()
            if @handle
                Log.debug('%s: retrieving source control status', self.clasd)
                @modified_files = []
                retrieve_status(@handle) do |fname|
                    Log.verbose('%s: modified file "%s"', self.class, fname)
                    @modified_files << File.expand_path(fname)
                end
                @handle.close
                @handle = nil
            end
            return @modified_files
        end
    end #~ SourceControl

    class DummySourceControl < SourceControl
        def initialize() end
        def file_modified?(fname) false end
        def modified_files() [] end
    end #~ DummySourceControl

    class GitSourceControl < SourceControl
        def initialize(path)
            super(path, [ 'git', 'status', '--porcelain' ])
        end
    protected
        def retrieve_status(fd)
            fd.readlines do |ln|
                fname = ln.chomp[3..-1]
                yield fname if File.exist?(fname)
            end
        end
    end #~ GitSourceControl

    $SourceControlProvider = DummySourceControl.new

    def self.init_source_control(provider: :git, path: File.dirname($0))
        unless Dir.exist?(path)
            Log.error "can't initialize source control, invalida path: '%s'", path
            return
        end
        case provider
        when :git
            $SourceControlProvider = GitSourceControl.new(path)
        else
            Log.fatal 'unsupported source control provider: %s', provider
        end
        return $SourceControlProvider
    end
    def file_modified?(fname)
        $SourceControlProvider.file_modified?(fname)
    end
    def modified_files?()
        $SourceControlProvider.modified_files?()
    end

end #~ Build
