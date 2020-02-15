
require './Common.rb'

require './Utils/Options.rb'
require './Utils/Log.rb'

module Build

    class Prerequisite < OptionVariable
        def initialize(name, getter)
            super(name, nil)
            @getter = getter
        end
        def available?()
            evaluate!.value
        end
        def evaluate!()
            restore(self.instance_exec(&@getter)) if @value.nil?
            return self
        end
    public
        def need_cmdline!(*args)
            args.collect!{|x| x =~ /\s/ ? "'#{x}''" : x }
            result = %{ #{args.join(' ')} }
            if $?.sucess?
                Log.debug("command succeed: %s -> '%s'", args, result)
                return result
            else
                Log.verbose("command failed: %s (exit code = %s)", args, $?)
                return false
            end
        end
        def need_file!(path)
            if File.exists?(id)
                Log.debug("found file '%s'", path)
                return true
            else
                Log.verbose("file does not exists '%s'", path)
                return false
            end
        end
        def need_folder!(path)
            if Dir.exists?(id)
                Log.debug("found directory '%s'", path)
                return true
            else
                Log.verbose("directory does not exists '%s'", path)
                return false
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
    end #~ Prerequisite

    def make_prerequisite(name, &getter)
        prereq = Prerequisite.new("#{self.name}.#{name}", getter)
        self.class.define_method(name) do
            return prereq.evaluate!
        end
        PersistentConfig[:vars] << prereq
        return prereq
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
            filenames.each do |path|
                return unless need_file!(path)
            end
            return filenames
        end
    end

end #~ Build
