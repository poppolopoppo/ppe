# frozen_string_literal: true

require_once '../Common.rb'
require_once 'Log.rb'

require 'open3'

module Process

    def self.trim_crlf(str)
        str.chomp!
        str.rstrip!
        return str.empty? ? nil : str
    end

    def self.start(env, *cmd, chdir: $WorkspacePath, quiet: false)
        result = nil
        Build::Log.log 'Process: %s (env: %s, quiet: %s)', cmd.join(' '), env, quiet
        Open3.popen3(env, *cmd, chdir: chdir) do |io_in, io_out, io_err, wait_thr|
            loop do
                if line = io_out.gets
                    line = trim_crlf(line)
                    if block_given?
                        yield line
                    elsif not quiet and line
                        if Build::Log.log?
                            Build::Log.raw(line, verbosity: :log)
                        else
                            Build::Log.pin(line)
                        end
                    end
                elsif line = io_err.gets
                    line = trim_crlf(line)
                    if line
                        Build::Log.raw(line, verbosity: :error)
                    end
                else
                    break
                end
            end
            result = wait_thr.value
        end
        Build::Log.clear_pin
        return result
    end

end #~ Process