# frozen_string_literal: true

require_once '../Common.rb'

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
            Log.debug("command result: %s", result)
            if $?.success?
                Log.debug("command succeed: %s -> '%s'", args, result)
                return result.lines.map(&:chomp)
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
                Log.verbose("file does not exists '%s'", path)
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
            if Dir.exists?(path)
                Log.debug("found directory '%s'", path)
                return path
            else
                Log.verbose("directory does not exists '%s'", path)
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
                Log.verbose("didn't glob any entry with pattern '%s'", pattern)
                return []
            end
        end
        def need_envvar!(id)
            if ENV.include?(id.to_s)
                result = ENV[id.to_s]
                Log.debug("found environment variable <%s> = '%s'", id, result)
                return result
            else
                Log.verbose("could not find environment variable <%s>", id)
                return false
            end
        end
    public
        def validate_FileExist!()
            validator = lambda do |x|
                case x
                when String
                    return File.exist?(x)
                when Array,Set
                    x.each do |y|
                        return false unless validator[y]
                    end
                    return true
                else
                    Log.fatal 'unsupported value <%s>: %s', x.class, x.inspect
                end
            end
            self.validate!(&validator)
        end
    end #~ Prerequisite

    def make_prerequisite(name, namespace: 'Prerequisite', default: nil, &getter)
        prereq = Prerequisite.new("#{namespace}.#{name}", default, getter)
        self.class.define_method(name) do
            return prereq.available?
        end
        return Build.make_persistent_opt(prereq)
    end
    def self.restore_prerequisite(name, value)
        Build.restore_persistent_opt(name, value)
    end

    def import_cmdline(name, *cmdline)
        make_prerequisite(name) do
            need_cmdline!(*cmdline)
        end
    end
    def import_envvar(name)
        make_prerequisite(name) do
            need_envvar!(name)
        end
    end
    def import_fileset(name, *filenames)
        make_prerequisite(name) do
            need_fileset!(*filenames)
        end
    end
    def import_folder(name, path)
        make_prerequisite(name) do
            need_folder!(path)
        end
    end
    def import_glob(name, *patterns)
        make_prerequisite(name) do
            results = []
            patterns.each do |pattern|
                results.concat(need_glob!(pattern))
            end
            results.sort!{|a, b| b <=> a } # descending
            results.empty? ? false : results
        end
    end

end #~ Build
