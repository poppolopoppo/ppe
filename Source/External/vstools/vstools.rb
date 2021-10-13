# frozen_string_literal: true

$Build.ppe_external!(:vstools) do
    tag!(:nosymbols)
    link!(:dynamic)
    extra_files!('Public/vstools.h')
    source_files!('Private/vstools.cpp')
    custom!() do |env, target|
        case env.platform.os
        when :Windows
            linkerOptions <<
                '/LTCG:INCREMENTAL' <<
                '/DEBUG:NONE' <<
                '/IGNORE:4099' <<
                '/WX:NO' # << '/NODEFAULTLIB:MSVCRT.lib'
        end
    end
end
