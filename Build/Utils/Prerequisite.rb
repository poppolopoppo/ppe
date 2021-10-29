# frozen_string_literal: true

require_once '../Utils/Options.rb'
require_once '../Utils/Log.rb'

module Build

    class Prerequisite < OptionVariable
        def initialize(name, default, getter)
            super(name, default)
            @getter = getter
        end
        def available?()
            evaluate!.value
        end
        def evaluate!()
            set!(self.instance_exec(&@getter)) if @value.nil?
            return self
        end
    public
        def need_cmdline!(*args)
            args.collect!{|x| x =~ /\s/ ? "'#{x}''" : x }
            result = %x{ #{args.join(' ')} }
            if $?.success?
                result = result.lines.map(&:chomp)
                Log.debug("command succeed: %s -> '%s'", args, result)
                return result
            else
                Log.warning("command failed: %s (exit code = %s)", args, $?)
                return false
            end
        end
        def need_file!(path)
            if File.exist?(path)
                Log.debug("found file '%s'", path)
                return path
            else
                Log.warning("file does not exists '%s'", path)
                return false
            end
        end
        def need_fileset!(*filenames)
            filenames.each do |path|
                return false unless need_file!(path)
            end
            return filenames.length == 1 ? filenames.first : filenames
        end
        def need_folder!(path)
            if DirCache.exists?(path)
                Log.debug("found directory '%s'", path)
                return path
            else
                Log.warnig("directory does not exists '%s'", path)
                return false
            end
        end
        def need_glob!(pattern, basepath: nil)
            if basepath.nil?
                basepath = File.dirname(pattern)
                pattern = File.basename(pattern)
            end
            entries = Dir.glob(pattern, File::FNM_DOTMATCH, base: basepath)
            unless entries.empty?
                Log.debug("glob %d entries with pattern '%s'", entries.length, pattern)
                entries.collect!{ |x| File.join(basepath, x) }
                return entries
            else
                Log.warning("didn't glob any entry with pattern '%s'", pattern)
                return []
            end
        end
        def need_envvar!(id)
            if ENV.include?(id.to_s)
                result = ENV[id.to_s]
                Log.debug("found environment variable <%s> = '%s'", id, result)
                return result
            else
                Log.warning("could not find environment variable <%s>", id)
                return false
            end
        end
        def need_wget!(url)
            require 'open-uri'
            Log.info("downloading '%s'...", url)
            if download = URI.open(url)
                local = File.join($TemporaryPath, url.to_s.split('/')[-1])
                FileUtils.mkdir_p(File.dirname(local))
                if IO.copy_stream(download, local)
                    Log.debug("downloaded '%s' to '%s'", url, local)
                    return local
                end
            end
            Log.warning("failed to download '%s'", url)
        end
        def need_zip!(archive, destination, *globs)
            require 'rubygems'
            require 'zip'
            if File.exists?(archive)
                FileUtils.rm_rf(destination) if Dir.exists?(destination)
                Log.verbose("extracting '%s' to '%s'...", archive, destination)
                listing = []
                Zip::File.open(archive) do |zfile|
                    files_to_extract = []
                    if globs.empty?
                        files_to_extract.concat(zfile.glob('*'))
                    else
                        globs.each{|x| files_to_extract.concat(zfile.glob(x)) }
                    end
                    files_to_extract.each do |src|
                        dst = File.join(destination, src.name)
                        dir = File.dirname(dst)
                        FileUtils.mkdir_p(dir) unless Dir.exists?(dir)
                        Log.debug("extract zipped file to '%s'", dst)
                        zfile.extract(src, dst)
                        listing << dst if File.exists?(dst)
                    end
                end
                Log.debug("downloaded %d files from '%s' to '%s'", listing.length, archive, destination)
                return listing
            else
                Log.warning("archive '%s' does not exist", archive)
                return false
            end
        end
    public
        def validate_DirExist!()
            self.validate!(&lambda do |x|
                case x
                when String
                    return DirCache.exist?(x)
                when Array,Set
                    x.each do |y|
                        return false unless validator[y]
                    end
                    return true
                when nil,false
                    return false
                else
                    Log.fatal 'unsupported value <%s>: %s', x.class, x.inspect
                end
            end)
        end
        def validate_FileExist!()
            self.validate!(&lambda do |x|
                case x
                when String
                    return File.exist?(x)
                when Array,Set
                    x.each do |y|
                        return false unless validator[y]
                    end
                    return true
                when nil,false
                    return false
                else
                    Log.fatal 'unsupported value <%s>: %s', x.class, x.inspect
                end
            end)
        end
    end #~ Prerequisite

    def make_prerequisite(name, namespace: 'Prerequisite', default: nil, host: self.class, &getter)
        prereq = Prerequisite.new("#{namespace}.#{name}", default, getter)
        Log.debug("define prerequisite method <%s::%s>", host, name)
        host.define_method(name) do
            return prereq.available?
        end
        return Build.make_persistent_opt(prereq)
    end

    def self.restore_prerequisite(name, value)
        Build.restore_persistent_opt(name, value)
    end

    def import_cmdline(name, *cmdline)
        make_prerequisite(name, namespace: 'Import') do
            need_cmdline!(*cmdline)
        end
    end
    def import_envvar(name)
        make_prerequisite(name, namespace: 'Import') do
            need_envvar!(name)
        end
    end
    def import_fileset(name, *filenames)
        make_prerequisite(name, namespace: 'Import') do
            need_fileset!(*filenames)
        end
    end
    def import_folder(name, path)
        make_prerequisite(name, namespace: 'Import') do
            need_folder!(path)
        end
    end
    def import_glob(name, *patterns)
        make_prerequisite(name, namespace: 'Import') do
            results = []
            patterns.each do |pattern|
                results.concat(need_glob!(pattern))
            end
            results.sort!{|a, b| b <=> a } # descending
            results.empty? ? false : results
        end
    end
    def import_remote_archive(name, destination, url, *files)
        make_prerequisite(name, namespace: 'Import') do
            if archive = need_wget!(url)
                need_zip!(archive, destination, *files)
            end
        end.validate_FileExist!
    end

end #~ Build
