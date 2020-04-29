# frozen_string_literal: true

require_once '../Common.rb'

module Build

    const_memoize(self, :os_x64?) do
        (/x64/ =~ RUBY_PLATFORM) != nil
    end
    const_memoize(self, :os_windows?) do
        (/cygwin|mswin|mingw|bccwin|wince|emx/ =~ RUBY_PLATFORM) != nil
    end
    const_memoize(self, :os_mac?) do
        (/darwin/ =~ RUBY_PLATFORM) != nil
    end
    const_memoize(self, :os_linux?) do
        (/linux/ =~ RUBY_PLATFORM) != nil
    end
    const_memoize(self, :os_unix?) do
        not (Build.os_windows? or Build.os_mac? or Build.os_linux?)
    end

    const_memoize(self, :os_name) do
        if Build.os_windows?
            :Windows
        elsif Build.os_linux?
            :Linux
        elsif Build.os_mac?
            :Mac
        elsif Build.os_unix?
            :Unix
        else
            raise StandardError.new("unknown platform")
        end
    end

end #~ Build