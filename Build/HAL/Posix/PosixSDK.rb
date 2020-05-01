# frozen_string_literal: true

require_once '../../Common.rb'
require_once '../../Utils/Prerequisite.rb'

module Build

    make_facet(:PosixSDK_Base) do
        compilerOptions << '-pthread' # need libthread for all threading
    end

    make_facet(:PosixSDK_X86) do
        self << Build.PosixSDK_Base
    end
    make_facet(:PosixSDK_X64) do
        self << Build.PosixSDK_Base
    end

end #~ Build