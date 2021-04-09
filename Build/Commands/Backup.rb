# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/SourceControl.rb'

require 'fileutils'
require 'rubygems/package'
require 'zlib'

module Build

    make_command(:"backup", 'Generate a snapshot backup archive') do |&namespace|
        archive = File.join($OutputPath, 'Backup', "#{Build::Project}-#{Build.branch?}-#{Build.revision?}-#{Time.now.to_i}.tar.gz")
        fileset = Build.staged_files?

        Log.log('Backup: generating snapshot backup in "%s" with %d files', archive, fileset.length)
        FileUtils.mkdir_p(File.dirname(archive), :verbose => Log.debug?)

        rawSize = 0
        numFiles = 0
        File.open(archive, 'wb') do |fd|
            Zlib::GzipWriter.wrap(fd) do |gzip|
                Gem::Package::TarWriter.new(gzip) do |tar|
                    fileset.each do |fname|
                        next unless File.exists?(fname)
                        content = IO.binread(File.join($WorkspacePath, fname))
                        if Log.verbose?
                            Log.verbose('Backup: append "%s" to archive (%d bytes)', fname, content.length)
                        else
                            Log.pin('Backup: append "%s" to archive (%d bytes)' % [fname, content.length])
                        end
                        tar.add_file_simple(fname, 0644, content.length) do |io|
                            io.write(content)
                        end
                        numFiles += 1
                        rawSize += content.length
                    end
                end
            end
        end

        Log.clear_pin
        Log.log('Backup: saved "%s" archive with %d files weighting %.3fmb (%.3fmb uncompressed)',
                File.basename(archive), numFiles, File.size(archive) / (1024.0 * 1024), rawSize / (1024.0 * 1024))
    end

end #~ Build
