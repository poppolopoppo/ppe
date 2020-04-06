# frozen_string_literal: true

require_once '../Common.rb'
require_once '../Core/Target.rb'
require_once '../Shared/Compiler.rb'

require 'pathname'

module Build

    class Environment < Decorator
        attr_reader :name, :platform, :config, :compiler
        def initialize(name, platform, config, compiler)
            super()
            @name = name
            @platform = platform
            @config = config
            @compiler = compiler
            @memoized = Facet.new
        end

        def family() "#{@platform.name}-#{@config.name}" end
        def varname() "#{@platform.name}_#{@config.name}" end

        def ext_for(output) return @compiler.ext_for(output) end

        def intermediate_path(*args)
            args.collect!{|x| x.to_s }
            File.join($IntermediatePath.to_s, @platform.name.to_s, @config.name.to_s, *args)
        end
        def source_path(path)
            case path
            when String
                return path == File.absolute_path(path) ? path : File.join($SourcePath, path)
            when Array
                path.collect{|x| source_path(x) }
            when Set
                a = path.to_a
                a.collect!{|x| source_path(x) }
                return a
            else
                Log.fatal 'unsupported source path: %s', path.inspect
            end
        end
        def relative_path(root, path)
            root = Pathname.new(root) if root.is_a?(String)
            case path
            when String
                path = source_path(path)
                return path.start_with?(root.to_s) ? Pathname.new(path).relative_path_from(root).to_s : path
            when Array
                path.collect{|x| relative_path(root, x) }
            when Set
                a = path.to_a
                a.collect!{|x| relative_path(root, x) }
                return a
            else
                Log.fatal 'unsupported relative path: %s', path.inspect
            end
        end

        def output_path(relativePath, output=:obj)
            relativePath = relativePath.to_s
            case output
            when :executable, :shared
                outputPath = File.join($BinariesPath, relativePath.tr('/', '-')<<'-'<<
                    @platform.name.to_s<<'-'<<@config.name.to_s<<@compiler.ext_for(output))
            when :debug
                Log.fatal 'you must use target_debug_path() instead'
            when :library, :obj, :pch
                outputPath = File.join($IntermediatePath, @platform.name.to_s, @config.name.to_s,
                    File.dirname(relativePath),
                    File.basename(relativePath, File.extname(relativePath)) ) <<
                        @compiler.ext_for(output)
            when :headers
                return nil
            else
                Log.fatal 'unsupported output type: <%s>', output
            end
            Log.debug '%s: output_path("%s", %s) -> "%s"', self.name, relativePath, output, outputPath
            return outputPath
        end

        def target_artefact_type(target)
            case target.type
            when :external, :library
                case target.link
                when :static
                    return :library
                when :dynamic
                    return :shared
                when nil
                else
                    Log.fatal 'unsupported target link type <%s>', target.link
                end
            when :executable
                Log.fatal 'executable must have static link type: "%s" -> <%s>', abs_path, target.link if target.link != :static
                return :executable
            when :headers
                return :headers
            else
                Log.fatal 'unsupported target type <%s>', target.type
            end
            case @config.link
            when :static
                case target.type
                when :external, :library
                    return :library
                when :executable
                    return :executable
                else
                    Log.fatal 'unsupported target type <%s>', target.type
                end
            when :dynamic
                Log.fatal 'only library can have dynamic link type: "%s" -> <%s>', target.abs_path, target.type if target.type != :library
                return :shared
            else
                Log.fatal 'unsupported config link type <%s>', @config.link
            end
        end
        def target_artefact_path(target)
            return output_path(target.abs_path, target_artefact_type(target))
        end
        def target_debug_path(artefact)
            return File.join(
                File.dirname(artefact),
                File.basename(artefact, File.extname(artefact)) ) <<
                self.ext_for(:debug)
        end

        def facet()
            if @memoized.defines.empty?
                Log.debug '%s: memoize facet', self.name

                @memoized.defines.append(
                    "BUILD_ENV=#{@name}",
                    "BUILD_PLATFORM=#{@platform.name}",
                    "BUILD_CONFIG=#{@config.name}",
                    "BUILD_COMPILER=#{@compiler.name}",
                    "BUILD_#{self.varname}" )

                @platform.decorate(@memoized, self)
                @config.decorate(@memoized, self)
                @compiler.decorate(@memoized, self)

                apply_decorator(@memoized, self)

                @memoized.expand!

                self.freeze
            end
            return @memoized
        end
        def expand(target)
            result = self.facet().deep_dup

            target.customize(result, self, target)

            @platform.customize(result, self, target)
            @config.customize(result, self, target)
            @compiler.customize(result, self, target)

            return result.expand!()
        end

    end #~ Environment

    def make_environment(compilerName, platformNames, configNames)
        all = []

        platformNames = [ platformNames ] unless platformNames.is_a?(Array)
        configNames = [ configNames ] unless configNames.is_a?(Array)

        platformNames.each do |platformName|
            Log.fatal 'invalid platform name: "%"', platformName unless platformName.is_a?(Symbol)

            configNames.each do |configName|
                Log.fatal 'invalid config name: "%"', platformName unless configName.is_a?(Symbol)

                environmentName = "#{compilerName}_#{platformName}_#{configName}".to_sym
                Build.const_memoize(self, environmentName) do
                    Log.verbose 'new environment <%s>', environmentName
                    platform = Build.send(platformName)
                    config = Build.send(configName)
                    compiler = Build.send(compilerName)
                    Environment.new(environmentName, platform, config, compiler)
                end

                all << environmentName
            end
        end
        return all
    end

    $BuildEnvironments = []
    def self.get_environments()
        return $BuildEnvironments
    end
    def self.fetch_environments()
        return $BuildEnvironments.collect{|x| Build.send(x) }
    end
    def self.append_environments(*env)
        $BuildEnvironments.concat(env)
    end

end #~ Build
