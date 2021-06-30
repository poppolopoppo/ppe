# frozen_string_literal: true

require_once '../Utils/Log.rb'
require_once '../Utils/OS.rb'

module Build

    $HAL = os_name
    Log.verbose('detected HAL platform = %s', $HAL)

    def self.hal_path()
        return File.join($BuildPath, 'HAL', $HAL.to_s)
    end

end #~ Build

require_once "./#{$HAL}/#{$HAL}.rb"
