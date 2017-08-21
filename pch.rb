#!/bin/env ruby

require 'benchmark'
require 'find'

REBUILD = !ARGV.collect{|s| s.downcase }.include?('-norebuild')

# TODO : parse COMPILE_CONFIGS from build output
PLATFORMS = %w{Win32 Win64}
CONFIGS = %w{Debug FastDebug Release Profiling Final}
COMPILE_CONFIGS = []
PLATFORMS.each {|p| CONFIGS.each{|c| COMPILE_CONFIGS << "#{p}-#{c}" }}

ROOTDIR = File.dirname(__FILE__)
BUILDDIR = File.absolute_path(File.join(ROOTDIR, 'Build'))
SOURCEDIR = File.absolute_path(File.join(ROOTDIR, 'Source'))
INTERMEDIATEDIR = File.absolute_path(File.join(ROOTDIR, 'Output', 'Intermediate'))
FBUILDCMD = File.absolute_path(File.join(ROOTDIR, 'fbuild.rb'))
PCHREFACTORCMD = File.absolute_path(File.join(ROOTDIR, 'Build', 'PCHRefactor.rb'))

ISOLATE_UNITY_OFF = ';#define DISABLE_UNITY_BUILDS ; %_NOCOMMIT%'
ISOLATE_UNITY_ON  = '#define DISABLE_UNITY_BUILDS ; %NOCOMMIT%'

def patch_bff(filename, src, dst)
    content = File.read(filename)
    content.gsub!(src, dst)
    File.write(filename, content)
end

Benchmark.bm(32) do |bm|

$projects = []

# find every stdafx.generated.h in Source/
bm.report('find all stdafx.generated.h') do
Find.find(SOURCEDIR) do |path|
    if FileTest.directory?(path)
        if File.basename(path)[0] == ?.
            Find.prune       # Don't look any further into this directory.
        else
            next
        end
    elsif File.basename(path) == "stdafx.generated.h"
        project_name = File.dirname(path).split(/\\|\//).last
        $projects << project_name
    end
end
end

# regen every stdafx.generated.h to empty files
if REBUILD
bm.report('clear previously generated files') do
$projects.each do |project_name|
    stdafx = File.join(SOURCEDIR, project_name, "stdafx.generated.h")
    system("ruby", PCHREFACTORCMD, stdafx, project_name)
end
end
end

# patch Common.bff to isolate Unity files
if REBUILD
bm.report('patch BFF to isolate unity files') do
    patch_bff(File.join(BUILDDIR, 'Common.bff'), ISOLATE_UNITY_OFF, ISOLATE_UNITY_ON)
end
end

# rebuild all projects
if REBUILD
bm.report('rebuild all projects') do
system("ruby", FBUILDCMD, '-clean', '-preprocessonly', *$projects)
end
end

# regen every stdafx.generated.h with new dependencies
bm.report('update stdafx.generated.h') do
$projects.each do |project_name|
    stdafx = File.join(SOURCEDIR, project_name, "stdafx.generated.h")
    system("ruby", PCHREFACTORCMD, stdafx, project_name, *COMPILE_CONFIGS)
end
end

# patch Common.bff to not isolate Unity files
if REBUILD
bm.report('revert BFF') do
    patch_bff(File.join(BUILDDIR, 'Common.bff'), ISOLATE_UNITY_ON, ISOLATE_UNITY_OFF)
end
end

end
