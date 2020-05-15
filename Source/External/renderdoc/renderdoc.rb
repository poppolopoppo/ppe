# frozen_string_literal: true

$Build.ppe_headers!(:renderdoc) do
    extra_files!(*%w{
        Public/renderdoc-external.h
        Public/renderdoc_app.h })
    # header-only library
end
