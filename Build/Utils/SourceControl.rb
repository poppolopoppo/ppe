# frozen_string_literal: true

require_once '../Common.rb'

module Build

    class SourceControl
        def initialize(path, *cmdline)
            @path = File.expand_path(path)
            @modified_files = []
            Log.debug('%s: initialize source control \'%s\' in \'%s\'', self.name, cmdline.join(' '), @path)
            @handle = IO.popen(cmdline, 'r+', chdir: @path)
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
            Log.error('%s: retrieve_status(fd) is not implemented', self.name)
        end
        def join()
            if @handle
                Log.log('%s: retrieving source control status in \'%s\'', self.name, @path)
                @modified_files = []
                retrieve_status(@handle) do |fname|
                    Log.debug('%s: modified file "%s"', self.name, fname)
                    @modified_files << File.expand_path(fname)
                end
                @handle.close
                unless $?.success?
                    Log.error('%s: failed to retrieve source control status (exitcode=%s)', self.name, $?)
                end
                @handle = nil
            end
            return @modified_files
        end
    end #~ SourceControl

    class DummySourceControl < SourceControl
        def name() :Dummy end
        def initialize() end
        def file_modified?(fname) false end
        def modified_files() [] end
    end #~ DummySourceControl

    class GitSourceControl < SourceControl
        def name() :Git end
        def initialize(path)
            super(path, 'git', 'status', '-s')
        end
    protected
        def retrieve_status(fd)
            fd.readlines.each do |ln|
                path = File.join(@path, ln.chomp[3..-1])
                if Dir.exist?(path)
                    Dir.chdir(path) do
                        Dir.glob("**/*") do |p|
                            yield File.expand_path(p)
                        end
                    end
                elsif File.exist?(path)
                    yield path
                end
            end
        end
    end #~ GitSourceControl

    $SourceControlProvider = DummySourceControl.new

    def self.init_source_control(provider: :git, path: $SourcePath)
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
