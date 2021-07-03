# frozen_string_literal: true

$Build.ppe_headers!(:vma) do
    extra_files!(*%w{
        Public/vma-external.h
        vma.git/include/vk_mem_alloc.h })
end
