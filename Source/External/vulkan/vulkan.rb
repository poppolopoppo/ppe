# frozen_string_literal: true

require 'set'

$Build.ppe_external!(:vulkan) do
    extra_files!(*%w{
        Public/vulkan-exports.h
        Public/vulkan-exports.inl
        Public/vulkan-external.h
        Public/vulkan-fwd.h
        Public/vulkan-platform.h
    })
    source_files!(*%w{
        Private/vulkan.cpp
    })
    generate!('vulkan-exports.generated.h', :public) do |facet, env, io|
        vk_headers = Build.vk_system_headers(env, self)
        exports_inl = source_path(env, 'Public/vulkan-exports.inl')
        Build.vk_generate_header(exports_inl, env, io, *vk_headers)
    end
    generate!('vulkan-exports.generated.cpp', :private) do |facet, env, io|
        vk_headers = Build.vk_system_headers(env, self)
        exports_inl = source_path(env, 'Public/vulkan-exports.inl')
        Build.vk_generate_source(exports_inl, env, io, *vk_headers)
    end
end
