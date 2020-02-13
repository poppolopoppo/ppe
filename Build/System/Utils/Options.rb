
require './Common.rb'

require './Utils/Log.rb'

require 'optparse'
require 'yaml'

module Build

    OptionVariables = []

    class OptionVariable
        attr_reader :name, :opts
        attr_accessor :value
        def initialize(name, value, *opts)
            @name = name
            @opts = opts
            @value = value
        end
        def parse(opt)
            opt.on(*@opts) { |x| @value = x }
        end
        def self.attach(host, name, description, flag, default, values)
            var = OptionVariable.new("#{host}.#{name}", default, flag, description)
            var.opts << values unless values.nil?
            host.class.define_method(name) { return var.value }
            OptionVariables << var
            return var
        end
    end #~ OptionVariable

    def self.opt_array(name, description, init: [], values: nil)
        var = OptionVariable.attach(self, name, description, '--'<<name.to_s<<' X,Y,Z', [], values)
        var.opts << Array
        return var
    end
    def self.opt_value(name, description, init: nil, values: nil)
        OptionVariable.attach(self, name, description, '--'<<name.to_s<<' VALUE', nil, values)
    end
    def self.opt_switch(name, description, init: false)
        OptionVariable.attach(self, name, description, '--[no-]'<<name.to_s, false, nil)
    end

    PersistentConfig = {
        file: File.join(File.dirname($0), '.'<<File.basename($0)<<'.yml'),
        vars: []
    }

    def self.persistent_array(name, description, init: [], values: nil)
        var = opt_array(name, description, init: init, values: values)
        PersistentConfig[:vars] << var
        return var
    end
    def self.persistent_value(name, description, init: nil, values: nil)
        var = opt_value(name, description, init: init, values: values)
        PersistentConfig[:vars] << var
        return var
    end
    def self.persistent_switch(name, description, init: false)
        var = opt_switch(name, description, init: init)
        PersistentConfig[:vars] << var
        return var
    end

    def self.parse_options()
        OptionParser.new do |opts|
            opts.banner = "Usage: #{File.basename($0)} [options]"

            opts.separator ""
            opts.separator "Config options:"

            OptionVariables.each { |var| var.parse(opts) }

            opts.separator ""
            opts.separator "Common options:"

            opts.on_tail("-c FILE", "--config FILE", "Set config file") do |fname|
                Build.load_options(fname)
                PersistentConfig[:file] = fname
            end
            opts.on_tail("-q", "--[no-]quiet", "Run quietly") do |v|
                Log.verbosity(:error) if v
            end
            opts.on_tail("-v", "--[no-]verbose", "Run verbosely") do |v|
                Log.verbosity(:verbose) if v
            end
            opts.on_tail("-d", "--[no-]debug", "Run with dbeug") do |v|
                $DEBUG = v
                Log.verbosity(:debug) if v
            end
            opts.on_tail("--version", "Show build version") do |v|
                Log.info("Version: #{Build::VERSION}")
                exit
            end
            opts.on_tail("-h", "--help", "Show this message") do
                puts opts
                exit
              end

            opts.separator ""
        end.parse!
    end

    def self.save_options(dst=PersistentConfig[:file])
        Log.verbose("save persistent options to '#{dst}'")
        serialized = {}
        PersistentConfig[:vars].each do |var|
            serialized[var.name] = var.value unless var.value.nil?
        end
        begin
            File.open(dst, 'w') do |fd|
                fd.write(serialized.to_yaml)
            end
            return true
        rescue Errno::ENOENT
            Log.warning('failed to save persistent options to '"#{dst}': non writable path ?")
        end
    end

    def self.load_options(src=PersistentConfig[:file])
        Log.verbose("load persistent options from '#{src}'")
        begin
            serialized = YAML.load(File.read(src))
            PersistentConfig[:vars].each do |var|
                if serialized.key?(var.name)
                    var.value = serialized[var.name]
                end
            end
            return true
        rescue Errno::ENOENT
            Log.warning('failed to load persistent options from '"#{src}': file does not exist ?")
            return false
        end
    end

end #~ Build
