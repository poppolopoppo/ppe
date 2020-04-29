
require 'fileutils'
require 'pathname'
require 'tempfile'

module Dumpbin
    VSVERSION=140
    VSCOMMONTOOLS=ENV["VS#{VSVERSION}COMNTOOLS"]
    VSVCBIN=Pathname.new(File.join(VSCOMMONTOOLS, '..', '..', 'VC', 'BIN')).realpath
    VSDUMPBIN=File.join(VSVCBIN, 'dumpbin.exe')

    def self.from_obj(flags, obj, output=nil)
        unless output
            tmpfile=Tempfile.new('vs_dumpbin_in_obj')
            output=tmpfile.path
            tmpfile.close
        end
        system(VSDUMPBIN, '/NOLOGO', *flags, obj, "/OUT:#{output}")
        return output
    end

    def self.from_dir(flags, path, ext='.obj', output=nil)
        unless output
            tmpfile=Tempfile.new('vs_dumpbin_in_dir')
            output=tmpfile.path
            tmpfile.close
        else
            FileUtils.rm_f(output, :verbose => true)
        end
        path.gsub!('\\', '/')
        Dir.glob("#{path}/**/*#{ext}").each do |filename|
            puts "#{output}: #{filename} (#{flags})"
            tmp = Dumpbin.from_obj(flags, filename)
            File.open(output, 'a') {|f| f.puts File.read(tmp) }
        end
        return output
    end

    def self.headers_in_obj(obj, output=nil) Dumpbin.from_obj('/HEADERS', obj, output) end
    def self.headers_in_dir(path, ext='.obj', output=nil) Dumpbin.from_dir('/HEADERS', path, ext, output) end

    def self.symbols_in_obj(obj, output=nil) Dumpbin.from_obj('/SYMBOLS', obj, output) end
    def self.symbols_in_dir(path, ext='.obj', output=nil) Dumpbin.from_dir('/SYMBOLS', path, ext, output) end

end #~ Dumpbin

PLATFORM='x64'
CONFIG='Debug'
ROOTDIR="D:\\code\\core\\"
INTERMEDIATEDIR="#{ROOTDIR}output\\Intermediate\\#{PLATFORM}\\#{CONFIG}\\"

#Dumpbin.headers_in_dir(INTERMEDIATEDIR+'Core.RTTI', '.obj', INTERMEDIATEDIR+comdat_rtti.txt)
#Dumpbin.headers_in_dir(INTERMEDIATEDIR+'Core.ContentGenerator', '.obj', INTERMEDIATEDIR+'comdat_contentgenerator.txt')
Dumpbin.headers_in_dir(INTERMEDIATEDIR, '.obj', INTERMEDIATEDIR+'comdat.txt')
