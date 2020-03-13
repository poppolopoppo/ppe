# frozen_string_literal: true

require_once '../Utils/Log.rb'
require_once '../Utils/OS.rb'

module Build

    $HAL = os_name
    Log.verbose('detected HAL platform = %s', $HAL)

end #~ Build

require "./HAL/#{$HAL}/#{$HAL}.rb"
