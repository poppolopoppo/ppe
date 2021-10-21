# frozen_string_literal: true


require_once '../Core/Target.rb'
require_once '../Shared/Compiler.rb'

require 'fileutils'
require 'pathname'
require 'set'

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

        def generated_key(*args)
            args.collect!{|x| x.to_s }
            File.join(@platform.name.to_s, @config.name.to_s, *args)
        end
        def generated_path(*args)
            args.collect!{|x| x.to_s }
            File.join($GeneratedPath.to_s, @platform.name.to_s, @config.name.to_s, *args)
        end

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
                return Pathname.new(path).relative_path_from(root).to_s # path.start_with?(root.to_s) ? Pathname.new(path).relative_path_from(root).to_s : path
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

        def self.executable_path(relativePath, platform_config, ext)
            path = relativePath.to_s.tr('/', '-')
            path = path << '-' << platform_config.to_s unless platform_config.nil?
            return File.join($BinariesPath, path << ext)
        end

        def output_path(relativePath, output=:obj)
            relativePath = relativePath.to_s
            case output
            when :executable, :shared
                outputPath = Environment.executable_path(relativePath, @platform.name.to_s<<'-'<<@config.name.to_s, @compiler.ext_for(output))
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
            #Log.debug '%s: output_path("%s", %s) -> "%s"', self.name, relativePath, output, outputPath
            return outputPath
        end

        def target_artifact_type(target)
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
        def target_need_link?(target)
            case target_artifact_type(target)
            when :executable, :shared
                return true
            when :headers, :library
                return false
            else
                Assert.unexpected(target_artifact_type(target))
            end
        end
        def target_dynamic_link?(target)
            return (:shared == target_artifact_type(target))
        end
        def target_artifact_path(target)
            return output_path(target.abs_path, target_artifact_type(target))
        end
        def target_debug_path(artifact)
            return File.join(
                File.dirname(artifact),
                File.basename(artifact, File.extname(artifact)) ) <<
                self.ext_for(:debug)
        end
        def target_deploy(target, filename)
            dst = File.join(
                File.dirname(target_artifact_path(target)),
                File.basename(filename) )
            unless File.exists?(dst) && FileUtils.identical?(filename, dst)
                Log.verbose("deploy '%s' to '%s'", filename, dst)
                FileUtils.copy_file(filename, dst)
                return true
            else
                return false
            end
        end
        def facet()
            if @memoized.defines.empty?
                Log.debug '%s: memoize facet', self.name

                @memoized.defines.append(
                    "BUILD_ENVIRONMENT=#{@name}",
                    "BUILD_PLATFORM=#{@platform.name}",
                    "BUILD_CONFIG=#{@config.name}",
                    "BUILD_COMPILER=#{@compiler.name}",
                    "BUILD_FAMILY=#{self.family}",
                    ### *DONT* do that: it would invalidate the build at every call
                    ### see Shared/Generated.rb instead ;)
                    #"BUILD_BRANCH=#{Build.branch?}",
                    #"BUILD_REVISION=#{Build.revision?}",
                    #"BUILD_TIMESTAMP=#{Time.now.to_i}",
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

            result.defines.append(
                "BUILD_TARGET_NAME=#{target.abs_path}",
                "BUILD_TARGET_ORDINAL=#{target.ordinal}" )

            target.decorate(result, self)
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

    def each_build_aliases(environments, targets, only_executables=false, &each_alias)
        Assert.expect(environments, Array)
        Assert.expect(targets, Array)

        configs = Set.new
        platforms = Set.new
        namespaces = Hash.new
        target_aliases = Set.new

        all_aliases = []

        environments.each do |env|
            Assert.expect(env, Environment)

            configs << env.config
            platforms << env.platform

            target_aliases = []

            if only_executables
                targets = targets.dup
                targets.delete_if do |target|
                    not target.executable?
                end
            end

            targets.collect do |target|
                Assert.expect(target, Target)

                nsp = target.namespace
                set = namespaces[nsp]
                set = namespaces[nsp] = Set.new if set.nil?
                set << target

                target_aliases << "#{target.abs_path}-#{env.family}"

                unless only_executables
                    each_alias.call target_aliases.last, []
                end
            end

            unless only_executables
                each_alias.call("#{env.family}", target_aliases)
            end

            all_aliases.concat(target_aliases)
        end

        namespaces.each do |namespace, namespace_targets|
            Assert.expect(namespace, Namespace)
            Assert.expect(namespace_targets, Set)

            namespace_aliases = []
            platforms.each do |platform|
                configs.each do |config|
                    each_alias.call((namespace_aliases << "#{namespace.path}-#{platform.name}-#{config.name}").last,  namespace_targets.collect do |target|
                        "#{target.abs_path}-#{platform.name}-#{config.name}"
                    end.to_a)
                end
            end

            each_alias.call("#{namespace.path}", namespace_aliases)
        end unless only_executables

        platforms.each do |platform|
            Assert.expect(platform, Platform)

            platform_aliases = []
            targets.each do |target|
                each_alias.call((platform_aliases << "#{target.abs_path}-#{platform.name}").last, configs.collect do |config|
                    "#{target.abs_path}-#{platform.name}-#{config.name}"
                end.to_a)
            end

            each_alias.call "#{platform.name}", platform_aliases

            namespaces.keys.each do |namespace|
                each_alias.call("#{namespace.path}-#{platform.name}", configs.collect do |config|
                    "#{namespace.path}-#{platform.name}-#{config.name}"
                end.to_a)
            end
        end unless only_executables

        configs.each do |config|
            Assert.expect(config, Configuration)

            config_aliases = []
            targets.each do |target|
                each_alias.call((config_aliases << "#{target.abs_path}-#{config.name}").last, platforms.collect do |platform|
                    "#{target.abs_path}-#{platform.name}-#{config.name}"
                end.to_a)
            end

            next if only_executables

            each_alias.call "#{config.name}", config_aliases

            namespaces.keys.each do |namespace|
                each_alias.call("#{namespace.path}-#{config.name}", platforms.collect do |platform|
                    "#{namespace.path}-#{platform.name}-#{config.name}"
                end.to_a)
            end
        end

        targets.each do |target|
            target_aliases = []
            environments.each do |env|
                target_aliases << "#{target.abs_path}-#{env.family}"
            end
            each_alias.call target.abs_path, target_aliases
        end unless only_executables

        each_alias.call('All', all_aliases) unless only_executables # default target
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
    def self.environment_compiler()
        $BuildEnvironments.each do |x|
            env = Build.send(x)
            comp = env.compiler
            Assert.expect?(comp, Compiler)
            return comp
        end
        return nil
    end

end #~ Build
