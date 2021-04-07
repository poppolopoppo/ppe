# frozen_string_literal: true

require 'set'

class DirCache
    @@_all_exists = Set.new

    def self.exist?(path)
        path = File.expand_path(path)
        return true if @@_all_exists.include?(path)
        if Dir.exist?(path)
            @@_all_exists << path
            return true
        else
            return false
        end
    end
    class << self
        alias_method :'exists?', :'exist?'
    end

    @@_all_patterns = {}

    def self.[](pattern)
        pattern = File.expand_path(pattern)
        cached = @@_all_patterns[pattern]
        if cached.nil?
            cached = Dir[pattern]
            cached.freeze
            @@_all_patterns[pattern] = cached
        end
        return cached
    end
end
