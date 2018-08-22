#!/bin/env ruby

require 'fileutils'
require 'ruby-progressbar'
require 'zip'

NOW=Time.now
BRANCH='master'
ROOTDIR=File.expand_path(File.dirname(__FILE__))
TARNAME=File.basename(ROOTDIR)
OUTPUTNAME=File.join(ROOTDIR, "Output", "Backup",
    "#{TARNAME}_#{BRANCH}_%04d-%02d-%02d_%02d-%02d-%02d.zip" % [ 
    NOW.year, NOW.month, NOW.day, NOW.hour, NOW.min, NOW.sec
    ] )
    

puts "Create backup tarball in '#{OUTPUTNAME}' ..."

Dir.chdir(ROOTDIR)
FileUtils.mkdir_p(File.dirname(OUTPUTNAME), :verbose => true)

def fetch_git_files(branch)
    files = `git ls-tree -r --name-only #{branch}`.split("\n")
    files.delete_if do |filename|
        false == File.exist?(File.join(ROOTDIR, filename))
    end
    return files
end

input = fetch_git_files(BRANCH)
puts "Found #{input.length} files to backup"
$stdout.flush

Zip.default_compression = Zlib::BEST_COMPRESSION
Zip::File.open(OUTPUTNAME, Zip::File::CREATE) do |zipfile|
    input.each do |filename|
        zipfile.add(filename, File.join(ROOTDIR, filename))
     end
end

BYTES_TO_MEGABYTES = 1.0 / (1024 * 1024)
puts("Finished => %6.3f mb" % (File.size(OUTPUTNAME) * BYTES_TO_MEGABYTES))
