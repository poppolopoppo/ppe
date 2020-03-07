
require_once '../Common.rb'
require_once '../Core/Target.rb'
require_once '../Shared/Compiler.rb'

module Build

    class Environment < Decorator
        attr_reader :name, :platform, :config, :compiler
        def initialize(name, platform, config, compiler)
            super()
            @name = name
            @platform = platform
            @config = config
            @compiler = compiler
            @memoized = nil
        end
        def familly() "#{@platform.name}-#{@config.name}" end
        def intermediate_path(*args)
            args.collect!{|x| x.to_s }
            File.join($IntermediatePath.to_s, @platform.name.to_s, @config.name.to_s, *args)
        end
        def source_path(relativePath)
            case relativePath
            when String
                File.join($SourcePath, relativePath)
            when Array
                relativePath.collect{|x| source_path(x) }
            when Set
                a = relativePath.to_a
                a.collect!{|x| source_path(x) }
                return a
            else
                Log.fatal 'unsupported source path: %s', relativePath.inspect
            end
        end
        def ext_for(output) return @compiler.ext_for(output) end
        def output_path(relativePath, output=:obj)
            relativePath = relativePath.to_s
            dstExt = @compiler.ext_for(output)
            case output
            when :binary, :shared
                outputPath = File.join($BinariesPath, relativePath.gsub('/', '-')<<'-'<<@platform.name.to_s<<'-'<<@config.name.to_s<<dstExt)
            when :debug
                Log.fatal 'you must use target_debug_path() instead'
            else
                outputPath = File.join($IntermediatePath, @platform.name.to_s, @config.name.to_s,
                    File.dirname(relativePath),
                    File.basename(relativePath, File.extname(relativePath)) ) << dstExt
            end
            Log.debug '%s: output_path("%s", %s) -> "%s"', self.name, relativePath, output, outputPath
            return outputPath
        end
        def target_artefact_type(target)
            case target.type
            when :external
                Log.fatal 'executable must have static link type: "%s" -> <%s>', target.abs_path, target.link if target.link != :static
                return :library
            when :library
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
                return :binary
            else
                Log.fatal 'unsupported target type <%s>', target.type
            end
            case @config.link
            when :static
                case target.type
                when :external, :library
                    return :library
                when :executable
                    return :binary
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
        def target_debug_path(target)
            artefact = target_artefact_path(target)
            return File.join(
                File.dirname(artefact),
                File.basename(artefact, File.extname(artefact)) ) <<
                self.ext_for(:debug)
        end
        def facet()
            if @memoized.nil?
                Log.debug '%s: memoize facet', self.name

                @memoized = Facet.new
                @memoized.defines.append(
                    "BUILD_ENV=#{@name}",
                    "BUILD_PLATFORM=#{@platform.name}",
                    "BUILD_CONFIG=#{@config.name}",
                    "BUILD_COMPILER=#{@compiler.name}",
                    "BUILD_#{@platform.name}_#{@config.name}" )

                @memoized << @platform.facet << @config.facet << @compiler.facet

                apply_decorator(@memoized, self)
            end
            return @memoized
        end
        def expand(target)
            result = self.facet().clone

            target.customize(result, self, target)

            @platform.customize(result, self, target)
            @config.customize(result, self, target)
            @compiler.customize(result, self, target)

            link = target.link.nil? ? @config.link : target.link
            case link
            when :static
                result.defines << 'BUILD_LINK_STATIC'
            when :dynamic
                result.defines << 'BUILD_LINK_DYNAMIC'
            else
                Log.fatal 'unsupported link type <%s>', link
            end

            @compiler.add_linkType(result, link)

            result.defines.each do |token|
                @compiler.add_define(result, token)
            end
            result.includes.each do |path|
                @compiler.add_forceInclude(result, path)
            end
            result.includePaths.each do |path|
                @compiler.add_includePath(result, path)
            end
            result.librarys.each do |path|
                @compiler.add_library(result, path)
            end
            result.libraryPaths.each do |path|
                @compiler.add_libraryPath(result, path)
            end

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
    def self.append_environments(*env)
        $BuildEnvironments.concat(env)
    end

end #~ Build
