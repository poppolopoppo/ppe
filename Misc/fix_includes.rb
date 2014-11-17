# script to migrate all includes

require 'fileutils'
require 'find'
require 'pp'
require 'ruby-progressbar'

def scan_files(dir, filters, &block)
    Find.find(dir) do |path|
        if FileTest.directory?(path)
            Find.prune if File.basename(path)[0] == ?.
        elsif path =~ filters
            block.call(path)
        end
    end
end

SOURCEDIR = File.join(File.dirname($0), "../Source/")
IGNOREFILES = /(stdafx|targetver)\./
INCLUDEREXP = /#(\s*)include(\s+)"([^"]+)"/i
TEMPEXT = '.fix.txt'
BACKUPEXT = '.bak.txt'

def all_headers(&block) scan_files(SOURCEDIR, /.*\.h$/i, &block) end
def all_sources(&block) scan_files(SOURCEDIR, /.*\.cpp$/i, &block) end
def all_temps(&block) scan_files(SOURCEDIR, /.*\.fix\.txt$/i, &block) end
def all_backups(&block) scan_files(SOURCEDIR, /.*\.bak\.txt$/i, &block) end

def parse(filename)
    source = filename.slice(SOURCEDIR.length..filename.length)
    dirname = File.dirname(source)
    basename = File.basename(source)
    return dirname, basename
end

def projectname(filename)
    filename.gsub(SOURCEDIR, '').split('/').first
end

HEADERS = {}
SOURCES = {}

DO_HEADERS  = true
DO_SOURCES  = true
DRYRUN      = false
CLEARBACKUP = true

if (DO_HEADERS)
    puts "Search for headers in '#{SOURCEDIR}' ..."

    all_headers do |header|
        next if header =~ IGNOREFILES
        dirname, basename = parse(header)
        raise "header doublon : #{header}" if HEADERS.include?(basename)
        HEADERS[basename] = dirname
    end

    puts "Found #{HEADERS.length} headers."
    puts "Cleaning headers ..."

    pbar = ProgressBar.create(:title => "Headers", :starting_at => 0, :total => HEADERS.length)

    fixs = 0

    HEADERS.each do |basename, dirname|
        filename = File.join(SOURCEDIR, dirname, basename)
        projname = projectname(filename)

        pbar.log "[#{projname.ljust(20)}] #{filename}"

        lines = File.readlines(filename)

        File.open(filename + TEMPEXT, 'w') do |output|
            queue = []
            lines.each_with_index do |line, line_index|
                if (match = line.match(INCLUDEREXP)) && !(match[3] =~ IGNOREFILES)
                    begin
                        b = File.basename(match[3])
                        corrected = File.join(HEADERS[b], b)
                        queue << "##{match[1]}include#{match[2]}\"#{corrected}\""
                        fixs += 1
                    rescue => e
                        $stderr.puts "failed to parse '#{line}' in #{filename}:#{line_index} !"
                        raise e
                    end
                else
                    queue.sort!
                    queue.each {|q| output.puts q }
                    queue.clear

                    output.puts line
                end
            end
            queue.sort!
            queue.each {|q| output.puts q }
            queue.clear
        end

        pbar.increment
    end

    puts "Applied #{fixs} fixs to #{HEADERS.length} headers."

end

