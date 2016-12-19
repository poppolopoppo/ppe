#!/bin/env ruby

VERSION='1.1'
VERSION_HEADER="; Version = <#{VERSION}>"

SOLUTION_ROOT = File.absolute_path(File.dirname(__FILE__))
SOLUTION_FBUILDROOT = File.join(SOLUTION_ROOT, 'Build')
SOLUTION_FBUILDCMD = File.join(SOLUTION_FBUILDROOT, 'FBuild.exe')
SOLUTION_PATHFILE = File.join(SOLUTION_FBUILDROOT, '_solution_path.bff')

Dir.chdir(SOLUTION_ROOT)

def touch_header(filename)
    File.open(filename, 'r') do |f|
        return f.read(VERSION_HEADER.length)
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

    VS110COMNTOOLS=ENV['VS110COMNTOOLS']
    VS120COMNTOOLS=ENV['VS120COMNTOOLS']
    VS140COMNTOOLS=ENV['VS140COMNTOOLS']
    VS141COMNTOOLS=ENV['VS141COMNTOOLS'] ? ENV['VS141COMNTOOLS'] : 'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\\'

    VS110BIN = File.join(VS110COMNTOOLS, '..', '..', 'VC', 'bin')
    VS120BIN = File.join(VS120COMNTOOLS, '..', '..', 'VC', 'bin')
    VS140BIN = File.join(VS140COMNTOOLS, '..', '..', 'VC', 'bin')
    VS141BIN = File.join(VS141COMNTOOLS, '..', '..', 'VC', 'Tools', 'MSVC', '14.10.24728', 'bin', 'HostX64')

    vs110cluid = nil
    vs120cluid = nil
    vs140cluid = nil
    vs141cluid = nil
    '00'.upto('99') do |id|
        vs110cluid = "10#{id}" if File.exists?(File.join(VS110BIN, "10#{id}", 'clui.dll'))
        vs120cluid = "10#{id}" if File.exists?(File.join(VS120BIN, "10#{id}", 'clui.dll'))
        vs140cluid = "10#{id}" if File.exists?(File.join(VS140BIN, "10#{id}", 'clui.dll'))
        vs141cluid = "10#{id}" if File.exists?(File.join(VS141BIN, 'x64', "10#{id}", 'clui.dll'))
    end

    vs110cluid = 'VS110 not found' if vs110cluid.nil?
    vs120cluid = 'VS120 not found' if vs120cluid.nil?
    vs140cluid = 'VS140 not found' if vs140cluid.nil?
    vs141cluid = 'VS141 not found' if vs141cluid.nil?

    llvm_x64 = 'C:\Program Files\LLVM'
    llvm_x86 = 'C:\Program Files (x86)\LLVM'

    File.open(SOLUTION_PATHFILE, 'w') do |f|
        f.puts VERSION_HEADER
        f.puts "; Fichier généré par fbuild (#{Time.now})."
        f.puts
        f.puts ".SolutionPath = '#{SOLUTION_ROOT}'"
        f.puts
        f.puts ".VS110CLUID = '#{vs110cluid}'"
        f.puts ".VS120CLUID = '#{vs120cluid}'"
        f.puts ".VS140CLUID = '#{vs140cluid}'"
        f.puts ".VS141CLUID = '#{vs141cluid}'"
        f.puts
        f.puts ";VS110COMNTOOLS = '#{VS110COMNTOOLS}'"
        f.puts ";VS120COMNTOOLS = '#{VS120COMNTOOLS}'"
        f.puts ";VS140COMNTOOLS = '#{VS140COMNTOOLS}'"
        f.puts ".VS141COMNTOOLS = '#{VS141COMNTOOLS}'"
        f.puts
        f.puts ".LLVMBasePathX86 = '#{llvm_x86}'"
        f.puts ".LLVMBasePathX64 = '#{llvm_x64}'"
    end

    puts "Done writing '#{SOLUTION_PATHFILE}'."
end

exec(SOLUTION_FBUILDCMD, *ARGV)
