# frozen_string_literal: true

$Build.ppe_headers!(:renderdoc) do
    extra_files!(*%w{
        Public/renderdoc-external.h
        renderdoc.git/renderdoc/api/app/renderdoc.h })
    # header-only library
end
