#!/bin/env ruby

VERSION = "1.0" # increment to invalidate all stdafx.generated.h
PERCENT_INCLUSION_THRESHOLD = 75
MIN_INCLUSION_COUNT = 6

require 'rbconfig'
RUBY_INTERPRETER_PATH = File.join(
    RbConfig::CONFIG["bindir"],
    RbConfig::CONFIG["RUBY_INSTALL_NAME"] +
    RbConfig::CONFIG["EXEEXT"] )

CURRENT_BASENAME = "#{File.basename(__FILE__)}"
def print_usage()
    $stderr.puts "usage: #{CURRENT_BASENAME} <PCHFILE> <TARGET> <CONFIG> [<CONFIG>...]"
end
def missing_arg(name)
    $stderr.puts "error: missing argument <#{name}>"
    print_usage
    exit 2
end

PCHFILE = ARGV[0]
TARGET = ARGV[1]
CONFIGS = ARGV[2..-1]
missing_arg(:PCHFILE) if PCHFILE.nil?
missing_arg(:TARGET) if TARGET.nil?
missing_arg(:CONFIGS) if CONFIGS.nil?

BUILDDIR = File.dirname(__FILE__)
SOURCEDIR = File.absolute_path(File.join(BUILDDIR, '..', 'Source'))
PROJECTDIR = File.absolute_path(File.join(BUILDDIR, '..', 'Source', TARGET))
FBUILDCMD = File.absolute_path(File.join(BUILDDIR, '..', 'fbuild.rb'))

#GIT_REFLOG = %x(git reflog -1).split(' ').first

ADDITIONAL_INFOS = []

def dputs(*args)
    puts(*args) if $DEBUG
end

require 'pathname'
require 'set'

class FParser
    attr_reader :line, :depth
    def initialize(input)
        @input = input
        @line = -1
        @depth = -1
        @current = nil
        @stack = []
        @index = {}
    end
    def current() @input[@line] end
    def next() @input[@line + 1] end
    def next!()
        @line += 1
        return false if current.nil?
        d = FParser.item_depth(current)

        if !@stack.empty? && @depth >= d
            @line, @depth = *@stack.pop
            dputs "pop to #{@line}:#{@depth}"
        end

        key = current.strip
        if key == "..."
            raise "invalid traversal : should have jumped..."
        else
            jump = @index[key]
            if jump.nil?
                @index[key] = [@line, d]
                dputs "register <#{key}>@#{@line+1}:#{d}" # register first child
            elsif self.next().strip == '...'
                @stack << [@line+2, d] # skip current line + "..."
                @line, @depth = *jump
                dputs "jump to <#{key}>@#{@line}:#{@depth}"
            end
        end

        return true
    end
private
    def self.item_depth(line)
        return 0 if line.nil?
        depth = 0
        depth += 1 while depth < line.length && line[depth] == ' '
        raise "invalid spacing" if 0 != (depth % 2)
        return (depth / 2)
    end
end

RE_FILEDEP = /File\s+(.*)$/
RE_BINARY = /\.(exe|dll)$/i
RE_UNITY = /Unity_(\d+)_of_(\d+)\.cpp/
RE_SYSHEADERS = /(stdafx)|(targetver.h)/i
def parse_file(output, dependencies)
    raise "invalid fastbuild output" if output.nil?
    output.scan(RE_FILEDEP) do |match|
        filename = match[0]
        next if filename.nil?
        next if filename =~ RE_BINARY
        next if filename =~ RE_UNITY
        next if filename =~ RE_SYSHEADERS
        count = dependencies[filename]
        count = 0 if count.nil?
        dependencies[filename] = (count + 1)
    end
end



