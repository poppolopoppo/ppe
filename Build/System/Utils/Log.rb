
require './Common.rb'

module Build

    module Log
        ICONS = {
            debug:      '##',
            verbose:    '--',
            info:       '->',
            warning:    '??',
            error:      '!?',
            fatal:      '!!'
        }

        LEVELS = ICONS.keys
        VERBOSITY = [ :info, :warning, :error, :fatal ]

        $show_caller = false
        $show_timestamp = true

        def self.elapsed?
            $started_at = Time.now if $started_at.nil?
            return (Time.now.to_f - $started_at.to_f) * 1000.0
        end

        def self.verbosity(min = :info)
            VERBOSITY.clear
            LEVELS.reverse.each do |level|
                VERBOSITY << level
                return if level == min
            end
            raise ArgumentError.new("unknown verbosity level: #{min}")
        end

        $pinned_stream = $stdout
        $pinned_message = nil
        def self.pin(message)
            Log.clear_pin
            $pinned_message = message
            Log.attach_pin
        end
        def self.clear_pin()
            Log.backout_pin
            $pinned_message = nil
            return
        end
        def self.backout_pin()
            $pinned_stream.print("\b" * $pinned_message.length) if $pinned_message
            return
        end
        def self.attach_pin()
            $pinned_stream.print($pinned_message) if $pinned_message
            return
        end

        def self.puts(message: '', verbosity: :info)
            return unless VERBOSITY.include?(verbosity)

            Log.backout_pin

            log = ($show_timestamp ? ("[%010.5f]" % elapsed?()) : '') <<
                " #{ICONS[verbosity]}  " <<
                message.to_s

            case verbosity
            when :debug, :verbose, :info
                outp = $stdout
            when :warning, :error
                outp = $stderr
            when :fatal
                $stdout.flush
                $stderr.flush
                raise RuntimeError.new('fatal: '<<message)
            else
                raise ArgumentError.new("unsupported log verbosity: #{verbosity}")
            end

            outp.puts(log)
            if $show_caller
                outp.puts("\t at: "<<caller[1].to_s)
            end

            Log.attach_pin

            return
        end
        LEVELS.each do |level|
            define_singleton_method(level) do |message|
                Log.puts(message: message, verbosity: level)
            end
        end
    end #~ Log

end #~ Build

if $DEBUG
    Build::Log.verbosity(:debug)
end
