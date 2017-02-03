#!/bin/env ruby

VERSION='2.0'
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

Dir.chdir(SOLUTION_ROOT)

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
        puts "Evaluate dependency : #{@name} = #{@available ? 'ENABLED' : 'DISABLED'}"
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

class FVisualStudio2017RC < FVisualStudio
    attr_reader :versionrc
    def initialize()
        super('141')
    end
    def export(oss)
        super(oss)
        if @available
            oss.puts ".VS141VERSIONRC='#{@versionrc}'"
        else
            oss.puts ";.VS141VERSIONRC='XXX'"
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
        raise "Can't find Windows SDK #{@winver} version !" if sdkversions.empty?
        @version = sdkversions.sort.last
        return true
    end
end

regen=false
if not File.readable_real?(SOLUTION_PATHFILE)
    puts "First run, '#{SOLUTION_PATHFILE}' does not exists."
    regen = true
elsif touch_header(SOLUTION_PATHFILE) != VERSION_HEADER
    puts "Version not matching in '#{SOLUTION_PATHFILE}'."
    regen = true
end

if regen
    puts "Generating '#{SOLUTION_PATHFILE}' for FBuild..."
    puts "Solution root = '#{SOLUTION_ROOT}'"

    DEPENDENCIES= [
        [ 'Visual Studio 2012'      , FVisualStudio.new('110')  ],
        [ 'Visual Studio 2013'      , FVisualStudio.new('120')  ],
        [ 'Visual Studio 2015'      , FVisualStudio.new('140')  ],
        [ 'Visual Studio 2017 RC'   , FVisualStudio2017RC.new() ],
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

    File.open(SOLUTION_PATHFILE, 'w') do |f|
        f.puts VERSION_HEADER
        f.puts "; File generated by fbuild.rb (#{Time.now})."
        f.puts
        f.puts ".SolutionPath = '#{SOLUTION_ROOT}'"
        f.puts
        DEPENDENCIES.each do |(friendly, dep)|
            f.puts "; Depends on #{friendly} : #{dep.available ? 'ENABLED' : 'DISABLED'}"
            dep.export(f)
            f.puts
        end

        f.puts '; Lastest Visual Studio version available :'
        VISUALSTUDIO_DEFINES.each_with_index do |define, i|
            comment = latestVisual == i ? '' : ';'
            f.puts "#{comment}#define #{define}"
        end

        f.puts
    end

    puts "Done writing '#{SOLUTION_PATHFILE}'."
end

exec(SOLUTION_FBUILDCMD, *ARGV)