if (DO_SOURCES)
    puts "Search for sources in '#{SOURCEDIR}' ..."

    all_sources do |source|
        next if source =~ IGNOREFILES
        dirname, basename = parse(source)
        raise "source doublon : #{source}" if SOURCES.include?(basename)
        SOURCES[basename] = dirname
    end

    puts "Found #{SOURCES.length} sources."
    puts "Cleaning sources ..."

    pbar = ProgressBar.create(:title => "Sources", :starting_at => 0, :total => SOURCES.length)

    fixs = 0

    SOURCES.each do |basename, dirname|
        filename = File.join(SOURCEDIR, dirname, basename)
        projname = projectname(filename)

        pbar.log "[#{projname.ljust(20)}] #{filename}"

        reldir = "#{dirname}/"
        prjdir = "#{projname}/"

        lines = File.readlines(filename)

        File.open(filename + TEMPEXT, 'w') do |output|
            queue = []
            lines.each do |line|
                if (match = line.match(INCLUDEREXP)) && !(match[3] =~ IGNOREFILES)
                    b = File.basename(match[3])
                    corrected = File.join(HEADERS[b], b)
                    corrected.gsub!(reldir, '')
                    corrected.gsub!(prjdir, '')
                    queue << "##{match[1]}include#{match[2]}\"#{corrected}\""
                    fixs += 1
                else
                    queue.sort!
                    queue.each {|q| output.puts q }
                    queue.clear

                    output.puts line
                end
            end
            queue.sort!
            queue.each {|q| output.puts q }
            queue.clear
        end

        pbar.increment
    end

    puts "Applied #{fixs} fixs to #{SOURCES.length} headers."

end

if (DRYRUN)

    puts "Removing temporary files ..."

    all_temps do |temp|
        FileUtils.rm temp, :verbose => true
    end

    puts "Finished dry run."

else

    puts "Applying changes and making backup ..."

    all_temps do |temp|
        orig = temp.gsub(TEMPEXT, '')
        backup = orig + BACKUPEXT
        puts "#{orig} -> #{backup}"
        FileUtils.mv orig, backup, :verbose => true, :force => true
        FileUtils.mv temp, orig, :verbose => true, :force => true
    end

    puts "All changes were applied."

end

if (CLEARBACKUP)

    puts "Removing backup files ..."

    all_backups do |temp|
        FileUtils.rm temp, :verbose => true
    end

    puts "All backups were deleted."

end

# some extra code to be used when you fucked up :p
=begin

if (false)
    version = 3
    puts "Restoring backup version #{version}"

    scan_files(SOURCEDIR, /.*\.bak#{version}.txt$/) do |backup|
        orig = backup.gsub(".bak#{version}.txt", '')
        FileUtils.cp backup, orig, :verbose => true, :force => true
    end

    exit 0
end

if (false)
    puts "Delete ALL backup !"

    scan_files(SOURCEDIR, /\.bak\d?.txt/i) do |backup|
        FileUtils.rm backup, :verbose => true
    end

    exit 0
end

if (false)
    puts "Restoring case ..."

    all_sources do |source|
        next if source =~ IGNOREFILES
        dirname, basename = parse(source)
        lines = File.readlines(source)
        lines.each do |line|
            if (match = line.match(INCLUDEREXP)) && !(match[3] =~ IGNOREFILES)
                b = File.basename(match[3]).gsub(/\.h$/i, '')
                HEADERS[b.downcase] = b
            end
        end
    end

    all_headers do |header|
        next if header =~ IGNOREFILES
        dirname, basename = parse(header)
        lines = File.readlines(header)
        lines.each do |line|
            if (match = line.match(INCLUDEREXP)) && !(match[3] =~ IGNOREFILES)
                b = File.basename(match[3]).gsub(/\.h$/i, '')
                HEADERS[b.downcase] = b
            end
        end
    end

    all_sources do |source|
        next if source =~ IGNOREFILES
        dirname, basename = parse(source)
        basenamei = HEADERS[basename.downcase.gsub(/\.cpp$/i, '')]
        if basenamei
            basenamei = basenamei + '.cpp'
            src = File.join(SOURCEDIR, dirname, basename)
            dst = File.join(SOURCEDIR, dirname, basenamei)
            puts "#{src} -> #{dst}"
            File.rename(src, dst)
        end
    end

    all_headers do |header|
        next if header =~ IGNOREFILES
        dirname, basename = parse(header)
        basenamei = HEADERS[basename.downcase.gsub(/\.h$/i, '')]
        if basenamei
            basenamei = basenamei + '.h'
            src = File.join(SOURCEDIR, dirname, basename)
            dst = File.join(SOURCEDIR, dirname, basenamei)
            puts "#{src} -> #{dst}"
            File.rename(src, dst)
        end
    end

    exit 0
end
=end
