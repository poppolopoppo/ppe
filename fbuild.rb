#!/bin/env ruby

START_TIME=Time.now

VERSION='2.2'
VERSION_HEADER="; Version = <#{VERSION}>"

def absolute_path(path)
    path = File.absolute_path(path)
    if path =~/^\//
        path = path[1..-1]
        return path.insert(path.index('/'), ':')
    else
        return path
    end
end

SOLUTION_ROOT = absolute_path(File.dirname(__FILE__))
SOLUTION_FBUILDROOT = File.join(SOLUTION_ROOT, 'Build')
SOLUTION_FBUILDCMD = File.join(SOLUTION_FBUILDROOT, 'FBuild.exe')
SOLUTION_PATHFILE = File.join(SOLUTION_FBUILDROOT, '_solution_path.bff')

RUNNING_ON_WINDOWS = (ENV['OS'] == 'Windows_NT')

Dir.chdir(SOLUTION_ROOT)

class FHeader
    attr_reader :filename, :header
    def initialize(filename)
        @filename = filename
        @header = File.exists?(@filename) ? File.read(@filename) : nil
    end
    def match?(header)
        return (@header == header)
    end
    def write(header)
        File.write(@filename, header)
    end
end

def touch_header(filename)
    File.open(filename, 'r') do |f|
        return f.read(VERSION_HEADER.length)
    end
end

class FDependency
    attr_reader :name, :available
    def initialize(name)
        @name = name
    end
    def eval()
        @available = eval_()
        #puts "Evaluate dependency : #{@name} = #{@available ? 'ENABLED' : 'DISABLED'}"
    end
    def export(oss)
        comment = @available ? '' : ';'
        oss.puts "#{comment}#define WITH_#{@name.to_s.upcase}"
    end
private
    def eval_()
        return false
    end
end

class FDirectoryDependency < FDependency
    attr_reader :var, :path
    def initialize(name, var, path)
        super(name)
        @var = var
        @path = path
    end
    def export(oss)
        super(oss)
        comment = @available ? '' : ';'
        oss.puts "#{comment}.#{var} = '#{@path}'"
    end
private
    def eval_()
        return Dir.exists?(@path)
    end
end

class FVisualStudio < FDependency
    attr_reader :toolset, :comntools, :cluid
    def initialize(toolset)
        @toolset = toolset
        super("VISUALSTUDIO_TOOLSET_#{@toolset}")
    end
    def export(oss)
        super(oss)
        if @available
            oss.puts ".VS#{@toolset}CLUID='#{@cluid}'"
            oss.puts ".VS#{@toolset}COMNTOOLS='#{@comntools}'"
        else
            oss.puts ";.VS#{@toolset}CLUID='XXX'"
            oss.puts ";.VS#{@toolset}COMNTOOLS='XXX'"
        end
    end
private
    def eval_()
        envname = "VS#{@toolset}COMNTOOLS"
        @comntools = ENV[envname]
        return false if @comntools.nil?
        binpath = File.join(@comntools, '..', '..', 'VC', 'bin')
        '00'.upto('99') do |id|
            @cluid = "10#{id}" if File.exists?(File.join(binpath, "10#{id}", 'clui.dll'))
        end
        raise "Can't find CLUID version #{@name} !" if @cluid.nil?
        return true
    end
end

class FVisualStudio2017 < FVisualStudio
    attr_reader :versionrc
    def initialize()
        super('141')
    end
    def export(oss)
        super(oss)
        if @available
            oss.puts ".VS141VERSION='#{@versionrc}'"
        else
            oss.puts ";.VS141VERSION='XXX'"
        end
    end
private
    def eval_()
        @comntools = 'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\\'
        msvcpath = File.join(@comntools, '..', '..', 'VC', 'Tools', 'MSVC')
        return false unless Dir.exists?(msvcpath)
        versions = Dir.entries(msvcpath)
        return false if versions.empty?
        @versionrc = versions.sort.last
        binpath = File.join(@comntools, '..', '..', 'VC', 'Tools', 'MSVC', @versionrc, 'bin', 'HostX64')
        return false unless Dir.exists?(binpath)
        '00'.upto('99') do |id|
            @cluid = "10#{id}" if File.exists?(File.join(binpath, 'x64', "10#{id}", 'clui.dll'))
        end
        raise "Can't find CLUID version #{@name} !" if @cluid.nil?
        return true
    end
end

