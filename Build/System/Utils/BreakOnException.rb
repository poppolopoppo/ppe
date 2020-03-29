
require_once 'Log.rb'

Build::Log.warning('Hooking exceptions for debug')

class Exception
    alias original_initalize initialize
    def initialize(*args)
        original_initalize(*args)
    end
end
