#!/usr/bin/env ruby

SOURCE_FILES = /\.(h|hh|hxx|inl|cpp|cc|c|bff)$/i

user = ENV['USER']

def git_modified_files()
    `git diff-index --cached --name-only HEAD`.split("\n")
end

def sourcefile_error(sourcefile, line, message)
    $stderr.puts "#{sourcefile.filename}(#{line}): #{message}"
end

class SourceFile
    attr_reader :filename, :content
    def initialize(filename)
        @filename = filename
        @content = File.readlines(filename)
    end
    def match?(regexp, &block)
        @content.each_with_index do |str, line|
            str.match(regexp) do |*args|
                block.call(line, *args)
            end
        end
    end
    def self.create?(filename)
        File.exist?(filename) ? SourceFile.new(filename) : nil
    end
end #~ SourceFile

def sourcefile_check_nocommit(sourcefile)
    succeed = true
    sourcefile.match?(/%NOCOMMIT%/i) do |line, m|
        succeed = false
        sourcefile_error(sourcefile, line, 'found %NOCOMMIT%')
    end
    return succeed
end

HOOKS = {
    SOURCE_FILES => [
        method(:sourcefile_check_nocommit)
    ]
}

succeed = true
git_modified_files.each do |filename|
    HOOKS.each do |regexp, hooks|
        next unless filename =~ regexp
        if sourcefile = SourceFile.create?(filename)
            hooks.each {|h| succeed &= h.call(sourcefile) }
        end
    end
end

exit(0) if succeed

$stderr.puts "pre-commit hook failed"
exit(1)
