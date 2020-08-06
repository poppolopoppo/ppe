# frozen_string_literal: true

$Build.namespace(:Runtime) do
    include!('Core/Core.rb')
    include!('VFS/VFS.rb')
    include!('RHI/RHI.rb')
    include!('RTTI/RTTI.rb')
    include!('Serialize/Serialize.rb')
    include!('Network/Network.rb')
    include!('Application/Application.rb')
    include!('Remoting/Remoting.rb')
end
