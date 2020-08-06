# frozen_string_literal: true

$Build.ppe_module!(:Remoting) do
    public_deps!(
        namespace.Core,
        namespace.RTTI,
        namespace.Serialize,
        namespace.Network,
        namespace.Application )
end
