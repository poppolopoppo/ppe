# frozen_string_literal: true

Build.namespace(:PPE) do
    include!(
        'External/External.rb',
        'Runtime/Runtime.rb',
        'ContentPipeline/ContentPipeline.rb',
        'Tools/Tools.rb' )
    #include!('Engine/Engine.rb')
end
