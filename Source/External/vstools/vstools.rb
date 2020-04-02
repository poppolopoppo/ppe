# frozen_string_literal: true

$Build.ppe_external!(:vstools) do
    link!(:dynamic)
    extra_files!('Public/vstools.h')
    isolated_files!('Private/vstools.cpp')
    custom!() do |env, target|
        case env.platform.os
        when :Windows
            linkerOptions << '/IGNORE:4099' << '/NODEFAULTLIB:MSVCRT.lib'
        end
    end
end