class FWindowsSDK < FDependency
    BASEPATH='C:\Program Files (x86)\Windows Kits\\'
    attr_reader :winver, :version
    def initialize(winver)
        @winver = winver
        super("WINDOWS_SDK#{winver_s}")
    end
    def sdkpath() File.join(BASEPATH, @winver) end
    def winver_s() winver.to_s().gsub('.','') end
    def export(oss)
        super(oss)
        if @available
            oss.puts ".WindowsSDKBasePath#{winver_s}='#{sdkpath}'"
            oss.puts ".WindowsSDKVersion#{winver_s}='#{@version}'"
        else
            oss.puts ";.WindowsSDKBasePath#{winver_s}='XXX'"
            oss.puts ";.WindowsSDKVersion#{winver_s}='XXX'"
        end
    end
private
    def eval_()
        return false unless Dir.exists?(sdkpath)
        libpath = File.join(sdkpath, 'lib')
        return false unless Dir.exists?(libpath)
        sdkversions = Dir.entries(libpath)
        sdkversions.delete_if {|p| !Dir.exists?(File.join(libpath, p, 'um')) }
        raise "Can't find Windows SDK #{@winver} version !" if sdkversions.empty?
        @version = sdkversions.sort.last
        return true
    end
end

DEPENDENCIES= [
    [ 'Visual Studio 2012'      , FVisualStudio.new('110')  ],
    [ 'Visual Studio 2013'      , FVisualStudio.new('120')  ],
    [ 'Visual Studio 2015'      , FVisualStudio.new('140')  ],
    [ 'Visual Studio 2017'      , FVisualStudio2017.new()   ],
    [ 'Windows SDK 8.1'         , FWindowsSDK.new('8.1')    ],
    [ 'Windows SDK 10'          , FWindowsSDK.new('10')     ],
    [ 'LLVM for Windows x86'    , FDirectoryDependency.new('LLVMFORWINDOWS_X86', 'LLVMBasePathX86', 'C:\Program Files (x86)\LLVM') ],
    [ 'LLVM for Windows x64'    , FDirectoryDependency.new('LLVMFORWINDOWS_X64', 'LLVMBasePathX64', 'C:\Program Files\LLVM') ],
]

DEPENDENCIES.each { |(friendly, dep)| dep.eval }

VISUALSTUDIO_DEFINES = %w{
    USE_VISUALSTUDIO_2012
    USE_VISUALSTUDIO_2013
    USE_VISUALSTUDIO_2015
    USE_VISUALSTUDIO_2017
}

latestVisual = nil
VISUALSTUDIO_DEFINES.length.times do |i|
    latestVisual = i if DEPENDENCIES[i][1].available
end

newHeader = StringIO.new()
newHeader.puts VERSION_HEADER
newHeader.puts "; File generated by fbuild.rb"
newHeader.puts
newHeader.puts ".SolutionPath = '#{SOLUTION_ROOT}'"
newHeader.puts
DEPENDENCIES.each do |(friendly, dep)|
    newHeader.puts "; Depends on #{friendly} : #{dep.available ? 'ENABLED' : 'DISABLED'}"
    dep.export(newHeader)
    newHeader.puts
end

newHeader.puts '; Lastest Visual Studio version available :'
VISUALSTUDIO_DEFINES.each_with_index do |define, i|
    comment = latestVisual == i ? '' : ';'
    newHeader.puts "#{comment}#define #{define}"
end
newHeader.puts
newHeader = newHeader.string

oldHeader = FHeader.new(SOLUTION_PATHFILE)
unless oldHeader.match?(newHeader)
    puts "Regen solution bff : #{SOLUTION_PATHFILE}"
    oldHeader.write(newHeader)
end

SOURCE_FILES_PATTERN=/\.cpp$/
MODIFIED_FILES_SRC=File.join(SOLUTION_ROOT, 'Source')
MODIFIED_FILES_LIST=File.join(SOLUTION_ROOT, 'Build', '.modified_files')
def git_modified_files()
    outp = `git status --porcelain=v1 -u normal "#{MODIFIED_FILES_SRC}"`
    return outp.split("\n").collect! do |l|
        l.chomp!
        fname = l[3..-1]
        next unless fname =~ SOURCE_FILES_PATTERN
        fname = File.join(SOLUTION_ROOT, fname)
        fname.gsub!('/', '\\') if RUNNING_ON_WINDOWS
        fname
    end
    .delete_if {|fname| fname.nil? || fname.length == 0 || !File.exists?(fname) }
    .sort!
end

File.open(MODIFIED_FILES_LIST, 'w') do |f|
    modifieds = git_modified_files()
    if modifieds.length > 0
        puts "#{modifieds.length} modified source files according to git"
        modifieds.each do |fname|
            f.puts(fname)
        end
    end
end

END_TIME=Time.now
$stdout.puts("Spent %5.3f milliseconds in #{__FILE__}" % [(END_TIME - START_TIME)*1000])
$stdout.flush

exec(SOLUTION_FBUILDCMD, *ARGV)
