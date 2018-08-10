#!/bin/env ruby

require 'fileutils'
require 'find'
require 'pathname'
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

SOURCEDIR = File.absolute_path(File.join(File.dirname(__FILE__), "../Source/"))
IGNOREFILES = /(stdafx|targetver|Engine_old)/

class String
    def is_header?() return self =~ /.*\.h$/i end
    def is_source?() return self =~ /.*\.cpp$/i end
end

INCLUDE_RE=/#(\s*)include(\s+)"(?!stdafx)([a-z0-9_\.-]+)"/i

PROJECTDIR = File.join(SOURCEDIR, ARGV[0])
unless Dir.exist?(PROJECTDIR)
    $stderr.puts "usage: fix_local_includes.rb <PROJECTDIR>"
    exit 2
end
puts "scaning #{PROJECTDIR} ..."

PUBLICDIR = File.join(PROJECTDIR, 'Public/')
PRIVATEDIR = File.join(PROJECTDIR, 'Private/')

scan_files(PRIVATEDIR, /.*\.cpp$/i) do |src|
    next if src =~ IGNOREFILES
    content = File.read(src)
    relpath = File.dirname(src)[PRIVATEDIR.length..-1]
    content.gsub!(INCLUDE_RE, "#\\1include\\2\"#{relpath}/\\3\"")
    File.write(src, content)
    puts src
end
