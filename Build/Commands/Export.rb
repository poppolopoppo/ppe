# frozen_string_literal: true



require_once '../Core/Environment.rb'
require_once '../Core/Target.rb'

require_once '../Utils/JSONFile.rb'
require_once '../Utils/Log.rb'
require_once '../Utils/Options.rb'
require_once '../Utils/Prerequisite.rb'

module Build

    make_command(:export, 'Export expanded targets (JSON)') do |&namespace|
        export_output = Build.export_output

        source = JSONFile.new(export_output.filename, minify: Build.Minify)
        Export.make_targets(source, Build.get_environments(), namespace[])

        source.export_ifn?(export_output)
    end

    make_persistent_file(:export_output) do
        ext = File.extname(Build::Script)
        File.join($OutputPath, File.basename(Build::Script, ext) << '.json')
    end

    module Export

        def self.make_targets(json, environments, namespace)
            export = {}
            targets = namespace.all
            environments.each do |envname|
                env = Build.send(envname)
                targets.each do |target|
                    target_alias = "#{target.name}-#{env.family}"
                    expanded = env.expand(target)
                    export[target_alias] = Export.make_facet(expanded)
                end
            end
            json.value!(export)
            return
        end

        def self.make_facet(facet)
            export = {}
            facet.each{|k,v| export[k] = v.to_a}
            return export
        end

    end #~ Export

end #~ Build
