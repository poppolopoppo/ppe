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

NAMESPACE_RE=/namespace Core/

scan_files(SOURCEDIR, /.*\.(h|cpp)$/i) do |path|
    next if path =~ IGNOREFILES
    content = File.read(path)
    content.gsub!(NAMESPACE_RE, 'namespace PPE')
    content.gsub!('"Core.Application/', '"')
    content.gsub!('"Core.RTTI/', '"')
    content.gsub!('"Core.Serialize/', '"')
    content.gsub!('"Core.Graphics/', '"')
    content.gsub!('"Core.Network/', '"')
    content.gsub!('"Core.Pixmap/', '"')
    content.gsub!('"Core.Lattice/', '"')
    content.gsub!('"Core/IO/VFS/', '"')
    content.gsub!('"Core/', '"')
    content.gsub!('CORE', 'PPE')
    File.write(path, content)
    puts path
end
