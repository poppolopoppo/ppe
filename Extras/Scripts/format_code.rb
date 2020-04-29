# script to migrate all includes

require 'fileutils'
require 'find'
require 'pp'
require 'progressbar'

WORKER_COUNT = 8
TARGET = ARGV[0]

require './codingstyle.rb'

def scan_files(dir, filters, &block)
    Find.find(dir) do |path|
        if FileTest.directory?(path)
            Find.prune if File.basename(path)[0] == ?.
        elsif path =~ filters
            block.call(path)
        end
    end
end

SOURCEFILES = Queue.new
def all_sources(&block) TARGET.nil? ?
    scan_files(SOURCEDIR, /.*\.(h|cpp)$/i, &block) :
    block.call(TARGET)
end
all_sources {|filename| SOURCEFILES << filename}
SOURCEFILES_COUNT = SOURCEFILES.length
EXIT_CODE = 0

THREADS = WORKER_COUNT.times.map do
    Thread.new do
        queue = SOURCEFILES
        while !queue.empty? && filename = queue.pop
            system(CLANGFORMAT, '-i', CODINGSTYLE_ARG, filename)
            if ($? != 0 || EXIT_CODE != 0)
                EXIT_CODE = 2
                exit(2)
            end
        end
    end
end

puts "Formatting #{SOURCEFILES_COUNT} source files ..."
pbar = ProgressBar.new("Sources", SOURCEFILES_COUNT)
while !SOURCEFILES.empty?
    sleep(0.1)
    pbar.set(SOURCEFILES_COUNT - SOURCEFILES.length)
end
pbar.finish

THREADS.each(&:join)
