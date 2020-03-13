# frozen_string_literal: true

require_once '../Common.rb'

module Build

    const_memoize(self, :is_x64?) do
        (/x64/ =~ RUBY_PLATFORM) != nil
    end
    const_memoize(self, :is_windows?) do
        (/cygwin|mswin|mingw|bccwin|wince|emx/ =~ RUBY_PLATFORM) != nil
    end
    const_memoize(self, :is_mac?) do
        (/darwin/ =~ RUBY_PLATFORM) != nil
    end
    const_memoize(self, :is_linux?) do
        (/linux/ =~ RUBY_PLATFORM) != nil
    end
    const_memoize(self, :is_unix?) do
        not (Build.is_windows? or Build.is_mac? or Build.linux?)
    end

    const_memoize(self, :os_name) do
        if Build.is_windows?
            "Windows"
        elsif Build.is_linux?
            "Linux"
        elsif Build.is_mac?
            "Mac"
        elsif Build.is_unix?
            "Unix"
        else
            raise StandardError.new("unknown platform")
        end
    end

end #~ Build