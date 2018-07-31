# rename every classes

require 'fileutils'
require 'find'
require 'pp'
require 'progressbar'
require 'set'
require 'thread'

WORKER_COUNT = 8
SOURCEDIR = ARGV[0].nil? ? File.join(File.dirname(__FILE__), "../Source") : ARGV[0]

def scan_files(dir, filters, &block)
    Find.find(dir) do |path|
        if FileTest.directory?(path)
            Find.prune if File.basename(path)[0] == ?.
        elsif path =~ filters
            block.call(path)
        end
    end
end

SOURCEFILES = []
def all_sources(&block)
    scan_files(SOURCEDIR, /.*\.(h|cpp)$/i, &block)
end
all_sources {|filename| SOURCEFILES << filename }
SOURCEFILES.sort!
SOURCEFILES_COUNT = SOURCEFILES.length

RE_COMMENT  = /\/\/.*$/
RE_COMMENT2 = /\/\*.*?\*\//m
RE_PREPROC  = /#(?!#).*$/
RE_INCLUDE  = /^#\s*include/
RE_USING    = /using\s+([\w\d_]+)\s+=\s+(.*);/
RE_TYPE     = /(class|struct)\s+([^<>{\s]+?)\s*(:|{|;)(?!:)/
RE_ENUM     = /enum(\s+class)?\s+([\w\d_]+)/
RE_API      = /[\w_]+_API\s+/
RE_TPLARGS  = /<.*>/
RE_ALIGN    = /ALIGN\(\d+\)\s+/

class SetMT
    attr_reader :name, :set, :mutex
    def initialize(name)
        @name = name
        @set = Set.new
        @mutex = Mutex.new
    end
    def add(name)
        @mutex.synchronize { @set.add(name) }
    end
    def delete(name)
        @mutex.synchronize { @set.delete(name) }
    end
    def length()
        @mutex.synchronize { return @set.length }
    end
end

ENUMS       = SetMT.new('ENUMS')
CLASSES     = SetMT.new('CLASSES')
STRUCTS     = SetMT.new('STRUCTS')
TEMPLATES   = SetMT.new('TEMPLATES')

puts "Found #{SOURCEFILES_COUNT} source files ..."

def clean_name?(name)
    name.gsub!(RE_API, '')
    name.gsub!(RE_TPLARGS, '')
    name.gsub!(RE_ALIGN, '')
    return false if name[0] == 'I' || name[0] == '_'
    return false if name[0].downcase == name[0]
    return false if name.include?('(')
    return false if name == "T"
    return false if name.upcase == name
    return true
end

def strip_comments!(content)
    content.gsub!(RE_COMMENT, '')
    content.gsub!(RE_COMMENT2, '')
end

def strip_preproc!(content)
    content.gsub!(RE_PREPROC, '')
end

def mt_io_work(what, workers=WORKER_COUNT, &block)
    pbar = ProgressBar.new(what, SOURCEFILES_COUNT)
    queue = Queue.new
    SOURCEFILES.each {|filename| queue << filename}
    threads = workers.times.map do
        Thread.new do
            while !queue.empty? && filename = queue.pop
                next unless filename
                content = File.read(filename)
                block.call(filename, content)
            end
        end
    end
    while !queue.empty?
        pbar.set(SOURCEFILES_COUNT - queue.length)
        sleep(0.1)
    end
    pbar.finish
    threads.each {|t| t.join }
    return
end

mt_io_work("Find", 1) do |filename, content|
    strip_comments!(content)
    strip_preproc!(content)
    content.scan(RE_TYPE) do |m|
        type = m[0]
        name = m[1]
        if clean_name?(name)
            if type == "struct"
                STRUCTS.add(name)
            elsif type == "class"
                CLASSES.add(name)
            else
                raise "invalid type: <#{type}>"
            end
        end
    end
    content.scan(RE_USING) do |m|
        name = m[0]
        value = m[1]
        if clean_name?(name)
            if value.match(/\b(_\w+|T)\b/)
                TEMPLATES.add(name)
            else
                STRUCTS.add(name)
            end
        end
    end
    content.scan(RE_ENUM) do |m|
        name = m[1]
        if clean_name?(name)
            ENUMS.add(name)
        end
    end
end

ENUMS.set.each {|type| CLASSES.set.delete?(type)}
ENUMS.set.each {|type| STRUCTS.set.delete?(type)}

TYPES_RE = (CLASSES.set.to_a.concat STRUCTS.set.to_a).collect! {|type| [type, /\b#{type}\b</] }

mt_io_work("Tpl", 1) do |filename, content|
    strip_comments!(content)
    strip_preproc!(content)
    TYPES_RE.each do |(type, rexp)|
        content.match(rexp) do |m|
            TEMPLATES.add(type)
        end
    end
end

TEMPLATES.set.each {|type| CLASSES.set.delete?(type)}
TEMPLATES.set.each {|type| STRUCTS.set.delete?(type)}

puts "\n"<<("*"*80)<<"\nCLASSES (#{CLASSES.set.length})"
puts CLASSES.set.to_a.sort.join(', ')
puts "\n"<<("*"*80)<<"\nSTRUCTS (#{STRUCTS.set.length})"
puts STRUCTS.set.to_a.sort.join(', ')
puts "\n"<<("*"*80)<<"\nTEMPLATES (#{TEMPLATES.set.length})"
puts TEMPLATES.set.to_a.sort.join(', ')
puts "\n"<<("*"*80)<<"\nENUMS (#{ENUMS.set.length})"
puts ENUMS.set.to_a.sort.join(', ')

puts "\n\n"

def intersect?(*sets)
    sets.each do |a|
        sets.each do |b|
            next if a == b
            int = a.set.intersection(b.set)
            raise "invalid sets (#{a.name}, #{b.name}) : #{int.to_a.join ', '}" unless int.empty?
        end
    end
end

#intersect?(CLASSES, STRUCTS, TEMPLATES, ENUMS)

def make_type_re(prefix, type)
    [/(?<!FWD_REFPTR\(|\.|->)\b#{type}\b/, prefix+type]
end

CLASSES_RE  = CLASSES.set.to_a.collect!     {|type| make_type_re('F', type) }
STRUCTS_RE  = STRUCTS.set.to_a.collect!     {|type| make_type_re('F', type) }
TEMPLATES_RE= TEMPLATES.set.to_a.collect!   {|type| make_type_re('T', type) }
ENUMS_RE    = ENUMS.set.to_a.collect!       {|type| make_type_re('E', type) }

#SOURCEFILES.replace ['D:\Code\Core\Source\Core\Maths\ScalarMatrix-inl.h']

EXCLUDED_SOURCEFILES = []# "Misc/../Source/Core/Memory/RefPtr.h" ]
mt_io_work("Replace") do |filename, content|
    next if EXCLUDED_SOURCEFILES.include?(filename)
    f = File.open(filename, "w")
    content = content.split("\n").each do |line|
        if line =~ RE_INCLUDE
            f.print line
        else
            CLASSES_RE.each    {|rexp| line.gsub!(*rexp) }
            STRUCTS_RE.each    {|rexp| line.gsub!(*rexp) }
            TEMPLATES_RE.each  {|rexp| line.gsub!(*rexp) }
            ENUMS_RE.each      {|rexp| line.gsub!(*rexp) }
            f.print line
        end
        f.print "\n"
    end
    f.close
end
