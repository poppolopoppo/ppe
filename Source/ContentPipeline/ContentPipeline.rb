# frozen_string_literal: true

$Build.namespace(:ContentPipeline) do
    include!('BuildGraph/BuildGraph.rb')
    include!('PipelineCompiler/PipelineCompiler.rb')
    include!('PipelineReflection/PipelineReflection.rb')
end
