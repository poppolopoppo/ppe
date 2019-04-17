#!/bin/env ruby

require 'fileutils'
require 'pathname'

START_TIME=Time.now

VERSION='2.5'
VERSION_HEADER="// VERSION = <#{VERSION}>"

require 'rbconfig'
RUBY_INTERPRETER_PATH = Pathname.new(File.join(
    RbConfig::CONFIG["bindir"],
    RbConfig::CONFIG["RUBY_INSTALL_NAME"] +
    RbConfig::CONFIG["EXEEXT"] )).realpath.to_s

SOLUTION_ROOT = Pathname.new(File.dirname(__FILE__)).realpath.to_s
SOLUTION_FBUILDROOT = File.join(SOLUTION_ROOT, 'Build')
SOLUTION_FBUILDCMD = File.join(SOLUTION_FBUILDROOT, 'FBuild.exe')
SOLUTION_PATHFILE = File.join(SOLUTION_FBUILDROOT, '_solution_path.bff')

RUNNING_ON_WINDOWS = (ENV['OS'] == 'Windows_NT')

Dir.chdir(SOLUTION_ROOT)

class FHeader
    attr_reader :filename, :header
    def initialize(filename)
        @filename = filename
        @header = nil
    end
    def match?(other)
        return (@filename == other.filename && @header == other.header)
    end
    def read?()
        @header = File.exist?(@filename) ? File.read(@filename) : nil
    end
    def write!()
        filename_old = @filename + '.old'
        FileUtils.mv(@filename, filename_old) if File.exist?(@filename)
        File.write(@filename, header)
    end

    def print(txt)
        @header = '' if @header.nil?
        @header += txt
        return self
    end
    def puts(txt='')
        self.print(txt + "\n")
    end
    def comment(txt)
        self.print("\n// #{txt}\n")
    end
    def define(name, enabled=true)
        self.print("#{enabled ? '#' : ';'}define WITH_#{name.to_s.upcase}\n")
    end
    def set(name, value)
        if value
            self.print(".#{name} = '#{value.gsub('\\', '/')}'\n")
        else
            self.print(";#{name} = 'XXX'\n")
        end
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
    def export(header)
        header.define(@name, @available)
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
    def export(header)
        super(header)
        header.set(var, @available ? @path : nil)
    end
private
    def eval_()
        return Dir.exist?(@path)
    end
end

class FVisualStudio < FDependency
    attr_reader :toolset, :default_comntools, :comntools, :cluid
    def initialize(toolset, default_comntools)
        @toolset = toolset
        @default_comntools = default_comntools
        super("VISUALSTUDIO_TOOLSET_#{@toolset}")
    end
    def export(header)
        super(header)
        header.set("VS#{@toolset}CLUID", @available ? @cluid : nil)
        header.set("VS#{@toolset}COMNTOOLS", @available ? @comntools : nil)
    end
private
    def eval_comntools_()
        return false if @comntools.nil?
        binpath = File.join(@comntools, '..', '..', 'VC', 'bin')
        '00'.upto('99') do |id|
            @cluid = "10#{id}" if File.exist?(File.join(binpath, "10#{id}", 'clui.dll'))
        end
        return !@cluid.nil?
    end
    def eval_()
        envname = "VS#{@toolset}COMNTOOLS"
        @comntools = ENV[envname]
        return true if eval_comntools_()
        @comntools = @default_comntools
        success = eval_comntools_()
        ENV[envname] = @comntools if success
        return success
    end
end

class FVisualStudioPost2015 < FVisualStudio
    attr_reader :version
    def initialize(major_version, toolset)
        root = 'C:\Program Files (x86)\Microsoft Visual Studio\\' + major_version
        comntools_communuty = root + '\Community\Common7\Tools\\'
        comntools_professional = root + '\Professional\Common7\Tools\\'
        super(toolset, Dir.exist?(comntools_professional) ?
            comntools_professional :
            comntools_communuty )
    end
    def export(header)
        super(header)
        header.set("VS#{@toolset}VERSION", @available ? @version : nil)
    end
private
    def eval_()
        @comntools = @default_comntools
        msvcpath = File.join(@comntools, '..', '..', 'VC', 'Tools', 'MSVC')
        return false unless Dir.exist?(msvcpath)
        versions = Dir.entries(msvcpath)
        return false if versions.empty?
        @version = versions.sort.last
        binpath = File.join(@comntools, '..', '..', 'VC', 'Tools', 'MSVC', @version, 'bin', 'HostX64')
        return false unless Dir.exist?(binpath)
        '00'.upto('99') do |id|
            @cluid = "10#{id}" if File.exist?(File.join(binpath, 'x64', "10#{id}", 'clui.dll'))
        end
        return !@cluid.nil?
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
    def export(header)
        super(header)
        header.set("WindowsSDKBasePath#{winver_s}", @available ? sdkpath : nil)
        header.set("WindowsSDKVersion#{winver_s}", @available ? @version : nil)
    end
