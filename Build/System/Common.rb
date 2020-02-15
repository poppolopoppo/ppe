
require './Utils/Log.rb'

module Build

    VERSION = '1.0.0'

    def self.set_workspace_path(path)
        path = File.absolute_path(path)
        Log.info("set workspace root   = '%s'", path)

        $WorkspacePath = path

        $DataPath = File.join($WorkspacePath, 'Data')
        $BuildPath = File.join($WorkspacePath, 'Build')
        $SourcePath = File.join($WorkspacePath, 'Source')
        $OutputPath = File.join($WorkspacePath, 'Output')
        $BinariesPath = File.join($OutputPath, 'Binaries')
        $IntermediatePath = File.join($OutputPath, 'Intermediate')
        $SavedPath = File.join($OutputPath, 'Saved')

        Log.debug("Data path            = '%s'", $DataPath)
        Log.debug("Build path           = '%s'", $BuildPath)
        Log.debug("Source path          = '%s'", $SourcePath)
        Log.debug("Output path          = '%s'", $OutputPath)
        Log.debug("Binaries path        = '%s'", $BinariesPath)
        Log.debug("Intermediate path    = '%s'", $IntermediatePath)
        Log.debug("Saved path           = '%s'", $SavedPath)
    end

    Build.set_workspace_path(File.dirname($0)) # create global variables

    def self.const_memoize(host, name, &memoized)
        cls = host.class
        ivar = ('@'<<name.to_s).to_sym
        cls.instance_variable_set(ivar, nil)
        cls.define_method(name) do
            value = cls.instance_variable_get(ivar)
            if value.nil?
                value = memoized.call
                cls.instance_variable_set(ivar, value)
            end
            return value
        end
    end

end #~ Build
