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
IGNOREFILES = /(stdafx|targetver)\./

class String
    def is_header?() return self =~ /.*\.h$/i end
    def is_source?() return self =~ /.*\.cpp$/i end
end

PROJECT_NAME=ARGV[0]
if PROJECT_NAME.nil? || PROJECT_NAME.empty?
    $stderr.puts "missing PROJECT_NAME = #{PROJECT_NAME}"
    exit 2
end
PROJECT_DIR=Pathname.new(File.join(SOURCEDIR, PROJECT_NAME))
unless Dir.exist?(PROJECT_DIR)
    $stderr.puts "invalid PROJECT_DIR = #{PROJECT_DIR}"
    exit 3
end
puts "PROJECT_NAME = #{PROJECT_NAME}"
puts "PROJECT_DIR = #{PROJECT_DIR}"

HEADERS=[]
SOURCES=[]
OTHERS=[]
FOLDERS=[]
scan_files(PROJECT_DIR, /.*/) do |path|
    next if path =~ IGNOREFILES
    path = Pathname.new(path)
    path = path.relative_path_from(PROJECT_DIR)
    folder = path.dirname.to_s
    FOLDERS << folder unless folder == "." || FOLDERS.include?(folder)
    path = path.to_s
    if path.is_header?
        HEADERS << path
    elsif path.is_source?
        SOURCES << path
    else
        OTHERS << path
    end
end

VERBOSE = true

puts "Found #{HEADERS.length} headers"
puts "Found #{SOURCES.length} sources"
puts "Found #{OTHERS.length} others"
puts "Found #{FOLDERS.length} folders"

HEADERS.each do |header|
    src = File.join(PROJECT_DIR, header)
    dst = File.join(PROJECT_DIR, 'Public', header)
    dir = File.dirname(dst)
    FileUtils.mkdir_p(dir, :verbose => VERBOSE)
    FileUtils.mv(src, dst, :verbose => VERBOSE)
end
SOURCES.each do |header|
    src = File.join(PROJECT_DIR, header)
    dst = File.join(PROJECT_DIR, 'Private', header)
    dir = File.dirname(dst)
    FileUtils.mkdir_p(dir, :verbose => VERBOSE)
    FileUtils.mv(src, dst, :verbose => VERBOSE)
end
FOLDERS.each do |folder|
    dst = File.join(PROJECT_DIR, folder)
    FileUtils.rm_rf(dst, :verbose => VERBOSE)
end
puts "dun <:"