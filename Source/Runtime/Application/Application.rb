# frozen_string_literal: true

$Build.ppe_module!(:Application) do
    public_deps!(
        namespace.Core,
        namespace.RHI )
    extra_files!('Public/HAL/Windows/resource.rc') if Build.os_windows?
    custom!() do |env, target|
        case env.platform.os
        when :Linux
            compilerOptions << '-lglfw3'
        end
    end
end
