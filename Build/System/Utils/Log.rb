
module Build

    module Log
        ICONS = {
            debug:      ' ~ ',
            verbose:    '---',
            info:       '-->',
            warning:    '/?\\',
            error:      '[!]',
            fatal:      'KO!',
        }

        LEVELS = ICONS.keys
        VERBOSITY = [ :info, :warning, :error, :fatal ]

        $started_at = Time.now
        $show_caller = false
        $show_timestamp = true

        def self.elapsed?
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
        def self.attach_pin()
            $pinned_stream.print($pinned_message) if $pinned_message
            return
        end
        def self.backout_pin()
            $pinned_stream.print("\b" * $pinned_message.length) if $pinned_message
            return
        end
        def self.without_pin(&block)
            Log.backout_pin
            block.call
            Log.attach_pin
        end

        class FatalError < RuntimeError
            def initialize(message)
                super(message)
            end
        end #~ FatalError

        def self.raw(message, args: nil, verbosity: :info)
            Log.without_pin do
                message = message % args if args
                case verbosity
                when :debug, :verbose, :info
                    $stdout.puts(message)
                when :warning, :error
                    $stderr.puts(message)
                when :fatal
                    $stdout.flush
                    $stderr.flush
                    raise FatalError.new('fatal: '<<message)
                else
                    $stderr.flush
                    raise ArgumentError.new("unsupported log verbosity: #{verbosity}")
                end
            end if VERBOSITY.include?(verbosity)
        end

        def self.puts(message: '', args: nil, verbosity: :info)
            return unless VERBOSITY.include?(verbosity)

            case message
            when String
                message = message % args if args
            else
                raise ArgumentError.new("can't format object with arguments: <#{message}> % #{args}") if args
                message = message.to_s
            end

            log = ($show_timestamp ? ("[%010.5f]" % elapsed?()) : '') <<
                " #{ICONS[verbosity]}  " <<
                message.to_s
            log << "\n\tat: " << caller[1].to_s if $show_caller

            Log.raw(log, verbosity: verbosity)
        end

        LEVELS.each do |level|
            define_singleton_method(level) do |message, *args|
                args = nil if args.empty?
                Log.puts(message: message, args: args, verbosity: level)
            end
        end

    end #~ Log

end #~ Build

if $DEBUG
    Build::Log.verbosity(:debug)
end