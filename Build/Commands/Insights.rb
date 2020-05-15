# frozen_string_literal: true

require_once '../Common.rb'

require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

if Build.os_windows?
require_once '../HAL/Windows/Windows.rb'
end

require 'fileutils'

module Build

    make_prerequisite(:vcperf) do
        vcperf = nil
        if Build.os_windows?
            compiler = Build.os_x64? ? Build.WindowsCompiler_X64 : Build.WindowsCompiler_X86
            vcperf = need_fileset!(File.join(compiler.expand?('$VisualStudioTools$'), 'vcperf.exe'))
        end
        vcperf
    end

    make_command(:'insights', 'Capture compilation insights') do |&namespace|
        if vcperf = Build.vcperf
            at_exit do |&ns|
                tracePath = File.join($SavedPath, 'Insights')
                FileUtils.mkdir_p(tracePath, :verbose => Log.verbose?)
                Insights.vcperf_stop File.join(tracePath, "#{Build::Project}_#{Time.now.to_i}.etl")
            end if Insights.vcperf_start.success?
        end
    end #~insights

    module Insights

        SESSION_NAME = "Build-#{Build::Project}"

        def self.vcperf_start()
            vcperf = Build.vcperf
            Assert.expect(vcperf, String)
            Log.verbose 'Insights: start vcperf <%s> session', SESSION_NAME
            Build.run_as(vcperf, '/start', SESSION_NAME)
        end

        def self.vcperf_stop(filename)
            vcperf = Build.vcperf
            Assert.expect(vcperf, String)
            Log.verbose 'Insights: stop vcperf <%s> session', SESSION_NAME
            Build.run_as(vcperf, '/stop', SESSION_NAME, filename)
            Log.info 'Insights: ETL trace <%s> dumped in "%s"', SESSION_NAME, filename
        end

    end #~ Insights

end #~ Build
