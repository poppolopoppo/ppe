# frozen_string_literal: true

$Build.ppe_external!(:iaca) do
    extra_files!(*%w{
        Public/Intel_IACA.h
        Private/iacaMarks.h })
    isolated_files!('Private/iaca-dummy.cpp')
end
