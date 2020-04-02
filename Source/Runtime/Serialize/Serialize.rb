# frozen_string_literal: true

$Build.ppe_module!(:Serialize) do
    public_deps!(
        namespace.Core,
        namespace.VFS,
        namespace.RTTI )
    excluded_files!('Private/Markup/MarkupSerializer.cpp') # TODO : still need some work, but is it really needed ?
end
