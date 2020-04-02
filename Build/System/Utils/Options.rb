# frozen_string_literal: true

require_once '../Utils/Log.rb'

require 'optparse'
require 'yaml'

module Build

    Args = []
    Commands = []
    Script = File.absolute_path($0)

    $BuildClean = false
    def Clean() $BuildClean end

    class Command
        attr_reader :name, :desc, :block
        def initialize(name, desc, &block)
            Log.fatal 'name must be a symbol: "%s"', name unless name.is_a?(Symbol)
            @name = name
            @desc = desc
            @block = block
        end
        def <=>(other) @name <=> other.name end
    end #~ Command

    $BuildCommand = []
    def self.make_command(name, desc, &block)
        cmd = Command.new(name, desc, &block)
        Commands << cmd
        return cmd
    end
    def defer_command(cmd)
        Log.debug 'defer <%s> command execution', cmd.name
        $BuildCommand << cmd
    end
    def run_command(&namespace)
        $BuildCommand.each do |cmd|
            Log.info 'run <%s> command: %s', cmd.name, Build::Args
            cmd.block.call(&namespace)
        end
        $BuildCommand.clear
        return
    end
    def make_commandline(*args)
        return [ RUBY_INTERPRETER_PATH, Build::Script ] + args
    end
    def make_commandstr(*args)
        line = make_commandline(*args)
        line.collect!{|x| x.match?(/\s/) ? "\"#{x}\"" : x }
        return line.join(' ')
    end

    OptionVariables = []

    class OptionVariable
        attr_reader :name, :opts
        attr_accessor :value, :default, :persistent
        attr_reader :validator
        def initialize(name, value)
            @name = name
            @value = value
            @default = value.deep_dup
            @persistent = false

            @flag = nil
            @description = nil
            @opts = []

            @validator = nil
        end
        def opt_on!(flag, description, values)
            @flag = flag
            @description = description
            @values = values
            return self
        end
        def parse(opt)
            desc = @persistent ?
                "#{@value}\t#{@description}" :
                @description

            if @values
                desc += "[#{@values.join(',')}]"
                opt.on(@flag, desc, *@opts) do |x|
                    @value = nil
                    @values.each do |y|
                        if x.downcase == y.downcase
                            @value = y
                        end
                    end
                    if @value.nil?
                        Log.fatal '%s: unknown value "%s" [%s]', @flag, x, @values.join(',')
                    end
                end
            else
                opt.on(@flag, desc, *@opts) { |x| @value = x }
            end
        end
        def restore!(val)
            Log.debug("restore <%s> option = '%s'", @name, val)
            if @validator
                unless self.instance_exec(val, &@validator)
                    Log.warning("couldn't validate <%s> option = '%s', keeping '%s'", @name, val, @value)
                    return @value
                end
            end
            @value = val
        end
        def set!(val)
            Log.debug("set <%s> option to explicit = '%s'", @name, val)
            @value = val
        end
        def default!()
            Log.debug("set <%s> option to default = '%s'", @name, @default)
            @value = @default
        end
        def validate!(&block)
            @validator = block
            return self
        end
        def to_s()
            "#{@name}=#{@value}"
        end
        def <=>(other) @name <=> other.name end
        def self.attach(host, name, description, flag, default, values)
            var = OptionVariable.new("#{host}.#{name}", default)
            var.opt_on!(flag, description, values)
            host.class.define_method(name) { return var.value }
            OptionVariables << var
            return var
        end
    end #~ OptionVariable

    def opt_array(name, description, init: [], values: nil)
        var = OptionVariable.attach(self, name, description, "--#{name} X,Y,Z", init, values)
        var.opts << Array
        return var
    end
    def opt_value(name, description, init: nil, values: nil)
        OptionVariable.attach(self, name, description, "--#{name} VALUE", init, values)
    end
    def opt_switch(name, description, init: false)
        OptionVariable.attach(self, name, description, "--[no-]#{name}", init, nil)
    end

    PersistentConfig = {
        file: File.join(File.dirname(Build::Script), ".#{File.basename(Build::Script, File.extname(Build::Script))}.build.yml"),
        hash: nil,
        vars: {},
        data: {},
    }

    def self.make_persistent_opt(var)
        var.persistent = true
        PersistentConfig[:vars][var.name] = var
        if PersistentConfig[:data].include?(var.name)
            var.set!(PersistentConfig[:data][var.name])
        end
        return var
    end
    def self.restore_persistent_opt(name, value)
        PersistentConfig[:vars][name].restore!(value)
    end

    def persistent_array(name, description, init: [], values: nil)
        var = opt_array(name, description, init: init, values: values)
        return Build.make_persistent_opt(var)
    end
    def persistent_value(name, description, init: nil, values: nil)
        var = opt_value(name, description, init: init, values: values)
        return Build.make_persistent_opt(var)
    end
    def persistent_switch(name, description, init: false)
        var = opt_switch(name, description, init: init)
        return Build.make_persistent_opt(var)
    end

    def self.parse_options(full=false)
        Args.replace(OptionParser.new do |opts|
            opts.banner = "Usage: #{File.basename(Build::Script)} command [options] [args]"

            opts.separator ""
            opts.separator "Commands:"
            Commands.sort_by(&:name).each do |cmd|
                opts.on("--#{cmd.name}", cmd.desc) do
                    Build.defer_command(cmd)
                end
            end

            vars = OptionVariables.sort_by(&:name)

            opts.separator ""
            opts.separator "Command options:"
            vars.each do |var|
                next if var.persistent
                var.parse(opts)
            end

            opts.separator ""
            opts.separator "Persistent config:"
            vars.each do |var|
                next unless var.persistent
                var.parse(opts)
            end

            opts.separator ""
            opts.separator "Common options:"

            opts.on("-w PATH", "--workspace PATH", "Set workspace path") do |path|
                Build.set_workspace_path(path)
            end
            opts.on("-c FILE", "--config FILE", "Set config file") do |fname|
                Build.load_options(fname)
                PersistentConfig[:file] = fname
            end
            opts.on("--clean", "Clear persistent data") do |fname|
                $BuildClean = true
                PersistentConfig[:data] = {}
                PersistentConfig[:vars].each do |name, var|
                    var.default!
                end
            end

            opts.separator ''

            opts.on("-v", "--verbose", "Run verbosely") do
                Log.verbosity(:verbose)
            end
            opts.on("-q", "--quiet", "Run quietly") do
                Log.verbosity(:error)
            end
            opts.on("-d", "--debug", "Run with dbeug") do
                $DEBUG = true
                Log.verbosity(:debug)
            end
            opts.on("-T", "--trace", "Trace program execution") do
                $show_caller = true
            end
            opts.on("-t", "--timestamp", "Show log timestamp") do
                $show_timestamp = true
            end

            opts.separator ''

            opts.on("--version", "Show build version") do
                Log.info("Version: %s", Build::VERSION)
                exit
            end
            opts.on("-h", "--help", "Show this message") do
                Log.raw opts
                exit
            end

            opts.separator ''
        end.parse!)
    end

    def self.save_options(dst=PersistentConfig[:file])
        serialized = {}
        PersistentConfig[:vars].each do |name, var|
            serialized[var.name] = var.value unless var.value.nil?
        end
        serialized = serialized.sort.to_h
        begin
            if PersistentConfig[:hash] != serialized.hash
                Log.verbose("save persistent options to '%s'", dst)
                File.open(dst, 'w') do |fd|
                    fd.write(serialized.to_yaml)
                end
                PersistentConfig[:data] = serialized
            else
                Log.verbose('skip saving persitent options since they did not change')
            end
            return true
        rescue Errno::ENOENT
            Log.warning("failed to save persistent options to '%s': non writable path ?", dst)
        end
    end

    def self.load_options(src=PersistentConfig[:file])
        Log.verbose("load persistent options from '#{src}'")
        begin
            serialized = YAML.load(File.read(src))
            PersistentConfig[:data] = serialized
            PersistentConfig[:hash] = serialized.hash
            PersistentConfig[:vars].each do |name, var|
                if serialized.key?(var.name)
                    var.restore!(serialized[var.name])
                end
            end
            return true
        rescue Errno::ENOENT
            Log.warning("failed to load persistent options from '%s': file does not exist ?", src)
            return false
        end
    end

    Build.make_command(:print, 'Show config data') do
        PersistentConfig[:vars].each do |name, var|
            Log.info 'persitent[%s] = %s', name, var.value.to_s
        end
    end

end #~ Build
