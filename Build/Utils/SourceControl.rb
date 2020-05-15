# frozen_string_literal: true

require_once '../Common.rb'

module Build

    class SourceControl
        def initialize(path, *cmdline)
            @path = File.expand_path(path)
            @modified_files = []
            @branch = @revison = @timestamp = nil
            Log.log('%s: initialize source control \'%s\' in \'%s\'', self.name, cmdline.join(' '), @path)
            @handle = IO.popen(cmdline, 'r+', chdir: @path)
        end
        def file_modified?(fname)
            fname = File.expand_path(fname)
            return self.join().include?(fname)
        end
        def modified_files?()
            return self.join()
        end
        def branch?()
            return @branch
        end
        def revision?()
            return @revision
        end
        def timestamp?()
            return @timestamp
        end
    protected
        def retrieve_status(fd)
            Log.error('%s: retrieve_status(fd) is not implemented', self.name)
        end
        def join()
            if @handle
                Log.verbose('%s: retrieving source control status in \'%s\'', self.name, @path)
                @modified_files = []
                retrieve_status(@handle) do |fname|
                    Log.debug('%s: modified file "%s"', self.name, fname)
                    @modified_files << File.expand_path(fname)
                end
                @handle.close
                if $?.success?
                    Log.log('%s: found %d modified files on current branch is <%s> (revision=%s)', self.name, @modified_files.length, self.branch?, self.revision?)
                else
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
            super(path, *%w{ git status -s })
        end
        def branch?()
            if @branch.nil?
                Dir.chdir(@path) do
                    @branch = %x{ git branch --show-current }
                    @branch.chomp!
                    @branch.strip!
                end
            end
            return @branch
        end
        def revision?()
            if @revision.nil?
                Dir.chdir(@path) do
                    @revision = %x{ git rev-parse HEAD }
                    @revision.chomp!
                    @revision.strip!
                end
            end
            return @revision
        end
        def timestamp?()
            if @timestamp.nil?
                Dir.chdir(@path) do
                    @timestamp = %x{ git log -1 --format=%at }
                    @timestamp.chomp!
                    @timestamp.strip!
                    @timestamp = @timestamp.to_i
                end
            end
            return @timestamp
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

    def branch?()
        $SourceControlProvider.branch?()
    end
    def revision?()
        $SourceControlProvider.revision?()
    end
    def timestamp?()
        $SourceControlProvider.timestamp?()
    end

end #~ Build
