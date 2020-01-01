module OS

    def OS.windows?()
        (/cygwin|mswin|mingw|bccwin|wince|emx/ =~ RUBY_PLATFORM) != nil
    end
    def OS.mac?()
        (/darwin/ =~ RUBY_PLATFORM) != nil
    end
    def OS.linux?()
        (/linux/ =~ RUBY_PLATFORM) != nil
    end
    def OS.unix?()
        not (OS.windows? or OS.mac? or OS.linux?)
    end

    def OS.name()
        if OS.windows?
            return "Windows"
        elsif OS.linux?
            return "Linux"
        elsif OS.mac?
            return "Mac"
        elsif OS.unix?
            return "Unix"
        else
            raise StandardError.new("unknown platfrom")
        end
    end

end #~ OS