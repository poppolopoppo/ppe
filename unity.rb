#!/bin/env ruby

require 'find'

SOURCESFILES_PER_UNITY = 20

ROOTDIR = File.dirname(__FILE__)
SOURCEDIR = File.absolute_path(File.join(ROOTDIR, 'Source'))

def patch_bff(filename, src, dst)
    content = File.read(filename)
    content.gsub!(src, dst)
    File.write(filename, content)
end

RE_SOURCEFILE = /\.((cpp)|(c))$/i
def sourcefiles_count(rootpath)
    count = 0
    Find.find(rootpath) do |path|
        if FileTest.directory?(path)
            if File.basename(path)[0] == ?.
                Find.prune       # Don't look any further into this directory.
            else
                next
            end
        elsif path =~ RE_SOURCEFILE
            count += 1
        end
    end
    return count
end

Dir.entries(SOURCEDIR).each do |project_name|
    path = File.join(SOURCEDIR, project_name)
    bff = File.join(path, "#{project_name}.bff")
    next unless File.exists?(bff)
    puts "Processing #{project_name}..."
    src_count = sourcefiles_count(path)
    unity_count = (src_count + SOURCESFILES_PER_UNITY - 1) / SOURCESFILES_PER_UNITY
    puts "Found #{src_count} source files => #{unity_count} unity"
    patch_bff(bff, /\.UnityNumFiles\s*=\s*(\d+)/, ".UnityNumFiles = #{unity_count}")
    puts "Written to #{bff}"
end
