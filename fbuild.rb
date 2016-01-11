#!/bin/env ruby

SOLUTION_ROOT = File.dirname(__FILE__)
SOLUTION_FBUILDROOT = File.join(SOLUTION_ROOT, 'Build')
SOLUTION_FBUILDCMD = File.join(SOLUTION_FBUILDROOT, 'FBuild.exe')
SOLUTION_PATHFILE = File.join(SOLUTION_FBUILDROOT, '_solution_path.bff')

Dir.chdir(SOLUTION_ROOT)

unless File.exist?(SOLUTION_PATHFILE)
    puts "Creating '#{SOLUTION_PATHFILE}' ..."

    VS110COMNTOOLS=ENV['VS110COMNTOOLS']
    VS120COMNTOOLS=ENV['VS120COMNTOOLS']
    VS140COMNTOOLS=ENV['VS140COMNTOOLS']

    vs110cluid = nil
    vs120cluid = nil
    vs140cluid = nil
    '00'.upto('99') do |id|
        vs110cluid = "10#{id}" if File.exists?("#{VS110COMNTOOLS}..\..\VC\bin\10#{id}\clui.dll")
        vs120cluid = "10#{id}" if File.exists?("#{VS120COMNTOOLS}..\..\VC\bin\10#{id}\clui.dll")
        vs140cluid = "10#{id}" if File.exists?("#{VS140COMNTOOLS}..\..\VC\bin\10#{id}\clui.dll")
    end

    File.open(SOLUTION_PATHFILE, 'w') do |f|
        f.puts "; Fichier généré par fbuild (#{Time.now})."
        f.puts ".SolutionPath   = '#{SOLUTION_ROOT}'"
        f.puts ".VS110CLUID     = '#{vs110cluid}'"
        f.puts ".VS120CLUID     = '#{vs120cluid}'"
        f.puts ".VS140CLUID     = '#{vs140cluid}'"
    end
end

exec(SOLUTION_FBUILDCMD, *ARGV)
