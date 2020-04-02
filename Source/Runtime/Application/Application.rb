# frozen_string_literal: true

$Build.ppe_module!(:Application) do
    public_deps!(namespace.Core)
    custom!() do |env, target|
        case env.platform.os
        when :Linux
            compilerOptions << '-lsdl'
        end
    end
end
