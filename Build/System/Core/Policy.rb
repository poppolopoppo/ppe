
require './Common.rb'

module Build

    class Policy
        attr_reader :name, :facet
        def initialize(name, facet: Facet.new)
            @name = name
            @facet = facet
            @customizations = []
        end
        def +(other) @facet + other.facet; return self end
        def -(other) @facet - other.facet; return self end
        def ==(other) @facet == other.facet; return self end
        def match?(expr)
            case expr
            when Regexp
                return @name =~ expr
            when String # assume glob string
                return File.fnmatch?(expr, @name)
            else
                raise ArgumentError.new('unexpected match expression')
            end
        end
        def custom!(&block)
            @customizations << block
            return self
        end
        def on_tag!(tag, options)
            custom!() do |facet, env|
                if facet.tag?(tag)
                    case options
                    when Facet
                        facet + options
                    when Hash
                        facet.set(options)
                    else
                        raise ArgumentError.new("unexpected options: #{options.inspect}")
                    end
                end
            end
        end
        def customize(facet, env)
            @customizations.each do |custom|
                custom.call(facet, env)
            end
        end
        Facet::SETS.each do |facet|
            aname = ('@'<<facet.to_s<<'s').to_sym
            define_method(facet.to_s<<'!') do |*values|
                f = @facet.instance_variable_get(aname)
                values.each {|v| f.append(v) }
                return self
            end
            define_method('no_'<<facet.to_s<<'!') do |*values|
                f = @facet.instance_variable_get(aname)
                values.each {|v| f.remove(v) }
                return self
            end
        end
    end #~ Policy

    class Platform < Policy
        attr_reader :target, :os
        def initialize(name, target, os)
            super(name)
            @target = target
            @os = os
        end
        def intrinsics_supported() Log.error("#{@name}: intrinsics_supported() is not implemented"); [] end
    end #~ Platform

    class Configuration < Policy
        attr_reader :link
        def initialize(name, link)
            super(name)
            @link = link
        end
    end #~ Configuration

    class Compiler < Policy
        attr_reader :compiler, :linker, :extra_files
        def initialize(name, compiler, linker, *extra_files)
            super(name)
            @compiler = compiler
            @linker = linker
            @extra_files = extra_files
        end
        def add_forceInclude(facet, filename) Log.error("#{@name}: add_forceInclude('#{filename}') is not implemented") end
        def add_includePath(facet, dirpath) Log.error("#{@name}: add_includePath('#{dirpath}') is not implemented") end
        def add_library(facet, filename) Log.error("#{@name}: add_library('#{filename}') is not implemented") end
        def add_libraryPath(facet, dirpath) Log.error("#{@name}: add_libraryPath('#{dirpath}') is not implemented") end
    end #~ Compiler

end #~ Build
