# frozen_string_literal: true

require_once 'ANSIColor.rb'

module Build

    @@_interactive_ = nil
    def self.Interactive()
        if @@_interactive_.nil?
            @@_interactive_ = $stdout.isatty
            case ENV['TERM']
            when 'xterm','xterm-256'
                @@_interactive_ = true
            end
        end
        return @@_interactive_
    end

    module Log
        ICONS = {
            debug:      ' ~ ',
            verbose:    ' - ',
            log:        '---',
            info:       '-->',
            warning:    '/?\\',
            error:      '[!]',
            fatal:      'KO!',
        }

        LEVELS = ICONS.keys
        VERBOSITY = [ :info, :warning, :error, :fatal, :success ]
        MAXMSGLEN = 4096*4

        ANSI = ANSI.colors(Build.Interactive)
        STYLES = {
            debug: ANSI[:fg0_magenta],
            verbose: ANSI[:fg1_black],
            log: ANSI[:fg0_white],
            info: ANSI[:fg1_white]+ANSI[:bold],
            warning: ANSI[:fg0_yellow],
            error: ANSI[:fg1_red],
            fatal: ANSI[:fg1_white]+ANSI[:bg0_red]+ANSI[:bold],
        }

        $started_at = Time.now
        $show_caller = false
        $show_timestamp = false

        def self.elapsed?
            return (Time.now.to_f - $started_at.to_f) * 1000.0
        end

        def self.verbosity(min = :info)
            VERBOSITY.clear
            $stdout.sync = $stderr.sync = (min == :debug)
            $show_timestamp = (min == :verbose or min == :debug)
            LEVELS.reverse.each do |level|
                VERBOSITY << level
                return if level == min
            end
            raise ArgumentError.new("unknown verbosity level: #{min}")
        end

        if Build.Interactive
            $pinned_stream = $stderr
            $pinned_message = nil
            def self.pin(message)
                Log.clear_pin
                message = message.chomp.strip
                message.tr!("\r", '')
                message.gsub!(ANSI[:kill_line], '')
                message.gsub!(ANSI[:cursor_up], '')
                $pinned_message = message
                Log.attach_pin
            end
            def self.clear_pin()
                Log.backout_pin
                $pinned_message = nil
                return
            end
            def self.attach_pin()
                $pinned_stream.print("\r#{ANSI[:kill_line]}#{$pinned_message}\n") if $pinned_message
                return
            end
            def self.backout_pin()
                $pinned_stream.print(ANSI[:cursor_up]+ANSI[:kill_line]) if $pinned_message
                return
            end
        else
            def self.pin(message) end
            def self.clear_pin() end
            def self.attach_pin() end
            def self.backout_pin() end
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

        def self.raw(message, args: nil, verbosity: :log)
            unless VERBOSITY.include?(verbosity)
                case verbosity
                when log
                    self.pin(message)
                end
                return
            end
            Log.without_pin do
                message = message.to_s
                message = message % args if args

                if message.length > MAXMSGLEN
                    Log.error 'Log: cropping next entry because it\'s more than %d characters long (%d chars)', MAXMSGLEN, message.length
                    message = message[0..MAXMSGLEN]
                end

                case verbosity
                when :debug, :verbose, :log, :info
                    $stdout.puts(Log.ansi_style(verbosity, message))
                    $stdout.flush
                when :warning, :error
                    $stderr.puts(Log.ansi_style(verbosity, message))
                    $stdout.flush
                when :fatal
                    $stdout.flush
                    $stderr.flush
                    $stderr.puts(Log.ansi_style(verbosity, message))
                    raise FatalError.new("fatal error")
                else
                    $stderr.flush
                    raise ArgumentError.new("unsupported log verbosity: #{verbosity}")
                end

            end if VERBOSITY.include?(verbosity) || verbosity == :fatal
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

            log = ($show_timestamp ? ("[%011.4f]" % elapsed?()) : '') +
                " #{ICONS[verbosity]}  #{message}"
            log << "\n\tat: " << caller_locations(2, 1)[0].to_s if $show_caller

            Log.raw(log, verbosity: verbosity)
        end

        LEVELS.each do |level|
            define_singleton_method("#{level}?") do
                VERBOSITY.include?(level)
            end
            define_singleton_method(level) do |message, *args|
                args = nil if args.empty?
                Log.puts(message: message, args: args, verbosity: level)
            end
        end

    private
        if Build.Interactive
            def self.ansi_style(verbosity, message)
                case verbosity
                when :success
                    result = String.new << ANSI[:bg0_white]
                    col = nil
                    message.each_char do |ch|
                        if col.nil? || ch =~ /\s/
                            prv = col
                            col = ANSI[::ANSI.random_color(:fg1)] while prv == col
                            result << col
                        end
                        result << ch
                    end
                    result << ANSI[:reset]
                    return result
                else
                    return STYLES[verbosity]+message.to_s+ANSI[:reset]
                end
            end
        else
            def self.ansi_style(verbosity, message)
                return message
            end
        end

    end #~ Log

    module Assert

        def self.check(&block)
            if $DEBUG
                Log.fatal 'assertion failed' unless block.call
            end
        end

        def self.expect(x, klass) Log.fatal 'unexpected value: <%s> "%s", need <%s>', x.class, x.to_s, klass unless x.is_a?(klass) end
        def self.expect?(x, klass) Log.fatal 'unexpected value: <%s> "%s", need <%s>', x.class, x.to_s, klass unless x.nil? || x.is_a?(klass) end
        def self.unexpected(x) Log.fatal 'unexpected value: <%s> "%s"', x.class, x.to_s end
        def self.unreached() Log.fatal 'unreachable state' end
        def self.not_implemented() Log.fatal 'not implemented' end

    end #~ Assert

end #~ Build

if $DEBUG
    Build::Log.verbosity(:debug)
end
