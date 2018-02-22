#!/bin/env ruby

require 'set'

VERSION = "1.1" # increment to invalidate all stdafx.generated.h
PERCENT_INCLUSION_THRESHOLD = 70
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
            #dputs "pop to #{@line}:#{@depth}"
        end

        key = current.strip
        if key == "..."
            raise "invalid traversal : should have jumped..."
        else
            jump = @index[key]
            if jump.nil?
                @index[key] = [@line, d]
                #dputs "register <#{key}>@#{@line+1}:#{d}" # register first child
            elsif self.next().strip == '...'
                @stack << [@line+2, d] # skip current line + "..."
                @line, @depth = *jump
                #dputs "jump to <#{key}>@#{@line}:#{@depth}"
            end
        end

        return true
    end
    def self.item_depth(line)
        return 0 if line.nil?
        depth = 0
        depth += 1 while depth < line.length && line[depth] == ' '
        raise "invalid spacing" if 0 != (depth % 2)
        return (depth / 2)
    end
end

RE_SOURCEDIR = SOURCEDIR.downcase.gsub('\\', '/')
RE_PROJECTDIR = PROJECTDIR.downcase.gsub('\\', '/')
class FDependency
    attr_reader :filename
    attr_reader :min_depth, :max_depth, :count
    attr_reader :is_source, :is_project
    def initialize(filename, depth)
        @filename = filename
        @min_depth = @max_depth = depth
        @count = 1
        @is_source = (@filename.downcase.include? RE_SOURCEDIR)
        @is_project = @is_source && (@filename.downcase.include? RE_PROJECTDIR)
    end
    def add(depth)
        @min_depth = depth if @min_depth > depth
        @max_depth = depth if @max_depth < depth
        @count += 1
        return
    end
    def to_s() @filename end
end #~ FDependency

RE_FILEDEP = /File\s+(.*)$/
RE_BINARY = /\.(exe|dll)$/i
RE_UNITY = /Unity_(\d+)_of_(\d+)\.cpp/
RE_STDAFX = /(stdafx)|(targetver\.h)/i
def parse_showdeps(output, depth, dependencies)
    raise "invalid fastbuild output" if output.nil?
    output.scan(RE_FILEDEP) do |match|
        filename = match[0]
        next if filename.nil?
        next if filename =~ RE_BINARY
        next if filename =~ RE_UNITY
        next if filename =~ RE_STDAFX

        filename = Pathname.new(filename).realpath.to_s
        filename.gsub!('\\', '/')

        key = filename.downcase

        if dep = dependencies[key]
            dep.add(depth)
        else
            dep = dependencies[key] = FDependency.new(filename, depth)
        end

        #dputs("[dep] '#{filename}' : #{dep.count}") if filename =~ /core\.h$/i
    end
end

RE_CPP = /\.(cpp)|c/i
RE_CPPINCLUDE = /#\s*include\s+[<'"](.*)[>'"]/
INCLUDE_SEARCHDIRS = [PROJECTDIR, SOURCEDIR]
def parse_source(filename, include_searchdirs, dependencies)
    File.foreach(filename) do |line|
        line = line.force_encoding('iso-8859-1').encode('utf-8')
        if m = line.match(RE_CPPINCLUDE)
            header = m[1]
            next if header =~ RE_STDAFX
            next if header =~ RE_CPP
            next if File.extname(header).empty? # system headers ignored

            include_searchdirs.each do |path|
                header = File.join(path, m[1])
                break if File.exist?(header)
            end

            next unless File.exist?(header) # don't handle #ifdef/#if so it could be an include from another platform/target

            header = Pathname.new(header).realpath.to_s
            dependencies << header.downcase.gsub('\\', '/')
        end
    end
    return
end

def process_target_deps(target, define, stds, sdks, prjs)
    dependencies = {}

    parser = FParser.new(%x("#{RUBY_INTERPRETER_PATH}" "#{FBUILDCMD}" -showdeps "#{target}").split("\n"))
    while parser.next!
        #dputs("[%5d][%3d] %s" % [parser.line, parser.depth, parser.current])
        parse_showdeps(parser.current, parser.depth, dependencies)
    end

    include_searchdirs = Set.new INCLUDE_SEARCHDIRS
    dependencies.values.each do |dep|
        path = File.dirname(dep.filename)
        include_searchdirs << path
    end

    headers_directlyincluded = Set.new
    dependencies.each do |key, dep|
        next unless dep.is_project
        headers_directlyincluded << key
        parse_source(dep.filename, include_searchdirs, headers_directlyincluded)
    end

    headers=[]
    sourcefiles_count = 0
    dependencies.each do |key, dep|
        filename = dep.filename
        extname = File.extname(dep.filename)
        if extname.empty?
            #dputs "[STD] #{filename} #{dep.count}"
            next if File.basename(filename)[0] == 'x' # MSVC specific headers
            headers << dep
        elsif filename =~ /\.h$/i && headers_directlyincluded.include?(key)
            #dputs "[PRJ] #{filename} #{dep.count}"
            headers << dep
        elsif dep.is_project && extname =~ /\.((cpp)|c)$/i
            #dputs "[SRC] #{filename} #{dep.count}"
            sourcefiles_count += 1
        else
            #dputs "[NON] #{filename} #{dep.count} #{dep.is_project}"
        end
    end

    std=[]
    prj=[]
    sdk=[]
    headers.each do |dep|
        filename, count = dep.filename, dep.count
        if dep.is_source
            next if filename =~ /\-inl\.h$/i
            filename = filename[(SOURCEDIR.length+1)..-1]
            percent = ((100 * count) / sourcefiles_count)
            if percent > PERCENT_INCLUSION_THRESHOLD && count > MIN_INCLUSION_COUNT
                prj << "\"#{filename}\"" #// #{percent}% #{count}/#{sourcefiles_count}"
            end
        elsif File.extname(filename).empty?
            basename = File.basename(filename)
            std << "<#{basename}>"
        elsif filename.split('/')[-2].upcase == 'INCLUDE'
            basename = File.basename(filename)
            sdk << "\"#{basename}\""
        end
    end

    ADDITIONAL_INFOS << ("%48s | %3s | %3s | %3s | %3s" % [
        target, std.length, sdk.length, prj.length, sourcefiles_count])

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
    "Standard"  => {},
    "SDK"       => {},
    "Project"   => {}
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
        oss.puts("// %48s | %3s | %3s | %3s | %3s" % [
            'Target', 'STD', 'SDK', 'PRJ', 'SRC'])
        oss.puts("// " << ('-'*(48+3*2*4)))
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