private
    def eval_()
        return false unless Dir.exist?(sdkpath)
        libpath = File.join(sdkpath, 'lib')
        return false unless Dir.exist?(libpath)
        sdkversions = Dir.entries(libpath)
        sdkversions.delete_if {|p| !Dir.exist?(File.join(libpath, p, 'um')) }
        raise "Can't find Windows SDK #{@winver} version !" if sdkversions.empty?
        @version = sdkversions.sort.last
        return true
    end
end

class FLLVMWindows < FDependency
    BASEPATH_X86 = 'C:\Program Files (x86)\LLVM'
    BASEPATH_X64 = 'C:\Program Files\LLVM'

    attr_reader :platform, :llvm_path, :clang_version
    def initialize()
        super('LLVM_WINDOWS')
    end
    def export(header)
        super(header)
        header.set("LLVMBasePath#{@platform}", @available ? @llvm_path : nil)
        header.set("LLVMClangVer#{@platform}", @available ? @clang_version : nil)
        header.set("LLVMWindowsBasePath", @available ? @llvm_path : nil)
        header.set("LLVMWindowsClangVer", @available ? @clang_version : nil)
    end
    def self.fetch_version?(llvm_path)
        clang_lib_path = File.join(llvm_path, 'lib', 'clang')
        entries = Dir.entries(clang_lib_path)
        if entries
            entries.delete_if{|x| x =~ /^\.+$/}
            entries.sort!
            return entries[0]
        else
            return nil
        end
    end
private
    def eval_()
        if Dir.exist?(BASEPATH_X64)
            @platform = 'X64'
            @llvm_path = BASEPATH_X64
        elsif Dir.exist?(BASEPATH_X86)
            @platform = 'X86'
            @llvm_path = BASEPATH_X86
        else
            @platform = nil
            @llvm_path = nil
            return false
        end
        @clang_version = FLLVMWindows.fetch_version?(@llvm_path)
        return (@clang_version != nil)
    end
end

DEPENDENCIES= [
    [ 'Visual Studio 2012'      , FVisualStudio.new('110', 'C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\\')  ],
    [ 'Visual Studio 2013'      , FVisualStudio.new('120', 'C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\\')  ],
    [ 'Visual Studio 2015'      , FVisualStudio.new('140', 'C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\\')  ],
    [ 'Visual Studio 2017'      , FVisualStudioPost2015.new('2017', 141) ],
    [ 'Visual Studio 2019'      , FVisualStudioPost2015.new('2019', 142) ],

    [ 'Windows SDK 8.1'         , FWindowsSDK.new('8.1')    ],
    [ 'Windows SDK 10'          , FWindowsSDK.new('10')     ],

    [ 'LLVM for Windows'        , FLLVMWindows.new() ],
]

DEPENDENCIES.each { |it| it[1].eval }

VISUALSTUDIO_DEFINES = %w{
    USE_VISUALSTUDIO_2012
    USE_VISUALSTUDIO_2013
    USE_VISUALSTUDIO_2015
    USE_VISUALSTUDIO_2017
    USE_VISUALSTUDIO_2019
}

latestVisual = nil
VISUALSTUDIO_DEFINES.length.times do |i|
    latestVisual = i if DEPENDENCIES[i][1].available
end

newHeader = FHeader.new(SOLUTION_PATHFILE)
newHeader.puts VERSION_HEADER
newHeader.comment("File generated by fbuild.rb")
newHeader.set("SolutionPath", SOLUTION_ROOT)
newHeader.set("RubyInterpreterPath", RUBY_INTERPRETER_PATH)

DEPENDENCIES.each do |(friendly, dep)|
    newHeader.comment("Depends on #{friendly} : #{dep.available ? 'ENABLED' : 'DISABLED'}")
    dep.export(newHeader)
end

newHeader.comment('Latest Visual Studio version available :')
VISUALSTUDIO_DEFINES.each_with_index do |define, i|
    comment = latestVisual == i ? '' : ';'
    newHeader.puts "#{comment}#define #{define}"
end

oldHeader = FHeader.new(SOLUTION_PATHFILE)
oldHeader.read?()

unless oldHeader.match?(newHeader)
    puts "Regen solution bff : #{SOLUTION_PATHFILE}"
    newHeader.write!()
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
    .delete_if {|fname| fname.nil? || fname.length == 0 || !File.exist?(fname) }
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
$stdout.puts("#{SOLUTION_FBUILDCMD} #{ARGV.join(' ')}")
$stdout.flush
unless exec(SOLUTION_FBUILDCMD, *ARGV)
    $stderr.puts "failed to start fastbuild, command line:"
    $stderr.puts([SOLUTION_FBUILDCMD, *ARGV].join(' '))
    exit 5
end
