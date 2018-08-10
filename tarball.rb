#!/bin/env ruby

require 'ruby-progressbar'
require 'zip'

NOW=Time.now
BRANCH='master'
ROOTDIR=File.expand_path(File.dirname(__FILE__))
TARNAME=File.basename(ROOTDIR)
OUTPUTNAME=File.join(ROOTDIR, 
    "#{TARNAME}_#{BRANCH}_%04d-%02d-%02d_%02d-%02d-%02d.zip" % [ 
    NOW.year, NOW.month, NOW.day, NOW.hour, NOW.min, NOW.sec
    ] )
    
Dir.chdir(ROOTDIR)

puts "Create backup tarball in '#{OUTPUTNAME}' ..."

def fetch_git_files(branch)
    files = `git ls-tree -r --name-only #{branch}`.split("\n")
    files.delete_if do |filename|
        false == File.exist?(File.join(ROOTDIR, filename))
    end
    return files
end

input = fetch_git_files(BRANCH)
puts "Found #{input.length} files to backup"

pbar = ProgressBar.create(:title => "Compress", :starting_at => 0, :total => input.length)
Zip::File.open(OUTPUTNAME, Zip::File::CREATE) do |zipfile|
    input.each do |filename|
        #pbar.log filename
        zipfile.add(filename, File.join(ROOTDIR, filename))
        pbar.increment
     end
end

puts "Finished"