def process_target_deps(target, define, stds, sdks, prjs)
    dependencies = {}

    parser = FParser.new(%x("#{RUBY_INTERPRETER_PATH}" "#{FBUILDCMD}" -showdeps "#{target}").split("\n"))
    while parser.next!
        dputs("[%5d][%3d] %s" % [parser.line, parser.depth, parser.current])
        parse_file(parser.current, dependencies)
    end

    headers=[]
    sources=[]
    dependencies.each do |filename, count|
        filename = Pathname.new(filename)
        filename = filename.realpath.to_s
        if (filename =~ /\.h$/i || File.extname(filename).empty?)
            headers << [filename, count]
        elsif filename =~ /\.((cpp)|c)$/i
            next unless filename.upcase.include?(PROJECTDIR.upcase)
            sources << [filename, count]
        end
    end

    headerfiles_count = headers.length
    sourcefiles_count = sources.length

    std=[]
    prj=[]
    sdk=[]
    sourcedir_cmp = SOURCEDIR.upcase + '/'
    headers.each do |(filename, count)|
        if filename.upcase.include?(sourcedir_cmp)
            next if filename =~ /\-inl\.h$/i
            filename = filename[sourcedir_cmp.length..-1]
            percent = ((100 * count) / sourcefiles_count)
            prj << "\"#{filename}\" // #{percent}% #{count}/#{sourcefiles_count}" if percent > PERCENT_INCLUSION_THRESHOLD && count > MIN_INCLUSION_COUNT
        elsif File.extname(filename).empty?
            basename = File.basename(filename)
            next if basename[0] == 'x' # MSVC includes
            std << "<#{basename}>"
        elsif filename.split('/')[-2].upcase == 'INCLUDE'
            basename = File.basename(filename)
            sdk << "\"#{basename}\""
        end
    end

    ADDITIONAL_INFOS << "Processed dependencies for #{target} : #{std.length} std, #{sdk.length} sdk, #{prj.length} prj, #{sourcefiles_count} src"
    puts ADDITIONAL_INFOS.last

    list = stds[std]
    list = [] if list.nil?
    list << define unless list.include?(define)
    stds[std] = list

    list = sdks[sdk]
    list = [] if list.nil?
    list << define unless list.include?(define)
    sdks[sdk] = list

    list = prjs[prj]
    list = [] if list.nil?
    list << define unless list.include?(define)
    prjs[prj] = list
end

HEADERS = {
    "Standard" => {},
    "SDK" => {},
    "Project" => {}
}
CONFIGS.each do |config|
    target = "#{TARGET}-#{config}"
    define = "BUILDCONFIG_%s_%s" % config.split('-')
    process_target_deps(target, define, HEADERS['Standard'], HEADERS['SDK'], HEADERS['Project'])
end

File.open(PCHFILE, 'w') do |oss|
    oss.puts "/*"
    oss.puts "// Generated by #{CURRENT_BASENAME} v#{VERSION} for target <#{TARGET}>"
    oss.puts("//" + ("-")*78)
    oss.puts "// Date : #{Time.now.utc}"
    #oss.puts "// User : #{ENV['USERDOMAIN']}"
    #oss.puts "// Sha1 : #{GIT_REFLOG}"
    oss.puts "*/"
    unless ADDITIONAL_INFOS.empty?
        oss.puts
        ADDITIONAL_INFOS.each do |info|
            oss.puts "// #{info}"
        end
    end
    oss.puts
    if CONFIGS.empty?
        oss.puts "// empty precompiled header generated without config"
    else
        HEADERS.each do |name, entries|
            oss.puts "/*"
            oss.puts "// #{name} headers"
            oss.puts("//" + ("-")*78)
            oss.puts "*/"
            oss.puts "#if 0 // dummy" unless entries.length == 1

            entries.each do |headers, defines|
                unless entries.length == 1
                    cond = defines.collect{|m| "defined(#{m})" }.join(" || ")
                    oss.puts "#elif #{cond}"
                end
                indent = entries.length == 1 ? "" : "    "
                headers.each do |header|
                    oss.puts "##{indent}include #{header}"
                end
            end
            unless entries.length == 1
                oss.puts "#else"
                oss.puts '#     error "unknown build config"'
                oss.puts "#endif"
            end
            oss.puts
        end
    end
end
